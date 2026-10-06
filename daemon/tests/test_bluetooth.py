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


class LightingClientTests(unittest.TestCase):
    def test_body_effect_controls_never_write_logo_or_scroll(self):
        from openrazer_daemon.bluetooth.device import BasiliskV3ProBluetooth
        class Device:
            def __init__(self):
                self.calls = []
            def invoke(self, *args):
                self.calls.append(args)
        device = Device()
        cases = [('setStatic', (0, 255, 0), 'set_static'),
                 ('setNone', (), 'set_none'),
                 ('setSpectrum', (), 'set_spectrum'),
                 ('setBreathSingle', (0, 255, 0), 'set_breath_single'),
                 ('setBreathDual', (0, 255, 0, 255, 0, 0), 'set_breath_dual'),
                 ('setBreathRandom', (), 'set_breath_random'),
                 ('setWave', (2,), 'set_wave')]
        for method, args, operation in cases:
            with self.subTest(method=method):
                device.calls.clear()
                getattr(BasiliskV3ProBluetooth, method)(device, *args)
                self.assertEqual(device.calls, [(operation, 'backlight', *args)])

    def test_dual_color_readback_exposes_both_colors_and_random_exposes_none(self):
        from openrazer_daemon.bluetooth.device import BasiliskV3ProBluetooth
        class Device:
            raw = bytes.fromhex('02020002ff00ff00ffff')
            def invoke(self, operation, zone):
                return self.raw
        device = Device()
        self.assertEqual(BasiliskV3ProBluetooth.colors(device, 'logo'), [255, 0, 255, 0, 255, 255])
        device.raw = bytes.fromhex('02000000000000000000')
        self.assertEqual(BasiliskV3ProBluetooth.colors(device, 'logo'), [])
        device.raw = bytes(10)
        self.assertEqual(BasiliskV3ProBluetooth.colors(device, 'logo'), [])


