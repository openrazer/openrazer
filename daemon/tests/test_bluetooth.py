# SPDX-License-Identifier: GPL-2.0-or-later
import asyncio
import unittest
from unittest.mock import patch
from openrazer_daemon.bluetooth.protocol import VendorSession, parse_frames, parse_dpi, SERVICE
from openrazer_daemon.bluetooth.backend import BasiliskSettings, connected_candidates

DPI_RAW = bytes.fromhex('030501900190010000022003200300000340064006000004800c800c0000050019001900')


class Value:
    def __init__(self, value):
        self.value = value


class ProtocolTests(unittest.TestCase):
    def test_fragmented_reply_and_rejection(self):
        header = bytes([0x30, 21, 0, 0, 0, 0, 0, 2])
        data = bytes(range(21))
        self.assertIsNone(parse_frames(0x30, [header, data[:20]]))
        self.assertEqual(parse_frames(0x30, [header, data[:20], data[20:]]), data)
        with self.assertRaises(ValueError):
            parse_frames(0x30, [bytes([0x30, 0, 0, 0, 0, 0, 0, 3])])

    def test_discovery_requires_paired_connected_resolved_and_service(self):
        path = '/org/bluez/hci0/dev_AA'
        props = {key: Value(True) for key in ('Paired', 'Connected', 'ServicesResolved')}
        props['Address'] = Value('AA:BB:CC:DD:EE:FF')
        objects = {path: {'org.bluez.Device1': props}, path+'/service': {'org.bluez.GattService1': {'Device': Value(path), 'UUID': Value(SERVICE)}}}
        self.assertEqual(connected_candidates(objects), {path: 'AA:BB:CC:DD:EE:FF'})
        for key in ('Paired', 'Connected', 'ServicesResolved'):
            props[key] = Value(False)
            self.assertEqual(connected_candidates(objects), {})
            props[key] = Value(True)
        objects.pop(path+'/service')
        self.assertEqual(connected_candidates(objects), {})

    def test_truncated_dpi_and_missing_last_marker(self):
        self.assertEqual(parse_dpi(DPI_RAW)['active_index'], 2)
        with self.assertRaises(ValueError):
            parse_dpi(DPI_RAW[:10])


class FakeSession:
    def __init__(self):
        self.values = {bytes.fromhex('0b840100'): DPI_RAW}
        self.writes = []
        self.reads = []
        self.bad_readback = False

    async def read(self, key):
        self.reads.append(key)
        value = self.values[key]
        return b'\x01' if self.bad_readback else value

    async def write(self, key, payload):
        self.writes.append((key, payload))
        read_key = bytes([key[0], key[1] | 0x80, key[2], key[3]])
        self.values[read_key] = payload
        await asyncio.sleep(0)


