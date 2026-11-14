# SPDX-License-Identifier: GPL-2.0-or-later

import unittest
from unittest.mock import MagicMock, Mock, patch

import openrazer_mouse_init as mouse_init


class StartupTest(unittest.TestCase):
    def test_already_driver_mode_still_establishes_kernel_intent(self):
        hid = MagicMock()
        mode = hid.__truediv__.return_value
        mode.write_bytes.return_value = 2
        mode.read_bytes.return_value = b'\x03\x00'
        self.assertTrue(mouse_init.attempt(hid))
        mode.write_bytes.assert_called_once_with(b'\x03\x00')

    def test_unconfirmed_write_does_not_complete_initialization(self):
        hid = MagicMock()
        mode = hid.__truediv__.return_value
        mode.write_bytes.return_value = 2
        mode.read_bytes.return_value = b'\x00\x00'
        self.assertFalse(mouse_init.attempt(hid))
        mode.write_bytes.side_effect = OSError('not ready')
        self.assertFalse(mouse_init.attempt(hid))

    def test_busy_startup_waits_for_input_without_idle_usb_polling(self):
        hid = Mock()
        hid.exists.return_value = True
        event = mouse_init.EVENT.pack(0, 0, 2, 0, 10)
        calls = []

        def selected(*_args):
            calls.append(1)
            self.assertEqual(attempt.call_count, 3)
            return ([42], [], []) if len(calls) == 3 else ([], [], [])

        with patch.object(mouse_init, 'attempt', side_effect=[False, False, False, True]) as attempt, \
                patch.object(mouse_init.time, 'sleep'), \
                patch.object(mouse_init, 'open_inputs', return_value=[42]), \
                patch.object(mouse_init.select, 'select', side_effect=selected), \
                patch.object(mouse_init.os, 'read', return_value=event), \
                patch.object(mouse_init.os, 'close') as close:
            self.assertEqual(mouse_init.initialize(hid), 0)
            self.assertEqual(attempt.call_count, 4)
            close.assert_called_once_with(42)

    def test_unplug_while_waiting_closes_input(self):
        hid = Mock()
        hid.exists.side_effect = [True, False]
        with patch.object(mouse_init, 'burst', return_value=False), \
                patch.object(mouse_init, 'open_inputs', return_value=[42]), \
                patch.object(mouse_init.select, 'select', return_value=([], [], [])), \
                patch.object(mouse_init.os, 'close') as close:
            self.assertEqual(mouse_init.initialize(hid), 0)
            close.assert_called_once_with(42)

    def test_only_mouse_activity_triggers_retry(self):
        for kind, code, value in [(0, 0, 0), (2, 0, 0), (1, 30, 1)]:
            self.assertFalse(mouse_init.activity(mouse_init.EVENT.pack(0, 0, kind, code, value)))
        for kind, code, value in [(2, 0, 1), (1, 0x110, 1), (1, 0x110, 0)]:
            self.assertTrue(mouse_init.activity(mouse_init.EVENT.pack(0, 0, kind, code, value)))

    def test_reject_other_hardware_and_paths(self):
        for name in ('../../other', '0003:1532:02C4.0001', '0003:1532:00D4.0001/other'):
            with self.assertRaises(ValueError):
                mouse_init.target(name)


if __name__ == '__main__':
    unittest.main()