class FakeSession:
    def __init__(self):
        self.values = {bytes.fromhex('0b840100'): DPI_RAW, bytes.fromhex('03820000'): b'\x01', bytes.fromhex('03800000'): b'\x01'}
        for led in (1, 4, 10):
            self.values[bytes([0x10, 0x83, 0, led])] = bytes.fromhex('03000000000000000000')
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
        self.assertEqual(session.writes[0], (bytes.fromhex('10030101'), bytes.fromhex('01000001ff0000000000')))
        self.assertEqual(session.writes[1], (bytes.fromhex('10030001'), bytes.fromhex('01000001ff0000000000')))
        await settings.perform('set_brightness', 'logo', 50.0)
        self.assertEqual(session.writes[2], (bytes.fromhex('10050104'), b'\x80'))
        self.assertAlmostEqual(await settings.perform('brightness', 'logo'), 128*100/255)

    async def test_persistent_color_survives_live_state_reset(self):
        session = FakeSession()
        settings = BasiliskSettings(session)
        await settings.perform('set_static', 'logo', 255, 0, 255)
        # Firmware reloads the base profile after reconnect/power-on.
        session.values[bytes.fromhex('10830004')] = session.values[bytes.fromhex('10830104')]
        self.assertEqual(await settings.perform('lighting', 'logo'), bytes.fromhex('01000001ff00ff000000'))

    async def test_other_active_profile_is_not_overwritten(self):
        session = FakeSession()
        session.values[bytes.fromhex('03820000')] = b'\x03'
        session.values[bytes.fromhex('03800000')] = b'\x01\x03'
        with self.assertRaises(ValueError):
            await BasiliskSettings(session).perform('set_static', 'logo', 255, 0, 0)
        self.assertEqual(session.writes, [])

    async def test_missing_base_profile_is_not_written(self):
        session = FakeSession()
        session.values[bytes.fromhex('03800000')] = b'\x03'
        with self.assertRaises(ValueError):
            await BasiliskSettings(session).perform('set_static', 'logo', 255, 0, 0)
        self.assertEqual(session.writes, [])

    async def test_spectrum_updates_saved_and_live_state(self):
        from openrazer_daemon.bluetooth.backend import SPECTRUM_PAYLOAD, lighting_effect
        session = FakeSession()
        settings = BasiliskSettings(session)
        await settings.perform('set_spectrum', 'logo')
        self.assertEqual(session.values[bytes.fromhex('10830104')], SPECTRUM_PAYLOAD)
        self.assertEqual(session.values[bytes.fromhex('10830004')], SPECTRUM_PAYLOAD)
        self.assertEqual(lighting_effect(await settings.perform('lighting', 'logo')), 'spectrum')

    async def test_breathing_preserves_selected_rgb_in_both_states(self):
        from openrazer_daemon.bluetooth.backend import lighting_effect
        session = FakeSession()
        settings = BasiliskSettings(session)
        await settings.perform('set_breath_single', 'scroll', 255, 0, 255)
        expected = bytes.fromhex('02010001ff00ff000000')
        self.assertEqual(session.values[bytes.fromhex('10830101')], expected)
        self.assertEqual(await settings.perform('lighting', 'scroll'), expected)
        self.assertEqual(lighting_effect(expected), 'breathSingle')
        with self.assertRaises(ValueError):
            await settings.perform('set_breath_single', 'scroll', 256, 0, 0)

    async def test_dual_and_random_breathing_preserve_saved_and_live_states(self):
        from openrazer_daemon.bluetooth.backend import lighting_effect
        session = FakeSession()
        settings = BasiliskSettings(session)
        await settings.perform('set_breath_dual', 'logo', 255, 0, 255, 0, 255, 255)
        dual = bytes.fromhex('02020002ff00ff00ffff')
        self.assertEqual(await settings.perform('lighting', 'logo'), dual)
        self.assertEqual(session.values[bytes.fromhex('10830104')], dual)
        self.assertEqual(lighting_effect(dual), 'breathDual')
        before = len(session.writes)
        with self.assertRaises(ValueError):
            await settings.perform('set_breath_dual', 'logo', 0, 0, 0, 0, -1, 0)
        self.assertEqual(len(session.writes), before)
        await settings.perform('set_breath_random', 'logo')
        random = bytes.fromhex('02000000000000000000')
        self.assertEqual(await settings.perform('lighting', 'logo'), random)
        self.assertEqual(session.values[bytes.fromhex('10830104')], random)
        self.assertEqual(lighting_effect(random), 'breathRandom')

    async def test_off_preserves_brightness_and_updates_both_lighting_states(self):
        from openrazer_daemon.bluetooth.backend import lighting_effect
        session = FakeSession()
        brightness_key = bytes.fromhex('10850104')
        session.values[brightness_key] = b'\x59'
        settings = BasiliskSettings(session)
        await settings.perform('set_none', 'logo')
        self.assertEqual(await settings.perform('lighting', 'logo'), bytes(10))
        self.assertEqual(session.values[bytes.fromhex('10830104')], bytes(10))
        self.assertEqual(session.values[brightness_key], b'\x59')
        self.assertEqual(lighting_effect(bytes(10)), 'none')

    async def test_wave_directions_and_invalid_direction(self):
        from openrazer_daemon.bluetooth.backend import lighting_effect
        session = FakeSession()
        settings = BasiliskSettings(session)
        for direction in (1, 2):
            await settings.perform('set_wave', 'backlight', direction)
            raw = await settings.perform('lighting', 'backlight')
            self.assertEqual(raw, bytes([4, direction, 0x28, 0, 0, 0, 0, 0, 0, 0]))
            self.assertEqual(session.values[bytes.fromhex('1083010a')], raw)
            self.assertEqual(lighting_effect(raw), 'wave')
            self.assertEqual(await settings.perform('wave_direction', 'backlight'), direction)
        before = len(session.writes)
        with self.assertRaises(ValueError):
            await settings.perform('set_wave', 'backlight', 3)
        self.assertEqual(len(session.writes), before)

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
    def test_unvalidated_effect_is_not_misreported(self):
        from openrazer_daemon.bluetooth.backend import lighting_effect
        self.assertEqual(lighting_effect(bytes.fromhex('07000000000000000000')), 'unknown')
        with self.assertRaises(ValueError):
            lighting_effect(b'\x03')

    def test_signal_during_startup_is_queued_with_signal_number(self):
        import signal
        from unittest.mock import MagicMock
        from openrazer_daemon.daemon import RazerDaemon
        daemon = RazerDaemon.__new__(RazerDaemon)
        daemon.quit = MagicMock()
        with patch('openrazer_daemon.daemon.signal.signal') as register, patch('openrazer_daemon.daemon.GLib.idle_add') as queue:
            daemon._init_signals()
            callback = register.call_args_list[1].args[1]
            callback(signal.SIGTERM, None)
            action, signum = queue.call_args.args
            self.assertEqual(signum, signal.SIGTERM)
            daemon.quit.assert_not_called()
            action(signum)
            daemon.quit.assert_called_once_with(signal.SIGTERM)

    def test_legacy_image_dictionary_required_by_razergenie(self):
        import json
        from openrazer_daemon.bluetooth.device import BasiliskV3ProBluetooth
        # No Bluetooth or session bus is needed for metadata.
        device = BasiliskV3ProBluetooth.__new__(BasiliskV3ProBluetooth)
        self.assertEqual(json.loads(device.getRazerUrls()), {
            key: device.DEVICE_IMAGE for key in ('top_img', 'side_img', 'perspective_img')})


if __name__ == '__main__':
    unittest.main()
