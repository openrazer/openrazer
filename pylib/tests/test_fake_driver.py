# SPDX-License-Identifier: GPL-2.0-or-later

"""Fake sysfs resets must work without root or a writable device directory."""

from pathlib import Path
import stat
import tempfile
import unittest

from openrazer._fake_driver import FakeDevice


class FakeDeviceResetTest(unittest.TestCase):
    def test_reset_restores_defaults_and_truncates_empty_endpoints(self):
        with tempfile.TemporaryDirectory() as tmp_dir:
            device = FakeDevice('razerblackwidowchroma', serial='TESTSERIAL', tmp_dir=tmp_dir)
            try:
                path = Path(tmp_dir) / '0003:1532:0203.0001'
                self.assertEqual(stat.S_IMODE(path.stat().st_mode), 0o555)
                device.set('matrix_brightness', '255000')
                device.set('device_serial', 'CHANGEDSERIAL')
                device.set('device_mode', 'changed binary default')
                device.set('matrix_effect_static', 'stale effect bytes')

                device.create_endpoints()

                self.assertEqual(device.get('matrix_brightness'), '0')
                self.assertEqual(device.get('device_serial'), 'TESTSERIAL')
                self.assertEqual(device.get('device_mode', binary=True), b'\x00\x00')
                self.assertEqual(device.get('matrix_effect_static', binary=True), b'')
                self.assertEqual(stat.S_IMODE(path.stat().st_mode), 0o555)
            finally:
                device.close()


if __name__ == '__main__':
    unittest.main()
