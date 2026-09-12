# SPDX-License-Identifier: GPL-2.0-or-later

import unittest
import unittest.mock

from openrazer_daemon.dbus_services.dbus_methods import mamba


class DummyDevice(object):
    def __init__(self):
        self.logger = unittest.mock.MagicMock()
        self.low_battery_threshold = 5
        self.persist_calls = []

    def get_driver_path(self, path):
        self.last_driver_path = path
        return "/fake/charge_low_threshold"

    def set_persistence(self, zone, key, value):
        self.persist_calls.append((zone, key, value))


class LowBatteryThresholdTest(unittest.TestCase):
    def setUp(self):
        self.device = DummyDevice()

    def test_get_low_battery_threshold_timeout_returns_cached_value(self):
        open_mock = unittest.mock.mock_open(read_data="12")
        open_mock.return_value.__enter__.return_value.read.side_effect = TimeoutError(110, "Connection timed out")

        with unittest.mock.patch("builtins.open", open_mock):
            result = mamba.get_low_battery_threshold(self.device)

        self.assertEqual(result, 5)
        self.assertEqual(self.device.low_battery_threshold, 5)
        self.assertEqual(self.device.last_driver_path, "charge_low_threshold")
        self.assertTrue(self.device.logger.exception.called)

    def test_get_low_battery_threshold_invalid_read_returns_cached_value(self):
        open_mock = unittest.mock.mock_open(read_data="not-an-int")

        with unittest.mock.patch("builtins.open", open_mock):
            result = mamba.get_low_battery_threshold(self.device)

        self.assertEqual(result, 5)
        self.assertEqual(self.device.low_battery_threshold, 5)
        self.assertTrue(self.device.logger.exception.called)

    def test_get_low_battery_threshold_success_updates_cache_and_clamps_high(self):
        open_mock = unittest.mock.mock_open(read_data="255")

        with unittest.mock.patch("builtins.open", open_mock):
            result = mamba.get_low_battery_threshold(self.device)

        self.assertEqual(result, 25)
        self.assertEqual(self.device.low_battery_threshold, 25)

    def test_get_low_battery_threshold_success_clamps_low(self):
        open_mock = unittest.mock.mock_open(read_data="0")

        with unittest.mock.patch("builtins.open", open_mock):
            result = mamba.get_low_battery_threshold(self.device)

        self.assertEqual(result, 5)
        self.assertEqual(self.device.low_battery_threshold, 5)

    def test_set_low_battery_threshold_clamps_and_persists_high(self):
        open_mock = unittest.mock.mock_open()

        with unittest.mock.patch("builtins.open", open_mock):
            mamba.set_low_battery_threshold(self.device, 100)

        self.assertEqual(self.device.low_battery_threshold, 25)
        self.assertEqual(self.device.persist_calls[-1], (None, "low_battery_threshold", 25))
        open_mock.assert_called_once_with("/fake/charge_low_threshold", "w")
        open_mock.return_value.__enter__.return_value.write.assert_called_once_with("63")

    def test_set_low_battery_threshold_clamps_and_persists_low(self):
        open_mock = unittest.mock.mock_open()

        with unittest.mock.patch("builtins.open", open_mock):
            mamba.set_low_battery_threshold(self.device, 1)

        self.assertEqual(self.device.low_battery_threshold, 5)
        self.assertEqual(self.device.persist_calls[-1], (None, "low_battery_threshold", 5))
        open_mock.assert_called_once_with("/fake/charge_low_threshold", "w")
        open_mock.return_value.__enter__.return_value.write.assert_called_once_with("12")
