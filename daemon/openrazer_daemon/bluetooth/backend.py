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
SCROLL_COMMANDS = {'mode': 5, 'acceleration': 6, 'smart_reel': 7}
SPECTRUM_PAYLOAD = bytes.fromhex('03000000000000000000')


def lighting_effect(payload):
    if len(payload) != 10:
        raise ValueError('Unexpected lighting state length')
    if payload == bytes(10):
        return 'none'
    if payload[:4] == bytes([1, 0, 0, 1]):
        return 'static'
    if payload == SPECTRUM_PAYLOAD:
        return 'spectrum'
    if payload == bytes([2, 0, 0, 0, 0, 0, 0, 0, 0, 0]):
        return 'breathRandom'
    if payload[:4] == bytes([2, 2, 0, 2]):
        return 'breathDual'
    if payload[:4] == bytes([2, 1, 0, 1]) and payload[7:] == bytes(3):
        return 'breathSingle'
    if payload[0] == 4 and payload[1] in (1, 2) and payload[2] == 0x28 and payload[3:] == bytes(7):
        return 'wave'
    return 'unknown'


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

    async def _require_base_profile(self):
        if await self.read_exact(bytes.fromhex('03820000'), 1) != b'\x01':
            raise ValueError('Profile settings writes are validated only for the active base profile')

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
        await self._require_base_profile()
        original = await self.session.read(bytes.fromhex('0b840100'))
        state = parse_dpi(original)
        if len(original) not in (1 + 7 * len(state['stages']), 2 + 7 * len(state['stages']), 38):
            raise ValueError('DPI write requires the validated five-slot table')
        payload = bytearray(original.ljust(38, b'\x00'))
        offset = 2 + state['active_index'] * 7
        payload[offset+1:offset+3] = int(x).to_bytes(2, 'little')
        payload[offset+3:offset+5] = int(y).to_bytes(2, 'little')
        await self.session.write(bytes.fromhex('0b040100'), bytes(payload))
        await asyncio.sleep(0.25)
        if parse_dpi(await self.session.read(bytes.fromhex('0b840100'))) != parse_dpi(payload):
            raise ValueError('DPI readback differed; setting state is uncertain')

    async def _set_dpi_stages(self, active_stage, stages):
        stages = list(stages)
        if not 1 <= len(stages) <= 5 or not 1 <= int(active_stage) <= len(stages):
            raise ValueError('DPI editing requires one to five stages and a valid active stage')
        if any(len(stage) != 2 or any(not 100 <= int(value) <= 30000 for value in stage) for stage in stages):
            raise ValueError('DPI must be 100..30000 on both axes')
        await self._require_base_profile()
        original = await self.session.read(bytes.fromhex('0b840100'))
        state = parse_dpi(original)
        if len(original) not in (1 + 7 * len(state['stages']), 2 + 7 * len(state['stages']), 38):
            raise ValueError('DPI stage editing requires the validated five-slot table')
        ids = [stage['id'] for stage in state['stages']]
        if len(set(ids)) != len(ids) or state['active_raw'] not in ids:
            raise ValueError('DPI stage IDs must be distinct and include the active stage')
        if len(stages) > len(ids):
            if ids != list(range(1, len(ids) + 1)):
                raise ValueError('Adding stages requires validated sequential stage IDs')
            ids = list(range(1, len(stages) + 1))
        payload = bytearray(original.ljust(38, b'\x00'))
        payload[0] = ids[int(active_stage) - 1]
        payload[1] = len(stages)
        for index, (x, y) in enumerate(stages):
            offset = 2 + index * 7
            payload[offset] = ids[index]
            payload[offset+1:offset+3] = int(x).to_bytes(2, 'little')
            payload[offset+3:offset+5] = int(y).to_bytes(2, 'little')
        await self.session.write(bytes.fromhex('0b040100'), bytes(payload))
        await asyncio.sleep(0.25)
        if parse_dpi(await self.session.read(bytes.fromhex('0b840100'))) != parse_dpi(payload):
            raise ValueError('DPI stages readback differed; setting state is uncertain')

    async def _scroll_setting(self, setting):
        command = SCROLL_COMMANDS[setting]
        raw = await self.read_exact(bytes([8, command | 0x80, 1, 0]), 1)
        if raw[0] not in (0, 1):
            raise ValueError('Unexpected scroll setting value')
        return raw[0]

    async def _set_scroll_setting(self, setting, value):
        if value not in (0, 1):
            raise ValueError('Scroll setting must be 0 or 1')
        command = SCROLL_COMMANDS[setting]
        await self._require_base_profile()
        await self.write_verified(bytes([8, command | 0x80, 1, 0]), bytes([8, command, 1, 0]), bytes([int(value)]))

    async def _brightness(self, zone):
        raw = await self.read_exact(bytes([0x10, 0x85, 1, ZONE_IDS[zone]]), 1)
        return raw[0] * 100.0 / 255.0

    async def _set_brightness(self, zone, percent):
        if not math.isfinite(float(percent)) or not 0 <= percent <= 100:
            raise ValueError('Brightness must be 0..100 percent')
        await self._require_base_profile()
        led = ZONE_IDS[zone]
        payload = bytes([round(percent * 255 / 100)])
        await self.write_verified(bytes([0x10, 0x85, 1, led]), bytes([0x10, 5, 1, led]), payload)

    async def _lighting(self, zone):
        return await self.read_exact(bytes([0x10, 0x83, 0, ZONE_IDS[zone]]), 10)

    async def _set_static(self, zone, red, green, blue):
        if not all(0 <= int(value) <= 255 for value in (red, green, blue)):
            raise ValueError('RGB components must be 0..255')
        payload = bytes([1, 0, 0, 1, int(red), int(green), int(blue), 0, 0, 0])
        await self._set_lighting_payload(zone, payload)

    async def _set_none(self, zone):
        await self._set_lighting_payload(zone, bytes(10))

    async def _set_spectrum(self, zone):
        await self._set_lighting_payload(zone, SPECTRUM_PAYLOAD)

    async def _set_breath_single(self, zone, red, green, blue):
        if not all(0 <= int(value) <= 255 for value in (red, green, blue)):
            raise ValueError('RGB components must be 0..255')
        payload = bytes([2, 1, 0, 1, int(red), int(green), int(blue), 0, 0, 0])
        await self._set_lighting_payload(zone, payload)

    async def _set_breath_random(self, zone):
        await self._set_lighting_payload(zone, bytes([2, 0, 0, 0, 0, 0, 0, 0, 0, 0]))

    async def _set_breath_dual(self, zone, red, green, blue, red2, green2, blue2):
        colors = (red, green, blue, red2, green2, blue2)
        if not all(0 <= int(value) <= 255 for value in colors):
            raise ValueError('RGB components must be 0..255')
        await self._set_lighting_payload(zone, bytes([2, 2, 0, 2, *map(int, colors)]))

    async def _set_wave(self, zone, direction):
        if direction not in (1, 2):
            raise ValueError('Wave direction must be 1 or 2')
        payload = bytes([4, int(direction), 0x28, 0, 0, 0, 0, 0, 0, 0])
        await self._set_lighting_payload(zone, payload)

    async def _wave_direction(self, zone):
        payload = await self._lighting(zone)
        return payload[1] if lighting_effect(payload) == 'wave' else 1

    async def _set_lighting_payload(self, zone, payload):
        led = ZONE_IDS[zone]
        # Target 0 changes only the live state. Target 1 is the base profile
        # and has been verified to retain RGB effects through a power cycle.
        active = await self.read_exact(bytes.fromhex('03820000'), 1)
        inventory = await self.session.read(bytes.fromhex('03800000'))
        if active != b'\x01' or 1 not in inventory or len(set(inventory)) != len(inventory) or any(target not in range(1, 6) for target in inventory):
            raise ValueError('Persistent lighting is validated only for the active base profile')
        await self.write_verified(bytes([0x10, 0x83, 1, led]), bytes([0x10, 3, 1, led]), payload)
        live_key = bytes([0x10, 0x83, 0, led])
        if await self.read_exact(live_key, 10) != payload:
            # Apply immediately if the profile write did not update the live mirror.
            # This is a distinct live operation, not a retry of the stored write.
            await self.write_verified(live_key, bytes([0x10, 3, 0, led]), payload)


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
