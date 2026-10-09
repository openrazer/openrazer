# SPDX-License-Identifier: GPL-2.0-or-later

"""Mouse Dock Pro lifecycle and failed RF transaction regressions."""

import configparser
from pathlib import Path
import tempfile
import threading
import types
import unittest
from unittest.mock import Mock, mock_open, patch

from openrazer_daemon.daemon import RazerDaemon
from openrazer_daemon.device import DeviceCollection
from openrazer_daemon.dbus_services.dbus_methods import mamba
from openrazer_daemon.hardware.accessory import RazerMouseDockPro
from openrazer_daemon.hardware.device_base import RazerDevice
from openrazer_daemon.hardware.mouse import (
    DockedMouseNotReady, RazerBasiliskV3ProDocked, RazerBasiliskV3ProWireless,
    RazerBasiliskV3Pro35KDocked, RazerBasiliskV3Pro35KWireless, RazerBasiliskV3Pro35KWired,
    RazerBasiliskV3Pro35KPhantomGreenEditionDocked, RazerBasiliskV3Pro35KPhantomGreenEditionWireless,
    RazerBasiliskV3Pro35KPhantomGreenEditionWired,
    RazerCobraHyperSpeedDocked, RazerCobraHyperSpeedWireless, RazerCobraHyperSpeed,
    RazerCobraProDocked, RazerCobraProWireless, RazerCobraProWired, RazerNagaV2ProDocked,
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
        self.daemon._device_lock = threading.RLock()
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

    def test_physical_serial_takes_over_logical_child_path(self):
        self.daemon._release_dock_child_for_serial('MOUSEOLD')
        self.assertNotIn('MOUSEOLD', self.daemon._razer_devices)
        self.assertEqual(self.daemon._razer_devices[self.dock_id].child_ids, [])
        self.old_mouse.remove_from_connection.assert_called_once_with()

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

    def test_unavailable_physical_serial_preserves_same_model_dock_children(self):
        other_dock_id = '0003:1532:00A4.0002'
        other_child_id = other_dock_id + ':mouse'
        other_mouse = FakeDockedMouse('MOUSEOTHER')
        self.daemon._razer_devices.add(other_child_id, 'MOUSEOTHER', other_mouse)
        (Path(self.tmp.name) / 'device_serial').write_text('')

        def check_registration(physical, _object_path):
            physical._is_closed = True
            self.assertTrue(physical.serial.startswith('UNKNOWN_'))
            self.assertIn(self.child_id, self.daemon._razer_devices)
            self.assertIn(other_child_id, self.daemon._razer_devices)
            raise RuntimeError('registration reached')

        with patch('openrazer_daemon.hardware.device_base.DBusService.__init__', autospec=True, side_effect=check_registration), \
                patch('openrazer_daemon.hardware.device_base.time.sleep'):
            with self.assertRaisesRegex(RuntimeError, 'registration reached'):
                RazerBasiliskV3ProWireless(
                    device_path=self.tmp.name, device_number=2, config=self.daemon._config,
                    persistence=self.daemon._persistence, testing=True,
                    additional_interfaces=None, additional_methods=[], unknown_serial_counter={},
                    prepare_registration=self.daemon._release_dock_child_for_serial,
                )
        self.old_mouse.close.assert_not_called()
        other_mouse.close.assert_not_called()

    def test_constructor_serial_controls_handover_for_startup_and_hotplug(self):
        class PhysicalMouse(RazerDevice):
            USB_VID = 0x1532
            USB_PID = 0x00AA

            @staticmethod
            def match(_sys_name, _sys_path):
                return True

        self.daemon._device_classes = [PhysicalMouse]
        self.daemon._udev_context = Mock()
        physical = types.SimpleNamespace(sys_name='0003:1532:00AA.0003', sys_path=self.tmp.name)
        (Path(self.tmp.name) / 'device_type').write_text('Basilisk')
        (Path(self.tmp.name) / 'device_serial').write_text('')

        def check_registration(mouse, object_path):
            mouse._is_closed = True
            self.assertEqual(object_path, '/org/razer/device/MOUSEOLD')
            self.assertNotIn(self.child_id, self.daemon._razer_devices)
            self.assertEqual(self.daemon._razer_devices[self.dock_id].child_ids, [])
            raise RuntimeError('registration reached')

        for route in ('startup', 'hotplug'):
            with self.subTest(route=route):
                if self.child_id not in self.daemon._razer_devices:
                    self.daemon._razer_devices.add(self.child_id, 'MOUSEOLD', self.old_mouse)
                    self.daemon._razer_devices[self.dock_id].child_ids.append(self.child_id)
                with patch.object(PhysicalMouse, '_read_driver_serial', return_value='MOUSEOLD'), \
                        patch('openrazer_daemon.hardware.device_base.DBusService.__init__', autospec=True, side_effect=check_registration), \
                        patch('openrazer_daemon.daemon.time.sleep'):
                    with self.assertRaisesRegex(RuntimeError, 'registration reached'):
                        if route == 'startup':
                            self.daemon._test_dir = None
                            self.daemon._udev_context.list_devices.return_value = [physical]
                            self.daemon._load_devices()
                        else:
                            self.daemon._add_device(physical)

    def test_malformed_physical_serial_does_not_crash_or_release_other_mouse(self):
        (Path(self.tmp.name) / 'device_serial').write_bytes(b'\xff\xff\n')
        mouse = object.__new__(RazerBasiliskV3ProWireless)
        mouse._is_closed = True
        mouse._serial = None
        mouse._device_path = self.tmp.name
        mouse._unknown_serial_counter = {}
        mouse.logger = Mock()
        with patch('openrazer_daemon.hardware.device_base.time.sleep'):
            serial = mouse.get_serial()
        self.daemon._release_dock_child_for_serial(serial)
        self.assertTrue(serial.startswith('UNKNOWN_'))
        self.assertIn(self.child_id, self.daemon._razer_devices)
        self.old_mouse.close.assert_not_called()

    def test_monitor_waits_until_physical_registration_finishes(self):
        constructing = threading.Event()
        release = threading.Event()
        armed = threading.Event()
        checked = threading.Event()
        errors = []
        self.daemon._collecting_udev_devices = [types.SimpleNamespace(sys_path=self.tmp.name)]

        def construct(_device):
            constructing.set()
            if not release.wait(5):
                raise RuntimeError('registration was never released')

        class StopMonitoring(Exception):
            pass

        def monitor():
            try:
                self.daemon._dock_mouse_monitor_loop()
            except StopMonitoring:
                pass
            except Exception as error:
                errors.append(error)

        self.daemon._add_device = Mock(side_effect=construct)
        self.daemon._arm_dock_mouse_notifications = Mock(side_effect=lambda: (armed.set(), [])[1])
        self.daemon._check_dock_mouse_state = Mock(side_effect=checked.set)
        self.daemon._wait_for_dock_mouse_event = Mock(side_effect=StopMonitoring)
        collector = threading.Thread(target=self.daemon._collecting_udev_method, args=(None,))
        monitor_thread = threading.Thread(target=monitor)
        with patch('openrazer_daemon.daemon.time.sleep'):
            collector.start()
            self.addCleanup(collector.join, 5)
            self.addCleanup(release.set)
            self.assertTrue(constructing.wait(5))
            monitor_thread.start()
            self.addCleanup(monitor_thread.join, 5)
            self.addCleanup(release.set)
            self.assertTrue(armed.wait(5))
            self.assertFalse(checked.wait(0.1))
            release.set()
            collector.join(5)
            monitor_thread.join(5)
        self.assertFalse(collector.is_alive())
        self.assertFalse(monitor_thread.is_alive())
        self.assertEqual(errors, [])
        self.assertTrue(checked.is_set())

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

    def test_original_basilisk_dock_advertises_8k_without_broadening_other_models(self):
        self.assertIn('get_supported_poll_rates', self.mouse.METHODS)
        self.assertEqual(mamba.get_supported_poll_rates(self.mouse), [125, 500, 1000, 2000, 4000, 8000])
        self.assertNotIn(8000, RazerBasiliskV3ProWireless.POLL_RATES or [])
        self.assertNotIn(8000, RazerNagaV2ProDocked.POLL_RATES)

    def assert_docked_8k_profile(self, docked, wireless, wired):
        registry = dict(RazerMouseDockPro._get_wireless_pid_registry())
        self.assertIs(registry[wireless.USB_PID], docked)
        dock = object.__new__(RazerMouseDockPro)
        dock._is_closed = True
        dock._get_wireless_pid_registry = Mock(return_value=registry)
        dock.get_active_mouse_identity = Mock(return_value=(wireless.USB_PID, 'MOUSESERIAL'))
        self.assertEqual(dock.get_child_devices(), [
            (docked, {'id_suffix': ':mouse', 'serial': 'MOUSESERIAL'}),
        ])
        self.assertEqual(docked.USB_PID, RazerMouseDockPro.USB_PID)
        self.assertIn('get_supported_poll_rates', docked.METHODS)
        self.assertEqual(docked.POLL_RATES, [125, 500, 1000, 2000, 4000, 8000])
        for direct in (wired, wireless):
            self.assertEqual(direct.POLL_RATES or [125, 500, 1000], [125, 500, 1000])
        mouse = object.__new__(docked)
        mouse._is_closed = True
        mouse.logger = Mock()
        mouse.poll_rate = 1000
        mouse._device_path = self.tmp.name
        poll_rate_path = self.path / 'poll_rate'
        poll_rate_path.write_text('1000\n')
        mamba.set_poll_rate(mouse, 8000)
        self.assertEqual(poll_rate_path.read_text(), '8000')
        self.assertEqual(mamba.get_poll_rate(mouse), 8000)
        self.assertEqual(mamba.get_supported_poll_rates(mouse), docked.POLL_RATES)

    def test_basilisk_35k_dock_profile_keeps_direct_mode_unchanged(self):
        self.assert_docked_8k_profile(RazerBasiliskV3Pro35KDocked, RazerBasiliskV3Pro35KWireless, RazerBasiliskV3Pro35KWired)

    def test_additional_dock_8k_profiles_keep_direct_modes_unchanged(self):
        for docked, wireless, wired in (
                (RazerBasiliskV3Pro35KPhantomGreenEditionDocked,
                 RazerBasiliskV3Pro35KPhantomGreenEditionWireless,
                 RazerBasiliskV3Pro35KPhantomGreenEditionWired),
                (RazerCobraProDocked, RazerCobraProWireless, RazerCobraProWired),
                (RazerCobraHyperSpeedDocked, RazerCobraHyperSpeedWireless, RazerCobraHyperSpeed)):
            with self.subTest(model=docked.DEVICE_NAME):
                self.assert_docked_8k_profile(docked, wireless, wired)
        self.assertNotIn(8000, RazerNagaV2ProDocked.POLL_RATES)

    def test_successful_polling_write_is_read_back_before_becoming_saved_state(self):
        driver_file = mock_open(read_data='1000\n')
        with patch('openrazer_daemon.dbus_services.dbus_methods.mamba.open', driver_file):
            self.mouse.setPollRate(1000)
        self.assertEqual([args[0][1] for args in driver_file.call_args_list], ['w', 'r'])
        driver_file().write.assert_called_once_with('1000')
        self.assertEqual(self.mouse.poll_rate, 1000)
        self.assertTrue(self.mouse._dock_poll_rate_known)
        self.assertEqual(self.persisted_values(), {'poll_rate': '1000'})

    def test_rejected_polling_readback_preserves_previous_polling_state(self):
        self.mouse.poll_rate = 500
        self.mouse._dock_poll_rate_known = True
        with patch('openrazer_daemon.dbus_services.dbus_methods.mamba.open', mock_open(read_data='500\n')):
            with self.assertRaisesRegex(RuntimeError, 'reported polling rate 500'):
                self.mouse.setPollRate(1000)
        self.assertEqual(self.mouse.poll_rate, 500)
        self.assertEqual(self.persisted_values(), {'poll_rate': '500'})

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
