# SPDX-License-Identifier: GPL-2.0-or-later

"""Mouse Dock Pro lifecycle and failed RF transaction regressions."""

import configparser
from pathlib import Path
import tempfile
import types
import unittest
from unittest.mock import Mock, patch

from openrazer_daemon.daemon import RazerDaemon
from openrazer_daemon.device import DeviceCollection
from openrazer_daemon.dbus_services.dbus_methods import mamba
from openrazer_daemon.hardware.accessory import RazerMouseDockPro
from openrazer_daemon.hardware.mouse import (
    DockedMouseNotReady, RazerBasiliskV3ProDocked,
)


class FakeDockedMouse:
    WIRELESS_PID = 0x00AB

    def __init__(self, expected_serial, **kwargs):
        self.serial = expected_serial
        self.close = Mock()
        self.remove_from_connection = Mock()
        self.register_parent = Mock()
        self.retry_pending_restore = Mock()
        self.read_reported_dpi = Mock(return_value=None)
        self.update_dpi_from_report = Mock()

    def get_serial(self):
        return self.serial


class DockIdentityTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.path = Path(self.tmp.name)
        self.dock = object.__new__(RazerMouseDockPro)
        self.dock._is_closed = True
        self.dock._device_path = self.tmp.name
        self.dock._get_wireless_pid_registry = Mock(return_value={0x00AB: RazerBasiliskV3ProDocked})

    def test_offline_pid_and_unknown_slot_are_preserved_without_child(self):
        (self.path / 'paired_slots').write_text('1:0:00ab 2:1:1234 3:0:ffff\n')
        self.assertEqual(self.dock.get_paired_slots(), [
            (1, False, '00ab'), (2, True, '1234'), (3, False, 'ffff'),
        ])
        self.assertEqual(self.dock.get_child_devices(), [])

    def test_unknown_active_slot_does_not_get_a_guessed_child_class(self):
        (self.path / 'paired_slots').write_text('1:1:1234 2:0:ffff\n')
        (self.path / 'mouse_serial').write_text('TESTMOUSE01\n')
        self.assertEqual(self.dock.get_child_devices(), [])

    def test_child_requires_valid_serial_and_stable_slot(self):
        (self.path / 'paired_slots').write_text('1:1:00ab 2:0:ffff\n')
        for serial in ('', 'bad serial', 'lowercase'):
            with self.subTest(serial=serial):
                (self.path / 'mouse_serial').write_text(serial)
                self.assertEqual(self.dock.get_child_devices(), [])
        (self.path / 'mouse_serial').write_text('TESTMOUSE01\n')
        self.dock.get_paired_slots = Mock(side_effect=[
            [(1, True, '00ab')], [(1, False, '00ab')],
        ])
        self.assertEqual(self.dock.get_child_devices(), [])

    def test_known_active_identity_selects_original_basilisk(self):
        (self.path / 'paired_slots').write_text('1:1:00ab 2:0:ffff\n')
        (self.path / 'mouse_serial').write_text('TESTMOUSE01\n')
        self.assertEqual(self.dock.get_child_devices(), [
            (RazerBasiliskV3ProDocked, {'id_suffix': ':mouse', 'serial': 'TESTMOUSE01'}),
        ])

    def test_mouse_connected_read_error_is_not_a_disconnect(self):
        with patch('builtins.open', side_effect=OSError(5, 'I/O error')):
            with self.assertRaises(OSError):
                self.dock.is_mouse_connected()

    def test_changed_serial_during_discovery_is_rejected(self):
        mouse = object.__new__(RazerBasiliskV3ProDocked)
        mouse._is_closed = True
        mouse._device_path = self.tmp.name
        mouse._serial = None
        mouse._expected_serial = 'MOUSEEXPECTED'
        mouse.logger = Mock()
        (self.path / 'mouse_serial').write_text('MOUSEOTHER\n')

        with self.assertRaises(DockedMouseNotReady):
            mouse.get_serial()
        self.assertIsNone(mouse._serial)


class DockLifecycleTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.daemon = object.__new__(RazerDaemon)
        self.daemon.logger = Mock()
        self.daemon._razer_devices = DeviceCollection()
        self.daemon._dock_mouse_pending = {}
        self.daemon._config = configparser.ConfigParser()
        self.daemon._persistence = configparser.ConfigParser()
        self.daemon._persistence_file = None
        self.daemon._test_dir = self.tmp.name
        self.daemon._unknown_serial_counter = {}
        self.daemon.device_added = Mock()
        self.daemon.device_removed = Mock()
        self.daemon.write_persistence = Mock()
        self.dock_id = '0003:1532:00A4.0001'
        self.child_id = self.dock_id + ':mouse'
        self.dock = object.__new__(RazerMouseDockPro)
        self.dock._is_closed = True
        self.dock._device_path = self.tmp.name
        self.dock.register_parent = Mock()
        self.dock.is_mouse_connected = Mock(return_value=True)
        self.dock.get_active_mouse_identity = Mock(return_value=(0x00AB, 'MOUSEOLD'))
        self.dock.get_child_devices = Mock(return_value=[
            (FakeDockedMouse, {'id_suffix': ':mouse', 'serial': 'MOUSENEW'}),
        ])
        self.daemon._razer_devices.add(self.dock_id, 'DOCKSERIAL', self.dock)
        self.old_mouse = FakeDockedMouse('MOUSEOLD')
        self.daemon._razer_devices.add(self.child_id, 'MOUSEOLD', self.old_mouse)
        self.daemon._razer_devices[self.dock_id].child_ids.append(self.child_id)

    def test_identity_replacement_requires_two_observations_and_releases_old_serial(self):
        self.dock.get_active_mouse_identity.return_value = (0x00AB, 'MOUSENEW')
        self.daemon._check_dock_mouse_state()
        self.assertIs(self.daemon._razer_devices[self.child_id].dbus, self.old_mouse)
        self.daemon._check_dock_mouse_state()
        self.assertNotIn('MOUSEOLD', self.daemon._razer_devices)
        self.assertEqual(self.daemon._razer_devices[self.child_id].serial, 'MOUSENEW')
        self.assertEqual(self.daemon._razer_devices[self.dock_id].child_ids, [self.child_id])
        self.old_mouse.close.assert_called_once_with(read_hardware=False)
        self.old_mouse.remove_from_connection.assert_called_once_with()
        self.daemon.device_removed.assert_called_once_with()
        self.daemon.device_added.assert_called_once_with()

    def test_one_unavailable_sample_does_not_remove_child_and_stable_identity_retries_restore(self):
        self.dock.get_active_mouse_identity.side_effect = [None, (0x00AB, 'MOUSEOLD')]
        self.daemon._check_dock_mouse_state()
        self.daemon._check_dock_mouse_state()
        self.assertIn(self.child_id, self.daemon._razer_devices)
        self.assertEqual(self.daemon._dock_mouse_pending, {})
        self.old_mouse.close.assert_not_called()
        self.old_mouse.retry_pending_restore.assert_called_once_with()
        self.old_mouse.read_reported_dpi.assert_called_once_with()
        self.old_mouse.update_dpi_from_report.assert_called_once_with(None)

    def test_link_down_removes_child_without_querying_paired_slots_or_rf(self):
        self.dock.is_mouse_connected.return_value = False
        self.daemon._check_dock_mouse_state()
        self.daemon._check_dock_mouse_state()
        self.assertNotIn(self.child_id, self.daemon._razer_devices)
        self.dock.get_active_mouse_identity.assert_not_called()
        self.old_mouse.close.assert_called_once_with(read_hardware=False)
        self.old_mouse.read_reported_dpi.assert_not_called()

    def test_status_error_interrupts_pending_disconnect(self):
        self.dock.is_mouse_connected.side_effect = [False, OSError(16, 'busy'), False, False]
        self.daemon._check_dock_mouse_state()
        self.assertEqual(self.daemon._dock_mouse_pending[self.dock_id], None)
        self.daemon._check_dock_mouse_state()
        self.assertEqual(self.daemon._dock_mouse_pending, {})
        self.assertIn(self.child_id, self.daemon._razer_devices)
        self.daemon._check_dock_mouse_state()
        self.assertIn(self.child_id, self.daemon._razer_devices)
        self.daemon._check_dock_mouse_state()
        self.assertNotIn(self.child_id, self.daemon._razer_devices)

    def test_two_unavailable_samples_remove_child_and_parent_reference(self):
        self.dock.get_active_mouse_identity.return_value = None
        self.daemon._check_dock_mouse_state()
        self.daemon._check_dock_mouse_state()
        self.assertNotIn(self.child_id, self.daemon._razer_devices)
        self.assertNotIn('MOUSEOLD', self.daemon._razer_devices)
        self.assertEqual(self.daemon._razer_devices[self.dock_id].child_ids, [])
        self.old_mouse.remove_from_connection.assert_called_once_with()
        self.daemon.device_removed.assert_called_once_with()
        self.daemon.device_added.assert_not_called()

    def test_existing_physical_serial_blocks_duplicate_logical_child(self):
        physical_mouse = FakeDockedMouse('MOUSENEW')
        self.daemon._razer_devices.add('physical-mouse', 'MOUSENEW', physical_mouse)
        self.dock.get_active_mouse_identity.return_value = (0x00AB, 'MOUSENEW')
        self.daemon._check_dock_mouse_state()
        self.daemon._check_dock_mouse_state()
        self.assertNotIn(self.child_id, self.daemon._razer_devices)
        self.assertIs(self.daemon._razer_devices['MOUSENEW'].dbus, physical_mouse)
        self.assertEqual(self.daemon._razer_devices[self.dock_id].child_ids, [])
        self.daemon.device_added.assert_not_called()

    def test_identity_change_during_child_discovery_is_not_registered(self):
        self.dock.get_active_mouse_identity.return_value = (0x00AB, 'MOUSEOTHER')
        self.daemon._check_dock_mouse_state()
        self.daemon._check_dock_mouse_state()
        self.assertNotIn(self.child_id, self.daemon._razer_devices)
        self.assertNotIn('MOUSENEW', self.daemon._razer_devices)
        self.daemon.device_added.assert_not_called()


class DockRestoreTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.path = Path(self.tmp.name)
        self.mouse = object.__new__(RazerBasiliskV3ProDocked)
        self.mouse._is_closed = True
        self.mouse._device_path = self.tmp.name
        self.mouse.storage_name = 'TESTMOUSE01'
        self.mouse.logger = Mock()
        self.mouse.persistence = configparser.ConfigParser()
        self.mouse.persistence.status = {'changed': False}
        self.mouse._testing = False
        self.mouse._disable_persistence = False
        self.mouse.zone = {}
        self.mouse.ZONES = ()
        self.mouse.dpi = [1800, 1800]
        self.mouse.poll_rate = 500
        self.mouse._dock_dpi_known = False
        self.mouse._dock_poll_rate_known = False
        self.mouse.getDPI = types.MethodType(mamba.get_dpi_xy, self.mouse)
        self.mouse.setDPI = types.MethodType(mamba.set_dpi_xy, self.mouse)
        self.mouse.getPollRate = types.MethodType(mamba.get_poll_rate, self.mouse)
        self.mouse.setPollRate = types.MethodType(mamba.set_poll_rate, self.mouse)

    def persisted_values(self):
        daemon = object.__new__(RazerDaemon)
        daemon.logger = Mock()
        daemon._persistence = self.mouse.persistence
        daemon._razer_devices = DeviceCollection()
        daemon._razer_devices.add('dock:mouse', self.mouse.storage_name, self.mouse)
        persistence_path = self.path / 'persistence.conf'
        daemon.write_persistence(str(persistence_path))
        reloaded = configparser.ConfigParser()
        reloaded.read(persistence_path)
        return dict(reloaded[self.mouse.storage_name])

    def test_first_discovery_reads_hardware_without_writing_defaults(self):
        (self.path / 'dpi').write_text('800:800\n')
        (self.path / 'poll_rate').write_text('1000\n')
        self.mouse.restore_dpi_poll_rate()
        self.assertEqual(self.mouse.dpi, [800, 800])
        self.assertEqual(self.mouse.poll_rate, 1000)
        self.assertEqual((self.path / 'dpi').read_text(), '800:800\n')
        self.assertEqual((self.path / 'poll_rate').read_text(), '1000\n')
        self.assertEqual(self.persisted_values(), {'dpi_x': '800', 'dpi_y': '800', 'poll_rate': '1000'})

    def test_unavailable_first_read_does_not_persist_synthetic_defaults(self):
        self.mouse.getDPI = Mock(side_effect=OSError('RF unavailable'))
        self.mouse.getPollRate = Mock(side_effect=OSError('RF unavailable'))
        self.mouse.restore_dpi_poll_rate()
        self.assertFalse(self.mouse._dock_dpi_known)
        self.assertFalse(self.mouse._dock_poll_rate_known)
        self.assertEqual(self.persisted_values(), {})

    def test_failed_saved_restore_keeps_intent_until_retry_succeeds(self):
        self.mouse.persistence[self.mouse.storage_name] = {'dpi_x': '900', 'dpi_y': '900', 'poll_rate': '1000'}
        self.mouse.dpi = [900, 900]
        self.mouse.poll_rate = 1000
        self.mouse.setDPI = Mock(side_effect=[OSError('RF unavailable'), None])
        self.mouse.setPollRate = Mock(side_effect=[RuntimeError('Readback was 500'), None])
        self.mouse.getDPI = Mock(return_value=[800, 800])
        self.mouse.getPollRate = Mock(return_value=500)
        self.mouse.restore_dpi_poll_rate()
        self.assertTrue(self.mouse._dock_restore_dpi)
        self.assertTrue(self.mouse._dock_restore_poll_rate)
        self.mouse.getDPI.assert_not_called()
        self.mouse.getPollRate.assert_not_called()
        self.assertEqual(self.persisted_values(), {'dpi_x': '900', 'dpi_y': '900', 'poll_rate': '1000'})
        self.mouse.retry_pending_restore()
        self.assertFalse(self.mouse._dock_restore_dpi)
        self.assertFalse(self.mouse._dock_restore_poll_rate)
        self.assertEqual(self.mouse.setDPI.call_count, 2)
        self.assertEqual(self.mouse.setPollRate.call_count, 2)
        self.mouse.setDPI.assert_called_with(900, 900)
        self.mouse.setPollRate.assert_called_with(1000)

    def test_reported_dpi_wins_over_saved_restore_without_rf_read(self):
        self.mouse.persistence[self.mouse.storage_name] = {'dpi_x': '900', 'dpi_y': '900'}
        self.mouse.dpi = [900, 900]
        (self.path / 'mouse_reported_dpi').write_text('1200:1200\n')
        self.mouse.setDPI = Mock()
        self.mouse.getDPI = Mock()
        self.mouse.restore_dpi_poll_rate()
        self.assertEqual(self.mouse.dpi, [1200, 1200])
        self.assertFalse(self.mouse._dock_restore_dpi)
        self.mouse.setDPI.assert_not_called()
        self.mouse.getDPI.assert_not_called()
        self.assertEqual(self.persisted_values(), {'dpi_x': '1200', 'dpi_y': '1200'})

    def test_failed_dpi_write_does_not_change_saved_value(self):
        self.mouse.dpi = [800, 800]
        with patch('openrazer_daemon.dbus_services.dbus_methods.mamba.open', side_effect=OSError('RF unavailable')):
            with self.assertRaises(OSError):
                self.mouse.setDPI(900, 900)
        self.assertEqual(self.mouse.dpi, [800, 800])
        self.assertFalse(self.mouse.persistence.status['changed'])
        self.assertEqual(self.mouse.zone, {})

    def test_polling_getter_reads_actual_rate_but_retains_pending_saved_intent(self):
        self.mouse.poll_rate = 1000
        self.mouse._dock_restore_poll_rate = True
        (self.path / 'poll_rate').write_text('500\n')
        self.assertEqual(self.mouse.getPollRate(), 500)
        self.assertEqual(self.mouse.poll_rate, 1000)
        self.mouse._dock_restore_poll_rate = False
        self.assertEqual(self.mouse.getPollRate(), 500)
        self.assertEqual(self.mouse.poll_rate, 500)


if __name__ == '__main__':
    unittest.main()
