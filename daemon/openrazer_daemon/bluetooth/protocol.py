# SPDX-License-Identifier: GPL-2.0-or-later

"""Basilisk V3 Pro vendor GATT framing; no automatic command retries."""
import asyncio

SERVICE = '52401523-f97c-7f90-0e7f-6c6f4e36db1c'
WRITE = '52401524-f97c-7f90-0e7f-6c6f4e36db1c'
NOTIFY = '52401525-f97c-7f90-0e7f-6c6f4e36db1c'
PNP = '00002a50-0000-1000-8000-00805f9b34fb'


def parse_frames(request, frames):
    """Return a complete response, None for incomplete frames, or raise on rejection."""
    for index, frame in enumerate(frames):
        if len(frame) >= 8 and frame[0] == request and frame[7] in (1, 2, 3, 5):
            if frame[7] != 2:
                raise ValueError(f'Device rejected request with status 0x{frame[7]:02x}')
            length = frame[1]
            continuation = b''.join(frames[index + 1:])
            # V3 Pro headers are 8 bytes; prefer separate continuation frames.
            payload = continuation if continuation else frame[8:]
            return payload[:length] if len(payload) >= length else None
    return None


def parse_dpi(payload):
    if len(payload) < 2 or not 1 <= payload[1] <= 5:
        raise ValueError('Malformed DPI stage count')
    raw_active, count = payload[:2]
    stages = []
    for i in range(count):
        offset = 2 + 7 * i
        if len(payload) < offset + 5:
            raise ValueError('Truncated DPI stage')
        x = int.from_bytes(payload[offset+1:offset+3], 'little')
        y = int.from_bytes(payload[offset+3:offset+5], 'little')
        if not (100 <= x <= 30000 and 100 <= y <= 30000):
            raise ValueError('DPI outside supported model range')
        stages.append({'id': payload[offset], 'x': x, 'y': y})
    ids = [stage['id'] for stage in stages]
    if len(set(ids)) != count or raw_active not in ids:
        raise ValueError('Malformed DPI stage IDs or active stage')
    active = ids.index(raw_active)
    return {'active_index': active, 'active_raw': raw_active, 'stages': stages}


class VendorSession:
    """Serialize vendor exchanges; never automatically retry commands."""
    def __init__(self, client):
        self.client = client
        self.request = 0x30
        self.queue = asyncio.Queue()
        self.lock = asyncio.Lock()
        self.trace = []
        self.faulted = False

    def notify(self, sender, data):
        self.queue.put_nowait(bytes(data))

    async def read(self, key):
        return await self.exchange(key)

    async def write(self, key, payload):
        return await self.exchange(key, payload)

    async def exchange(self, key, payload=b''):
        if self.faulted:
            raise ConnectionError('Vendor session faulted; reconnect the mouse before further commands')
        try:
            return await self._exchange(key, payload)
        except ValueError:
            # A complete rejection reply is synchronized; malformed local arguments send nothing.
            raise
        except (Exception, asyncio.CancelledError):
            self.faulted = True
            raise

    async def _exchange(self, key, payload=b''):
        if len(key) != 4 or len(payload) > 255:
            raise ValueError("Invalid vendor key or payload length")
        async with self.lock:
            if self.faulted:
                raise ConnectionError('Vendor session faulted; reconnect before further commands')
            while not self.queue.empty():
                self.queue.get_nowait()
            request = self.request
            self.request = (self.request + 1) % 256
            if self.request in (0, 1):
                # Firmware emits unsolicited notifications using request ID 1.
                self.request = 2
            header = bytes([request, len(payload), 0, 0]) + key
            self.trace.append({'tx': header.hex(), 'rx': []})
            self.trace[:] = self.trace[-64:]
            await self.client.write_gatt_char(WRITE, header, response=True)
            for offset in range(0, len(payload), 20):
                chunk = payload[offset:offset + 20]
                self.trace[-1].setdefault("payload_tx", []).append(chunk.hex())
                await self.client.write_gatt_char(WRITE, chunk, response=True)
            frames = []
            async with asyncio.timeout(5):
                while True:
                    frames.append(await self.queue.get())
                    self.trace[-1]['rx'].append(frames[-1].hex())
                    result = parse_frames(request, frames)
                    if result is not None:
                        return result


class BlueZClient:
    """Use the existing BlueZ connection without disconnecting the user's mouse."""
    def __init__(self, bus, address, objects):
        self.bus = bus
        self.address = address
        matches = [(path, interfaces['org.bluez.Device1']) for path, interfaces in objects.items()
                   if 'org.bluez.Device1' in interfaces and interfaces['org.bluez.Device1']['Address'].value.lower() == address.lower()]
        if len(matches) != 1:
            raise ValueError('Mouse not present in BlueZ; pair/connect it in KDE first')
        self.path, properties = matches[0]
        if not properties['Connected'].value or not properties['ServicesResolved'].value:
            raise ValueError('Mouse must already be connected with services resolved')
        self.name = properties.get('Name').value if 'Name' in properties else ''
        self.characteristics = {}
        self.services = []
        self.notifications = {}
        for path, interfaces in objects.items():
            if not path.startswith(self.path + '/'):
                continue
            if 'org.bluez.GattService1' in interfaces:
                props = interfaces['org.bluez.GattService1']
                self.services.append(props['UUID'].value.lower())
            if 'org.bluez.GattCharacteristic1' in interfaces:
                props = interfaces['org.bluez.GattCharacteristic1']
                self.characteristics[props['UUID'].value.lower()] = {'path': path, 'properties': props['Flags'].value}

    async def interface(self, uuid):
        path = self.characteristics[uuid.lower()]['path']
        intro = await self.bus.introspect('org.bluez', path)
        return self.bus.get_proxy_object('org.bluez', path, intro)

    async def read_gatt_char(self, uuid):
        obj = await self.interface(uuid)
        async with asyncio.timeout(5):
            return await obj.get_interface('org.bluez.GattCharacteristic1').call_read_value({})

    async def write_gatt_char(self, uuid, data, response=True):
        from dbus_fast import Variant
        obj = await self.interface(uuid)
        async with asyncio.timeout(5):
            await obj.get_interface('org.bluez.GattCharacteristic1').call_write_value(data, {'type': Variant('s', 'request')})

    async def start_notify(self, uuid, callback):
        obj = await self.interface(uuid)
        props = obj.get_interface('org.freedesktop.DBus.Properties')
        def changed(interface, updates, invalidated):
            if interface == 'org.bluez.GattCharacteristic1' and 'Value' in updates:
                callback(uuid, bytes(updates['Value'].value))
        props.on_properties_changed(changed)
        char = obj.get_interface('org.bluez.GattCharacteristic1')
        try:
            await char.call_start_notify()
        except Exception:
            props.off_properties_changed(changed)
            raise
        self.notifications[uuid] = (char, props, changed)

    async def stop_notify(self, uuid):
        char, props, handler = self.notifications.pop(uuid)
        try:
            await char.call_stop_notify()
        finally:
            props.off_properties_changed(handler)

