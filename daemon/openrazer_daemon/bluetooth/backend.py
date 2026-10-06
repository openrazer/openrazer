# SPDX-License-Identifier: GPL-2.0-or-later
"""BlueZ discovery and serialized model operations on a dedicated asyncio loop."""
import asyncio
import concurrent.futures
import logging
import math
import threading

from .protocol import BlueZClient, VendorSession, SERVICE, PNP, NOTIFY, parse_dpi

DEVICE_INTERFACE = 'org.bluez.Device1'
ZONE_IDS = {'backlight': 10, 'logo': 4, 'scroll': 1}
EXPECTED_PNP = bytes.fromhex('028e06ac00')


def connected_candidates(objects):
    """Only paired, resolved connections with the vendor service; never radio-scan."""
    service_devices = {str(interfaces['org.bluez.GattService1']['Device'].value)
                       for interfaces in objects.values() if 'org.bluez.GattService1' in interfaces
                       and interfaces['org.bluez.GattService1']['UUID'].value.lower() == SERVICE}
    return {path: props['Address'].value for path, interfaces in objects.items()
            if DEVICE_INTERFACE in interfaces for props in [interfaces[DEVICE_INTERFACE]]
            if path in service_devices and all(props.get(key) and props[key].value
                                                for key in ('Paired', 'Connected', 'ServicesResolved'))}


class BasiliskSettings:
    """Only hardware-validated features. Keep complete exchanges and RMW atomic."""
    def __init__(self, session):
        self.session = session
        self.lock = asyncio.Lock()

    async def read_exact(self, key, length):
        raw = await self.session.read(key)
        if len(raw) != length:
            raise ValueError('Unexpected setting response length')
        return raw

    async def write_verified(self, read_key, write_key, payload):
        await self.session.write(write_key, payload)
        await asyncio.sleep(0.25)
        if await self.session.read(read_key) != payload:
            raise ValueError('Bluetooth setting write was acknowledged but readback differed')

    async def perform(self, operation, *args):
        async with self.lock:
            return await getattr(self, '_' + operation)(*args)

    async def _battery(self):
        raw = await self.read_exact(bytes.fromhex('05810001'), 1)
        # Vendor charge level is 0..255, as in the OpenRazer USB API.
        return raw[0] * 100.0 / 255.0

    async def _idle(self):
        return int.from_bytes(await self.read_exact(bytes.fromhex('05840000'), 2), 'little')

    async def _set_idle(self, seconds):
        if not 60 <= int(seconds) <= 900:
            raise ValueError('Idle timeout must be 60..900 seconds')
        await self.write_verified(bytes.fromhex('05840000'), bytes.fromhex('05040000'), int(seconds).to_bytes(2, 'little'))

    async def _dpi_stages(self):
        state = parse_dpi(await self.session.read(bytes.fromhex('0b840100')))
        return state['active_index'], [(stage['x'], stage['y']) for stage in state['stages']]

    async def _dpi(self):
        active, stages = await self._dpi_stages()
        return stages[active]

    async def _set_dpi(self, x, y):
        if not all(100 <= int(value) <= 30000 for value in (x, y)):
            raise ValueError('DPI must be 100..30000 on both axes')
        original = await self.session.read(bytes.fromhex('0b840100'))
        state = parse_dpi(original)
        if len(state['stages']) != 5 or len(original) not in (36, 37, 38):
            raise ValueError('DPI write requires the validated five-stage table')
        payload = bytearray(original.ljust(38, b'\x00'))
        offset = 2 + state['active_index'] * 7
        payload[offset+1:offset+3] = int(x).to_bytes(2, 'little')
        payload[offset+3:offset+5] = int(y).to_bytes(2, 'little')
        await self.session.write(bytes.fromhex('0b040100'), bytes(payload))
        await asyncio.sleep(0.25)
        if parse_dpi(await self.session.read(bytes.fromhex('0b840100'))) != parse_dpi(payload):
            raise ValueError('DPI readback differed; setting state is uncertain')

    async def _brightness(self, zone):
        raw = await self.read_exact(bytes([0x10, 0x85, 1, ZONE_IDS[zone]]), 1)
        return raw[0] * 100.0 / 255.0

    async def _set_brightness(self, zone, percent):
        if not math.isfinite(float(percent)) or not 0 <= percent <= 100:
            raise ValueError('Brightness must be 0..100 percent')
        led = ZONE_IDS[zone]
        payload = bytes([round(percent * 255 / 100)])
        await self.write_verified(bytes([0x10, 0x85, 1, led]), bytes([0x10, 5, 1, led]), payload)

    async def _lighting(self, zone):
        return await self.read_exact(bytes([0x10, 0x83, 0, ZONE_IDS[zone]]), 10)

    async def _set_static(self, zone, red, green, blue):
        if not all(0 <= int(value) <= 255 for value in (red, green, blue)):
            raise ValueError('RGB components must be 0..255')
        led = ZONE_IDS[zone]
        payload = bytes([1, 0, 0, 1, int(red), int(green), int(blue), 0, 0, 0])
        await self.write_verified(bytes([0x10, 0x83, 0, led]), bytes([0x10, 3, 0, led]), payload)


