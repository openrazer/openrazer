# SPDX-License-Identifier: GPL-2.0-or-later
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import Mock, patch
import dbus

from openrazer_daemon.dbus_services.dbus_methods import blackshark
from openrazer_daemon.hardware.headsets import RazerBlackSharkV2HyperSpeedWireless, RazerBlackSharkV2HyperSpeedWired
from openrazer.client.devices.headsets import RazerBlackSharkV2HyperSpeed


class BlackSharkTests(unittest.TestCase):
    def test_boolean_controls_and_numeric_readback(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            device = SimpleNamespace(get_driver_path=lambda name: root / name)
            for name in ('sidetone_enabled', 'equalizer_enabled', 'do_not_disturb'):
                setter = getattr(blackshark, 'set_headset_' + name)
                getter = getattr(blackshark, 'get_headset_' + name)
                for value in (False, True):
                    setter(device, value)
                    self.assertIs(getter(device), value)
            for name, value in (('sidetone_level', dbus.Byte(12)), ('equalizer_preset', dbus.Byte(255)), ('dongle_led_mode', dbus.Byte(3))):
                getattr(blackshark, 'set_headset_' + name)(device, value)
                self.assertEqual(getattr(blackshark, 'get_headset_' + name)(device), value)

    def test_invalid_values_never_reach_sysfs(self):
        device = SimpleNamespace(get_driver_path=Mock())
        with patch.object(blackshark, '_write') as write:
            for name, value in (('sidetone_level', -1), ('sidetone_level', 16), ('dongle_led_mode', 4),
                                ('equalizer_preset', 1), ('equalizer_bands', [0] * 9),
                                ('equalizer_bands', [7] * 10), ('equalizer_bands', [0.5] * 10)):
                with self.assertRaises(ValueError):
                    getattr(blackshark, 'set_headset_' + name)(device, value)
            write.assert_not_called()

    def test_equalizer_signed_values_and_full_length(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            device = SimpleNamespace(get_driver_path=lambda name: root / name)
            gains = [-9, -3, 0, 1, 2, 3, 4, 5, 6, 0]
            blackshark.set_headset_equalizer_bands(device, gains)
            self.assertEqual(blackshark.get_headset_equalizer_bands(device), gains)

    def test_wired_capabilities_do_not_advertise_receiver_controls(self):
        wireless = RazerBlackSharkV2HyperSpeedWireless.METHODS
        wired = RazerBlackSharkV2HyperSpeedWired.METHODS
        for method in ('get_headset_dongle_led_mode', 'set_headset_dongle_led_mode', 'get_headset_wireless_connected'):
            self.assertIn(method, wireless)
            self.assertNotIn(method, wired)
        self.assertIn('get_battery', wired)
        self.assertIn('set_headset_equalizer_bands', wired)
        self.assertNotIn('set_headset_audio_enhancement', wireless)

    def test_pylib_capabilities_and_conversion(self):
        device = RazerBlackSharkV2HyperSpeed.__new__(RazerBlackSharkV2HyperSpeed)
        device.has = lambda capability: capability == 'equalizer'
        device._audio = Mock()
        device._audio.getEqualizerBands.return_value = [-9, 0, 1, 2, 3, 4, 5, 6, 0, 0]
        self.assertEqual(device.equalizer_bands, (-9, 0, 1, 2, 3, 4, 5, 6, 0, 0))
        device.equalizer_preset = 7
        device._audio.setEqualizerPreset.assert_called_once_with(7)
        with self.assertRaises(NotImplementedError):
            _ = device.dongle_led_mode
        with self.assertRaises(ValueError):
            device.equalizer_bands = (0,) * 9


if __name__ == '__main__':
    unittest.main()