class SettingsTests(unittest.IsolatedAsyncioTestCase):
    async def test_dpi_preserves_other_stages_and_reserved_bytes(self):
        session = FakeSession()
        settings = BasiliskSettings(session)
        await settings.perform('set_dpi', 1700, 1800)
        _, payload = session.writes[0]
        expected = bytearray(DPI_RAW.ljust(38, b'\x00'))
        expected[17:19] = (1700).to_bytes(2, 'little')
        expected[19:21] = (1800).to_bytes(2, 'little')
        self.assertEqual(payload, expected)
        self.assertEqual(await settings.perform('dpi'), (1700, 1800))

    async def test_invalid_settings_send_no_commands(self):
        session = FakeSession()
        settings = BasiliskSettings(session)
        for operation, args in [('set_dpi', (0, 1600)), ('set_dpi', (1600, 30001)), ('set_brightness', ('logo', float('nan'))), ('set_brightness', ('logo', 101)), ('set_static', ('scroll', 256, 0, 0)), ('set_idle', (0,))]:
            with self.assertRaises(ValueError):
                await settings.perform(operation, *args)
        self.assertEqual(session.writes, [])
        self.assertEqual(session.reads, [])

    async def test_readback_failure_no_write_retry(self):
        session = FakeSession()
        session.bad_readback = True
        with self.assertRaises(ValueError):
            await BasiliskSettings(session).perform('set_static', 'logo', 255, 0, 0)
        self.assertEqual(len(session.writes), 1)

    async def test_rgb_payload_and_brightness_units(self):
        session = FakeSession()
        settings = BasiliskSettings(session)
        await settings.perform('set_static', 'scroll', 255, 0, 0)
        self.assertEqual(session.writes[0], (bytes.fromhex('10030001'), bytes.fromhex('01000001ff0000000000')))
        await settings.perform('set_brightness', 'logo', 50.0)
        self.assertEqual(session.writes[1], (bytes.fromhex('10050104'), b'\x80'))
        self.assertAlmostEqual(await settings.perform('brightness', 'logo'), 128*100/255)

    async def test_rmw_operations_do_not_interleave(self):
        session = FakeSession()
        settings = BasiliskSettings(session)
        await asyncio.gather(settings.perform('set_dpi', 1700, 1800), settings.perform('set_dpi', 1900, 2000))
        self.assertEqual(await settings.perform('dpi'), (1900, 2000))

    async def test_write_fragmentation_no_retries(self):
        class Client:
            async def write_gatt_char(self, uuid, payload, response):
                writes.append(payload)
                if len(writes) == 3:
                    session.notify(None, bytes([0x30, 0, 0, 0, 0, 0, 0, 2]))
        writes = []
        session = VendorSession(Client())
        await session.write(bytes.fromhex('0b040100'), bytes(38))
        self.assertEqual([len(value) for value in writes], [8, 20, 18])

    async def test_timeout_faults_session_before_next_write(self):
        class Client:
            async def write_gatt_char(self, *args, **kwargs):
                raise TimeoutError()
        session = VendorSession(Client())
        with self.assertRaises(TimeoutError):
            await session.read(bytes.fromhex('05810001'))
        with self.assertRaises(ConnectionError):
            await session.write(bytes.fromhex('10030001'), bytes(10))


class ManagerTests(unittest.TestCase):
    def test_reconnect_replaces_object_and_emits_events(self):
        from openrazer_daemon.bluetooth.manager import BluetoothManager
        from openrazer_daemon.device import DeviceCollection
        from unittest.mock import MagicMock
        daemon = MagicMock()
        daemon._razer_devices = DeviceCollection()
        daemon._config.getboolean.return_value = False
        manager = BluetoothManager.__new__(BluetoothManager)
        manager.daemon, manager.backend, manager.registered = daemon, MagicMock(), {}
        with patch('openrazer_daemon.bluetooth.manager.BasiliskV3ProBluetooth') as cls:
            device = cls.return_value
            device.serial = 'BT_AABB'
            manager.reconcile({'/mouse': 'AA:BB'})
            manager.reconcile({'/mouse': 'AA:BB'})
            self.assertEqual(cls.call_count, 1)
            manager.reconcile({})
            device.remove_from_connection.assert_called_once()
            self.assertEqual(len(daemon._razer_devices), 0)
            manager.reconcile({'/mouse': 'AA:BB'})
            self.assertEqual(cls.call_count, 2)
        self.assertEqual(daemon.device_added.call_count, 2)
        daemon.device_removed.assert_called_once()


class CompatibilityTests(unittest.TestCase):
    def test_legacy_image_dictionary_required_by_razergenie(self):
        import json
        from openrazer_daemon.bluetooth.device import BasiliskV3ProBluetooth
        # No Bluetooth or session bus is needed for metadata.
        device = BasiliskV3ProBluetooth.__new__(BasiliskV3ProBluetooth)
        self.assertEqual(json.loads(device.getRazerUrls()), {
            key: device.DEVICE_IMAGE for key in ('top_img', 'side_img', 'perspective_img')})


if __name__ == '__main__':
    unittest.main()