class BlueZBackend:
    """Keep BlueZ notifications flowing while GLib dispatches synchronous client calls."""
    def __init__(self):
        self.logger = logging.getLogger('razer.bluetooth')
        self.loop = asyncio.new_event_loop()
        self.thread = threading.Thread(target=self._run, name='openrazer-bluez', daemon=True)
        self.bus = None
        self.devices = {}
        self.rejected = set()
        self.thread.start()
        try:
            self.call(self._connect())
        except Exception:
            self.loop.call_soon_threadsafe(self.loop.stop)
            self.thread.join(timeout=2)
            raise

    def _run(self):
        asyncio.set_event_loop(self.loop)
        self.loop.run_forever()
        self.loop.close()

    def call(self, coroutine):
        future = asyncio.run_coroutine_threadsafe(coroutine, self.loop)
        try:
            return future.result(timeout=12)
        except concurrent.futures.TimeoutError:
            future.cancel()
            raise TimeoutError('BlueZ operation timed out; no command retry') from None

    async def _connect(self):
        from dbus_fast.aio import MessageBus
        from dbus_fast.constants import BusType
        self.bus = await MessageBus(bus_type=BusType.SYSTEM).connect()

    async def discover(self):
        async with asyncio.timeout(5):
            obj = self.bus.get_proxy_object('org.bluez', '/', await self.bus.introspect('org.bluez', '/'))
            objects = await obj.get_interface('org.freedesktop.DBus.ObjectManager').call_get_managed_objects()
        candidates = connected_candidates(objects)
        self.rejected.intersection_update(candidates)
        for path in list(self.devices):
            if path not in candidates:
                await self.release(path)
        for path, address in candidates.items():
            if path in self.devices or path in self.rejected:
                continue
            client = BlueZClient(self.bus, address, objects)
            try:
                if (await client.read_gatt_char(PNP))[:5] != EXPECTED_PNP:
                    self.rejected.add(path)
                    continue
                session = VendorSession(client)
                await client.start_notify(NOTIFY, session.notify)
                self.devices[path] = (client, BasiliskSettings(session))
            except Exception:
                self.rejected.add(path)
                self.logger.warning('Cannot initialize Bluetooth device %s', path, exc_info=True)
        return {path: client.address for path, (client, _) in self.devices.items()}

    async def perform(self, path, operation, *args):
        if path not in self.devices:
            raise ConnectionError('Bluetooth device is no longer connected')
        return await self.devices[path][1].perform(operation, *args)

    async def release(self, path):
        pair = self.devices.pop(path, None)
        if pair:
            try:
                await pair[0].stop_notify(NOTIFY)
            except Exception:
                self.logger.debug('Notification cleanup after disconnect', exc_info=True)

    async def _shutdown(self):
        for path in list(self.devices):
            await self.release(path)
        if self.bus:
            self.bus.disconnect()

    def close(self):
        try:
            self.call(self._shutdown())
        finally:
            self.loop.call_soon_threadsafe(self.loop.stop)
            self.thread.join(timeout=2)
