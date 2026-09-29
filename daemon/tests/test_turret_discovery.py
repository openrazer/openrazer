# SPDX-License-Identifier: GPL-2.0-or-later

"""
Razer Turret wired and receiver discovery, observed at the daemon's boundaries.

The real daemon objects are served on a private dbus-daemon through their own
connection; the process session bus, its environment and the default D-Bus main
loop are left alone. udev, sysfs, GLib timers, hotplug threads and the clock are
faked, so no hardware or host bus is used.
"""
import configparser
import logging
import os
import shutil
import subprocess
import tempfile
import threading
import unittest
import unittest.mock
import xml.etree.ElementTree
from pathlib import Path

import dbus
import dbus.bus
import dbus.mainloop.glib
import dbus.service
from gi.repository import GLib

import openrazer_daemon.daemon
from openrazer._fake_driver import SPECS, FakeDevice
from openrazer_daemon.misc.battery_notifier import BatteryNotifier

KEYBOARD_SERIAL = 'TURRETKBD0001'
MOUSE_SERIAL = 'TURRETMOUSE0001'

WIRED_KEYBOARD = '0003:1532:023E.0001'
WIRED_KEYBOARD_INPUT = '0003:1532:023E.0002'
WIRED_MOUSE = '0003:1532:0075.0003'
RECEIVER_MOUSE = '0003:1532:0904.0004'
RECEIVER_KEYBOARD = '0003:1532:0904.0005'
RECEIVER_INPUT = '0003:1532:0904.0006'
LEGACY_MOUSE = '0003:1532:0084.0007'
LEGACY_SERIAL = 'XX0000000084'

WIRED_KEYBOARD_NAME = 'Razer Turret Keyboard for Xbox One (Wired)'
WIRED_MOUSE_NAME = 'Razer Turret Mouse for Xbox One (Wired)'
RECEIVER_KEYBOARD_NAME = 'Razer Turret Keyboard for Xbox One (Wireless)'
RECEIVER_MOUSE_NAME = 'Razer Turret Mouse for Xbox One (Wireless)'

# The generated fake driver files list the attributes each driver exposes
TURRET_SPECS = {WIRED_KEYBOARD_NAME: 'razerturretkeyboardforxboxonewired',
                WIRED_MOUSE_NAME: 'razerturretmouseforxboxonewired',
                RECEIVER_KEYBOARD_NAME: 'razerturretkeyboardforxboxonewireless',
                RECEIVER_MOUSE_NAME: 'razerturretmouseforxboxonewireless'}

# Enough 5 second ticks to pass the longest retry back-off
ROLE_RETRY_TICKS = 13

# Blocking D-Bus calls from the tests, in seconds
CALL_TIMEOUT = 10

_BUS = {}
_LOGGER = {}


def setUpModule():
    if shutil.which('dbus-daemon') is None:
        raise unittest.SkipTest('dbus-daemon is required')
    process = subprocess.Popen(['dbus-daemon', '--session', '--nofork', '--print-address'],
                               stdout=subprocess.PIPE, text=True)
    address = process.stdout.readline().strip()

    # Process-wide and permanent, but needed for D-Bus use from several threads
    dbus.mainloop.glib.threads_init()
    daemon_bus = dbus.bus.BusConnection(address, mainloop=dbus.mainloop.glib.DBusGMainLoop())
    loop = GLib.MainLoop()
    thread = threading.Thread(target=loop.run, daemon=True)
    thread.start()
    _BUS.update(process=process, loop=loop, thread=thread, daemon=daemon_bus,
                client=dbus.bus.BusConnection(address), session_address=os.environ.get('DBUS_SESSION_BUS_ADDRESS'))

    # The daemon configures its logger; without a log dir or console it has no handler
    logger = logging.getLogger('razer')
    _LOGGER.update(handlers=list(logger.handlers), level=logger.level, propagate=logger.propagate)
    logger.addHandler(logging.NullHandler())


def tearDownModule():
    if _LOGGER:
        logger = logging.getLogger('razer')
        logger.handlers[:] = _LOGGER['handlers']
        logger.setLevel(_LOGGER['level'])
        logger.propagate = _LOGGER['propagate']
    if _BUS:
        _BUS['client'].close()
        _BUS['daemon'].close()
        _BUS['loop'].quit()
        _BUS['thread'].join(timeout=5)
        _BUS['process'].terminate()
        _BUS['process'].wait(timeout=5)


def battery_notifiers(device_name):
    # The notifier threads are the daemon's per-device resources
    return [thread for thread in threading.enumerate()
            if isinstance(thread, BatteryNotifier) and thread._device_name == device_name]  # pylint: disable=protected-access


def remove_path(path):
    if path.is_dir():
        path.rmdir()
    elif path.exists():
        path.unlink()


class FakeSysfs:
    """HID devices as the drivers expose them under /sys/bus/hid/devices."""

    def __init__(self, root):
        self.root = Path(root)

    def path(self, name):
        return self.root / name

    def names(self):
        return sorted(path.name for path in self.root.iterdir())

    def add_keyboard(self, name, device_type, serial=KEYBOARD_SERIAL):
        self.add_fake_driver(name, TURRET_SPECS[device_type], serial)

    def add_mouse(self, name, device_type, serial=MOUSE_SERIAL):
        self.add_fake_driver(name, TURRET_SPECS[device_type], serial)

    def add_input_only(self, name):
        self.path(name).mkdir()

    def add_fake_driver(self, name, spec_name, serial):
        """Create the attributes listed in an upstream fake driver file."""
        config = configparser.ConfigParser()
        config.read(SPECS[spec_name])
        path = self.path(name)
        path.mkdir()
        for line in config['device']['files'].splitlines():
            if line.strip():
                chmod, filename, default, _perm = FakeDevice.parse_endpoint_line(line.strip())
                FakeDevice.create_endpoint(str(path / filename), chmod, default)
        self.set_serial(name, serial)

    def set_serial(self, name, serial):
        """A str/bytes value is the attribute content; None makes reads fail like an asleep role."""
        serial_path = self.path(name) / 'device_serial'
        if serial_path.is_dir():
            serial_path.rmdir()
        elif serial_path.exists():
            serial_path.unlink()
        if serial is None:
            serial_path.mkdir()
        elif isinstance(serial, bytes):
            serial_path.write_bytes(serial)
        else:
            serial_path.write_text(serial + '\n')

    def remove(self, name):
        shutil.rmtree(self.path(name))

    def read(self, name, filename):
        return (self.path(name) / filename).read_text()

    def break_attribute(self, name, filename):
        """Writes and reads fail with an OSError other than FileNotFoundError."""
        path = self.path(name) / filename
        path.unlink()
        path.mkdir()

    def restore_attribute(self, name, filename, value=''):
        path = self.path(name) / filename
        path.rmdir()
        path.write_text(value)

    def snapshot(self, name):
        return {path.name: path.read_bytes() for path in self.path(name).iterdir() if path.is_file()}

    def restore_dpi(self, name):
        """The driver reads DPI back as text after binary writes."""
        (self.path(name) / 'dpi').write_text('1800:1800\n')

    def clear(self, name, filename):
        (self.path(name) / filename).write_text('')

    def exists(self, name, filename):
        return (self.path(name) / filename).exists()


class FakeUdevDevice:
    def __init__(self, sysfs, name, action=None):
        self.sys_name = name
        self.sys_path = str(sysfs.path(name))
        self.device_path = '/devices/fake/' + name
        self.action = action


class FakeUdev:
    """pyudev Context and MonitorObserver: enumeration order and events are test controlled."""

    def __init__(self, sysfs):
        self.sysfs = sysfs
        self.order = None
        self.callback = None

    def list_devices(self, subsystem):
        assert subsystem == 'hid'
        return [FakeUdevDevice(self.sysfs, name) for name in (self.order or self.sysfs.names())]

    def observer(self, _monitor, callback, name):
        self.callback = callback
        self.monitor_observer = unittest.mock.MagicMock()
        return self.monitor_observer

    def emit(self, action, *names):
        for name in names:
            self.callback(FakeUdevDevice(self.sysfs, name, action))


class FakeClock:
    def __init__(self):
        self.now = 1000.0

    def monotonic(self):
        return self.now

    def sleep(self, seconds):
        self.now += seconds


class FakeTimers:
    """GLib.timeout_add_seconds sources, fired on demand."""

    def __init__(self, clock):
        self.clock = clock
        self.sources = {}
        self.next_id = 1

    def timeout_add_seconds(self, interval, callback, *args):
        source_id = self.next_id
        self.next_id += 1
        self.sources[source_id] = (interval, callback, args)
        return source_id

    def source_remove(self, source_id):
        return self.sources.pop(source_id, None) is not None

    def fire(self):
        """Advance the clock by one interval and run every due source once."""
        for source_id, (interval, callback, args) in list(self.sources.items()):
            self.clock.now += interval
            if source_id in self.sources and not callback(*args):
                self.sources.pop(source_id, None)

    @property
    def active(self):
        return bool(self.sources)


class DeferredThreads:
    """The daemon's threading module, except that new threads start when run() is called."""

    def __init__(self):
        self.pending = []

    def __getattr__(self, name):
        return getattr(threading, name)

    def Thread(self, target, args=(), kwargs=None, **_ignored):  # pylint: disable=invalid-name
        pending = self.pending

        class _Thread:
            daemon = False

            def start(self):
                pending.append((target, args, kwargs or {}))
        return _Thread()

    def run(self):
        while self.pending:
            target, args, kwargs = self.pending.pop(0)
            target(*args, **kwargs)


class TurretDiscoveryTest(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory(prefix='turret-discovery-')
        self.addCleanup(tmp.cleanup)
        self.sysfs = FakeSysfs(tmp.name)
        self.udev = FakeUdev(self.sysfs)
        self.clock = FakeClock()
        self.timers = FakeTimers(self.clock)
        self.threads = DeferredThreads()
        self.daemon = None

        patches = [
            unittest.mock.patch.object(openrazer_daemon.daemon, 'Context', lambda: self.udev),
            unittest.mock.patch.object(openrazer_daemon.daemon, 'Monitor'),
            unittest.mock.patch.object(openrazer_daemon.daemon, 'MonitorObserver', self.udev.observer),
            unittest.mock.patch.object(openrazer_daemon.daemon, 'time', self.clock),
            unittest.mock.patch.object(GLib, 'timeout_add_seconds', self.timers.timeout_add_seconds),
            unittest.mock.patch.object(GLib, 'source_remove', self.timers.source_remove),
            unittest.mock.patch('getpass.getuser', return_value='root'),
            unittest.mock.patch('setproctitle.setproctitle'),
            # Process-wide D-Bus and signal state belongs to the test process
            unittest.mock.patch.object(dbus, 'SessionBus', lambda: _BUS['daemon']),
            unittest.mock.patch.object(dbus.mainloop.glib, 'DBusGMainLoop'),
            unittest.mock.patch('signal.signal'),
            unittest.mock.patch.object(GLib, 'idle_add'),
        ]
        for patch in patches:
            patch.start()
            self.addCleanup(patch.stop)

    def tearDown(self):
        if self.daemon is None:
            return
        # Unplug everything: sysfs disappears before udev reports removal
        names = self.sysfs.names()
        for name in names:
            self.sysfs.remove(name)
        self.udev.emit('remove', *names)
        self.threads.run()
        self.daemon.stop()
        self.daemon.remove_from_connection()
        self.assertEqual(self.exported_paths(), [])

    # Boundaries
    def start_daemon(self, order=None, persistence_file=None):
        self.udev.order = order
        self.daemon = openrazer_daemon.daemon.RazerDaemon(persistence_file=persistence_file)
        # Hotplug batches start from here on
        patch = unittest.mock.patch.object(openrazer_daemon.daemon, 'threading', self.threads)
        patch.start()
        self.addCleanup(patch.stop)

    def plug(self, *names):
        """udev add events, batched like the daemon's 2 second collection window."""
        self.udev.emit('add', *names)
        self.threads.run()

    def unplug(self, *names):
        for name in names:
            self.sysfs.remove(name)
        self.udev.emit('remove', *names)
        self.threads.run()

    # Observations, all through a separate client connection
    def remote(self, path):
        # The unique name still answers after the daemon object itself is removed
        return _BUS['client'].get_object(_BUS['daemon'].get_unique_name(), path, introspect=False)

    def serials(self):
        daemon = self.remote('/org/razer')
        return sorted(str(serial) for serial in daemon.getDevices(dbus_interface='razer.devices', timeout=CALL_TIMEOUT))

    def exported_paths(self):
        introspection = self.remote('/org/razer/device').Introspect(dbus_interface='org.freedesktop.DBus.Introspectable', timeout=CALL_TIMEOUT)
        return sorted(node.get('name') for node in xml.etree.ElementTree.fromstring(str(introspection)).findall('node'))

    def call(self, serial, method, *args, interface='razer.device.misc'):
        device = self.remote('/org/razer/device/' + serial)
        return device.get_dbus_method(method, interface)(*args, timeout=CALL_TIMEOUT)

    def name_of(self, serial):
        return str(self.call(serial, 'getDeviceName'))

    def roles(self):
        return {serial: self.name_of(serial) for serial in self.serials()}

    def methods_of(self, serial):
        device = self.remote('/org/razer/device/' + serial)
        tree = xml.etree.ElementTree.fromstring(str(device.Introspect(dbus_interface='org.freedesktop.DBus.Introspectable', timeout=CALL_TIMEOUT)))
        return {(interface.get('name'), method.get('name'))
                for interface in tree.iter('interface') for method in interface.iter('method')
                if interface.get('name').startswith('razer.')}

    # Tests
    def test_daemon_uses_only_its_private_bus(self):
        self.sysfs.add_keyboard(RECEIVER_KEYBOARD, RECEIVER_KEYBOARD_NAME)
        self.start_daemon()

        self.assertEqual(self.serials(), [KEYBOARD_SERIAL])
        self.assertEqual(str(self.remote('/org/razer').version(dbus_interface='razer.daemon', timeout=CALL_TIMEOUT)),
                         openrazer_daemon.daemon.__version__)
        self.assertTrue(_BUS['client'].name_has_owner('org.razer'))
        self.assertEqual(os.environ.get('DBUS_SESSION_BUS_ADDRESS'), _BUS['session_address'])

    def test_receiver_exports_mouse_and_keyboard_roles(self):
        self.sysfs.add_mouse(RECEIVER_MOUSE, RECEIVER_MOUSE_NAME)
        self.sysfs.add_keyboard(RECEIVER_KEYBOARD, RECEIVER_KEYBOARD_NAME)
        self.sysfs.add_input_only(RECEIVER_INPUT)

        self.start_daemon()

        self.assertEqual(self.roles(), {KEYBOARD_SERIAL: RECEIVER_KEYBOARD_NAME, MOUSE_SERIAL: RECEIVER_MOUSE_NAME})
        self.assertEqual(self.exported_paths(), [KEYBOARD_SERIAL, MOUSE_SERIAL])
        self.assertEqual([int(i) for i in self.call(KEYBOARD_SERIAL, 'getVidPid')], [0x1532, 0x0904])
        self.assertEqual(str(self.call(KEYBOARD_SERIAL, 'getDeviceType')), 'keyboard')
        self.assertEqual(str(self.call(MOUSE_SERIAL, 'getDeviceType')), 'mouse')

    def test_receiver_roles_have_the_wired_capabilities(self):
        self.sysfs.add_keyboard(WIRED_KEYBOARD, WIRED_KEYBOARD_NAME, serial='TURRETKBD0002')
        self.sysfs.add_mouse(WIRED_MOUSE, WIRED_MOUSE_NAME, serial='TURRETMOUSE0002')
        self.sysfs.add_mouse(RECEIVER_MOUSE, RECEIVER_MOUSE_NAME)
        self.sysfs.add_keyboard(RECEIVER_KEYBOARD, RECEIVER_KEYBOARD_NAME)
        self.start_daemon()

        self.assertEqual(self.methods_of(KEYBOARD_SERIAL), self.methods_of('TURRETKBD0002'))
        self.assertEqual(self.methods_of(MOUSE_SERIAL), self.methods_of('TURRETMOUSE0002'))
        self.assertIn(('razer.device.lighting.chroma', 'setCustom'), self.methods_of(KEYBOARD_SERIAL))
        self.assertEqual([int(i) for i in self.call(KEYBOARD_SERIAL, 'getMatrixDimensions')], [6, 18])
        self.assertNotIn(('razer.device.lighting.chroma', 'setCustom'), self.methods_of(MOUSE_SERIAL))

    def add_all_turrets(self):
        self.sysfs.add_keyboard(WIRED_KEYBOARD, WIRED_KEYBOARD_NAME)
        self.sysfs.add_input_only(WIRED_KEYBOARD_INPUT)
        self.sysfs.add_mouse(WIRED_MOUSE, WIRED_MOUSE_NAME)
        self.sysfs.add_mouse(RECEIVER_MOUSE, RECEIVER_MOUSE_NAME)
        self.sysfs.add_keyboard(RECEIVER_KEYBOARD, RECEIVER_KEYBOARD_NAME)
        self.sysfs.add_input_only(RECEIVER_INPUT)

    def check_startup_prefers_wired(self, reverse):
        self.add_all_turrets()

        self.start_daemon(order=sorted(self.sysfs.names(), reverse=reverse))

        self.assertEqual(self.roles(), {KEYBOARD_SERIAL: WIRED_KEYBOARD_NAME, MOUSE_SERIAL: WIRED_MOUSE_NAME})
        self.assertEqual(self.exported_paths(), [KEYBOARD_SERIAL, MOUSE_SERIAL])
        # Initialisation restores the effect; the receiver roles were never initialised
        self.assertNotEqual(self.sysfs.read(WIRED_KEYBOARD, 'matrix_effect_spectrum'), '')
        self.assertEqual(self.sysfs.read(RECEIVER_KEYBOARD, 'matrix_effect_spectrum'), '')
        self.assertEqual(self.sysfs.read(RECEIVER_MOUSE, 'matrix_effect_spectrum'), '')

    def test_startup_prefers_wired_with_receiver_enumerated_last(self):
        self.check_startup_prefers_wired(reverse=False)

    def test_startup_prefers_wired_with_receiver_enumerated_first(self):
        self.check_startup_prefers_wired(reverse=True)

    def test_distinct_roles_are_independent(self):
        # Wired keyboard with receiver mouse: one object per physical device
        self.sysfs.add_keyboard(WIRED_KEYBOARD, WIRED_KEYBOARD_NAME)
        self.sysfs.add_mouse(RECEIVER_MOUSE, RECEIVER_MOUSE_NAME)
        self.sysfs.add_keyboard(RECEIVER_KEYBOARD, RECEIVER_KEYBOARD_NAME)
        self.sysfs.add_input_only(RECEIVER_INPUT)

        self.start_daemon()

        self.assertEqual(self.roles(), {KEYBOARD_SERIAL: WIRED_KEYBOARD_NAME, MOUSE_SERIAL: RECEIVER_MOUSE_NAME})

    def test_plugging_wired_hands_over_from_receiver_role(self):
        self.sysfs.add_mouse(RECEIVER_MOUSE, RECEIVER_MOUSE_NAME)
        self.sysfs.add_keyboard(RECEIVER_KEYBOARD, RECEIVER_KEYBOARD_NAME)
        self.sysfs.add_input_only(RECEIVER_INPUT)
        self.start_daemon()
        self.sysfs.clear(RECEIVER_MOUSE, 'matrix_effect_spectrum')

        self.sysfs.add_keyboard(WIRED_KEYBOARD, WIRED_KEYBOARD_NAME)
        self.sysfs.add_input_only(WIRED_KEYBOARD_INPUT)
        self.plug(WIRED_KEYBOARD, WIRED_KEYBOARD_INPUT)

        self.assertEqual(self.roles(), {KEYBOARD_SERIAL: WIRED_KEYBOARD_NAME, MOUSE_SERIAL: RECEIVER_MOUSE_NAME})
        self.assertEqual(self.exported_paths(), [KEYBOARD_SERIAL, MOUSE_SERIAL])
        # The unrelated receiver mouse object was not recreated
        self.assertEqual(self.sysfs.read(RECEIVER_MOUSE, 'matrix_effect_spectrum'), '')

    def test_handover_keeps_serial_and_settings(self):
        persistence_file = self.sysfs.root.parent / (self.sysfs.root.name + '.persistence')
        persistence_file.write_text('')
        self.addCleanup(persistence_file.unlink)
        self.sysfs.add_keyboard(RECEIVER_KEYBOARD, RECEIVER_KEYBOARD_NAME)
        self.start_daemon(persistence_file=str(persistence_file))
        self.call(KEYBOARD_SERIAL, 'setStatic', 0, 255, 0, interface='razer.device.lighting.chroma')

        self.sysfs.add_keyboard(WIRED_KEYBOARD, WIRED_KEYBOARD_NAME)
        self.plug(WIRED_KEYBOARD)

        self.assertEqual(self.roles(), {KEYBOARD_SERIAL: WIRED_KEYBOARD_NAME})
        self.assertEqual(str(self.call(KEYBOARD_SERIAL, 'getSerial')), KEYBOARD_SERIAL)
        self.assertEqual(str(self.call(KEYBOARD_SERIAL, 'getEffect', interface='razer.device.lighting.chroma')), 'static')
        self.assertEqual(self.sysfs.read(WIRED_KEYBOARD, 'matrix_effect_spectrum'), '')
        self.assertEqual((self.sysfs.path(WIRED_KEYBOARD) / 'matrix_effect_static').read_bytes(), bytes([0, 255, 0]))

    def test_unplugging_wired_hands_back_to_receiver_role(self):
        self.add_all_turrets()
        self.start_daemon()

        self.unplug(WIRED_KEYBOARD, WIRED_KEYBOARD_INPUT)

        self.assertEqual(self.roles(), {KEYBOARD_SERIAL: RECEIVER_KEYBOARD_NAME, MOUSE_SERIAL: WIRED_MOUSE_NAME})
        self.assertEqual(self.exported_paths(), [KEYBOARD_SERIAL, MOUSE_SERIAL])

    def test_unplugging_receiver_keeps_wired_objects(self):
        self.add_all_turrets()
        self.start_daemon()
        self.sysfs.clear(WIRED_KEYBOARD, 'matrix_effect_spectrum')
        # Verified serials of exported objects are not queried again
        self.sysfs.set_serial(WIRED_KEYBOARD, None)
        self.sysfs.set_serial(WIRED_MOUSE, None)

        self.unplug(RECEIVER_MOUSE, RECEIVER_KEYBOARD, RECEIVER_INPUT)

        self.assertEqual(self.roles(), {KEYBOARD_SERIAL: WIRED_KEYBOARD_NAME, MOUSE_SERIAL: WIRED_MOUSE_NAME})
        self.assertEqual(self.sysfs.read(WIRED_KEYBOARD, 'matrix_effect_spectrum'), '')
        self.assertFalse(self.timers.active)

    def test_asleep_role_is_retried_until_it_answers(self):
        self.sysfs.add_mouse(RECEIVER_MOUSE, RECEIVER_MOUSE_NAME)
        self.sysfs.add_keyboard(RECEIVER_KEYBOARD, RECEIVER_KEYBOARD_NAME, serial=None)
        self.sysfs.add_input_only(RECEIVER_INPUT)

        self.start_daemon()
        self.assertEqual(self.roles(), {MOUSE_SERIAL: RECEIVER_MOUSE_NAME})
        self.timers.fire()
        self.assertEqual(self.serials(), [MOUSE_SERIAL])

        # The keyboard wakes up, without any udev event
        self.sysfs.set_serial(RECEIVER_KEYBOARD, KEYBOARD_SERIAL)
        for _ in range(ROLE_RETRY_TICKS):
            self.timers.fire()

        self.assertEqual(self.roles(), {KEYBOARD_SERIAL: RECEIVER_KEYBOARD_NAME, MOUSE_SERIAL: RECEIVER_MOUSE_NAME})
        self.assertFalse(self.timers.active)

    def test_unplugging_wired_while_receiver_role_is_asleep(self):
        self.add_all_turrets()
        self.start_daemon()
        self.sysfs.set_serial(RECEIVER_KEYBOARD, None)

        self.unplug(WIRED_KEYBOARD, WIRED_KEYBOARD_INPUT)
        self.assertEqual(self.roles(), {MOUSE_SERIAL: WIRED_MOUSE_NAME})

        # It connects through the receiver a little later
        self.sysfs.set_serial(RECEIVER_KEYBOARD, KEYBOARD_SERIAL)
        for _ in range(ROLE_RETRY_TICKS):
            self.timers.fire()
        self.assertEqual(self.roles(), {KEYBOARD_SERIAL: RECEIVER_KEYBOARD_NAME, MOUSE_SERIAL: WIRED_MOUSE_NAME})

    def test_unplugging_wired_retries_an_asleep_receiver_role_immediately(self):
        self.add_all_turrets()
        self.sysfs.set_serial(RECEIVER_KEYBOARD, None)
        self.start_daemon()
        for _ in range(ROLE_RETRY_TICKS):
            self.timers.fire()

        self.sysfs.set_serial(RECEIVER_KEYBOARD, KEYBOARD_SERIAL)
        self.unplug(WIRED_KEYBOARD, WIRED_KEYBOARD_INPUT)

        self.assertEqual(self.roles(), {KEYBOARD_SERIAL: RECEIVER_KEYBOARD_NAME, MOUSE_SERIAL: WIRED_MOUSE_NAME})

    def test_receiver_role_waits_while_its_wired_device_is_present(self):
        # On its cable, the keyboard leaves its receiver role empty; the mouse is asleep
        self.sysfs.add_keyboard(WIRED_KEYBOARD, WIRED_KEYBOARD_NAME)
        self.sysfs.add_keyboard(RECEIVER_KEYBOARD, RECEIVER_KEYBOARD_NAME, serial=None)
        self.sysfs.add_mouse(RECEIVER_MOUSE, RECEIVER_MOUSE_NAME, serial=None)
        self.start_daemon()
        self.assertEqual(self.roles(), {KEYBOARD_SERIAL: WIRED_KEYBOARD_NAME})

        # Only the mouse role is retried
        with unittest.mock.patch('builtins.open', wraps=open) as opened:
            for _ in range(ROLE_RETRY_TICKS):
                self.timers.fire()
        queried = {Path(call.args[0]).parent.name for call in opened.call_args_list
                   if call.args and str(call.args[0]).endswith('device_serial')}
        self.assertEqual(queried, {RECEIVER_MOUSE})

        # Unplugging the cable queries the keyboard role at once
        self.sysfs.set_serial(RECEIVER_KEYBOARD, KEYBOARD_SERIAL)
        self.unplug(WIRED_KEYBOARD)
        self.assertEqual(self.roles(), {KEYBOARD_SERIAL: RECEIVER_KEYBOARD_NAME})

        self.sysfs.set_serial(RECEIVER_MOUSE, MOUSE_SERIAL)
        for _ in range(ROLE_RETRY_TICKS):
            self.timers.fire()
        self.assertEqual(self.roles(), {KEYBOARD_SERIAL: RECEIVER_KEYBOARD_NAME, MOUSE_SERIAL: RECEIVER_MOUSE_NAME})
        self.assertFalse(self.timers.active)

    def test_wired_device_of_another_kind_does_not_stop_retries(self):
        self.sysfs.add_mouse(WIRED_MOUSE, WIRED_MOUSE_NAME)
        self.sysfs.add_keyboard(RECEIVER_KEYBOARD, RECEIVER_KEYBOARD_NAME, serial=None)
        self.start_daemon()
        self.assertTrue(self.timers.active)

        self.sysfs.set_serial(RECEIVER_KEYBOARD, KEYBOARD_SERIAL)
        for _ in range(ROLE_RETRY_TICKS):
            self.timers.fire()
        self.assertEqual(self.roles(), {KEYBOARD_SERIAL: RECEIVER_KEYBOARD_NAME, MOUSE_SERIAL: WIRED_MOUSE_NAME})
        self.assertFalse(self.timers.active)

    def test_interface_removed_while_its_serial_is_read(self):
        self.sysfs.add_keyboard(RECEIVER_KEYBOARD, RECEIVER_KEYBOARD_NAME, serial=None)
        self.start_daemon()
        self.sysfs.set_serial(RECEIVER_KEYBOARD, KEYBOARD_SERIAL)
        serial_path = str(self.sysfs.path(RECEIVER_KEYBOARD) / 'device_serial')
        real_open = open

        def open_then_unplug(path, *args, **kwargs):
            opened = real_open(path, *args, **kwargs)
            if str(path) == serial_path:
                self.unplug(RECEIVER_KEYBOARD)
            return opened

        with unittest.mock.patch('builtins.open', open_then_unplug):
            self.timers.fire()

        self.assertEqual(self.serials(), [])
        self.assertEqual(self.exported_paths(), [])
        self.assertFalse(self.timers.active)

    def test_shutdown_while_a_serial_is_read(self):
        self.sysfs.add_mouse(RECEIVER_MOUSE, RECEIVER_MOUSE_NAME)
        self.sysfs.add_keyboard(RECEIVER_KEYBOARD, RECEIVER_KEYBOARD_NAME, serial=None)
        self.start_daemon()
        self.sysfs.set_serial(RECEIVER_KEYBOARD, KEYBOARD_SERIAL)
        receiver_keyboard_files = self.sysfs.snapshot(RECEIVER_KEYBOARD)
        # Stopping closes the mouse, which reads the DPI back as text
        self.sysfs.restore_dpi(RECEIVER_MOUSE)

        # The periodic retry blocks in the keyboard's serial read, on a real thread
        serial_path = str(self.sysfs.path(RECEIVER_KEYBOARD) / 'device_serial')
        reading = threading.Event()
        release = threading.Event()
        real_open = open

        def blocking_open(path, *args, **kwargs):
            if str(path) == serial_path:
                reading.set()
                release.wait(CALL_TIMEOUT)
            return real_open(path, *args, **kwargs)

        retry = threading.Thread(target=self.timers.fire)
        with unittest.mock.patch('builtins.open', blocking_open):
            try:
                retry.start()
                self.assertTrue(reading.wait(CALL_TIMEOUT))
                # Devices are still being collected when the daemon stops
                self.sysfs.add_keyboard(WIRED_KEYBOARD, WIRED_KEYBOARD_NAME, serial='TURRETKBD0002')
                self.sysfs.add_fake_driver(LEGACY_MOUSE, 'razerdeathadderv2', LEGACY_SERIAL)
                legacy_files = self.sysfs.snapshot(LEGACY_MOUSE)
                self.udev.emit('add', WIRED_KEYBOARD, LEGACY_MOUSE)
                # Handled by the main loop; must not wait for the blocked read
                self.remote('/org/razer').stop(dbus_interface='razer.daemon', timeout=CALL_TIMEOUT)
            finally:
                release.set()
                retry.join(CALL_TIMEOUT)
        self.assertFalse(retry.is_alive())

        self.threads.run()
        # Later events start no collection at all
        self.udev.emit('add', WIRED_KEYBOARD, LEGACY_MOUSE)
        self.assertEqual(self.threads.pending, [])
        self.remote('/org/razer').stop(dbus_interface='razer.daemon', timeout=CALL_TIMEOUT)

        self.assertEqual(self.serials(), [MOUSE_SERIAL])
        self.assertEqual(self.exported_paths(), [MOUSE_SERIAL])
        self.assertFalse(self.timers.active)
        self.assertEqual(self.sysfs.snapshot(RECEIVER_KEYBOARD), receiver_keyboard_files)
        self.assertEqual(self.sysfs.read(WIRED_KEYBOARD, 'matrix_effect_spectrum'), '')
        self.assertEqual(self.sysfs.snapshot(LEGACY_MOUSE), legacy_files)

    def test_shutdown_before_a_hotplug_device_is_created(self):
        self.start_daemon()
        self.sysfs.add_fake_driver(LEGACY_MOUSE, 'razerdeathadderv2', LEGACY_SERIAL)
        untouched = self.sysfs.snapshot(LEGACY_MOUSE)

        # The batch pauses while matching the device, just before creating it
        legacy_path = str(self.sysfs.path(LEGACY_MOUSE))
        matching = threading.Event()
        release = threading.Event()
        real_listdir = os.listdir

        def blocking_listdir(path='.'):
            if str(path) == legacy_path:
                matching.set()
                release.wait(CALL_TIMEOUT)
            return real_listdir(path)

        self.udev.emit('add', LEGACY_MOUSE)
        collector = threading.Thread(target=self.threads.run)
        with unittest.mock.patch('os.listdir', blocking_listdir):
            try:
                collector.start()
                self.assertTrue(matching.wait(CALL_TIMEOUT))
                self.remote('/org/razer').stop(dbus_interface='razer.daemon', timeout=CALL_TIMEOUT)
            finally:
                release.set()
                collector.join(CALL_TIMEOUT)
        self.assertFalse(collector.is_alive())

        self.assertEqual(self.serials(), [])
        self.assertEqual(self.exported_paths(), [])
        self.assertEqual(self.sysfs.snapshot(LEGACY_MOUSE), untouched)

    def test_shutdown_waits_for_a_hotplug_device_being_created(self):
        self.start_daemon()
        self.sysfs.add_fake_driver(LEGACY_MOUSE, 'razerdeathadderv2', LEGACY_SERIAL)
        self.sysfs.add_keyboard(WIRED_KEYBOARD, WIRED_KEYBOARD_NAME)
        wired_keyboard_files = self.sysfs.snapshot(WIRED_KEYBOARD)

        # The batch blocks while creating the older device, on a real thread
        serial_path = str(self.sysfs.path(LEGACY_MOUSE) / 'device_serial')
        reading = threading.Event()
        release = threading.Event()
        real_open = open

        def blocking_open(path, *args, **kwargs):
            if str(path) == serial_path:
                reading.set()
                release.wait(CALL_TIMEOUT)
            return real_open(path, *args, **kwargs)

        def stop():
            self.remote('/org/razer').stop(dbus_interface='razer.daemon', timeout=CALL_TIMEOUT)

        self.udev.emit('add', LEGACY_MOUSE, WIRED_KEYBOARD)
        collector = threading.Thread(target=self.threads.run)
        stopping = threading.Thread(target=stop)
        with unittest.mock.patch('builtins.open', blocking_open):
            try:
                collector.start()
                self.assertTrue(reading.wait(CALL_TIMEOUT))
                stopping.start()
                # Stopping waits until the device is created, like any other device I/O
                stopping.join(0.5)
                self.assertTrue(stopping.is_alive())
            finally:
                release.set()
                collector.join(CALL_TIMEOUT)
                stopping.join(CALL_TIMEOUT)
        self.assertFalse(collector.is_alive())
        self.assertFalse(stopping.is_alive())

        # Created before stopping, so kept (and closed) like devices found earlier.
        # The Turret of the same batch was still waiting to be exported.
        self.assertEqual(self.serials(), [LEGACY_SERIAL])
        self.assertEqual(self.exported_paths(), [LEGACY_SERIAL])
        self.assertEqual(self.sysfs.snapshot(WIRED_KEYBOARD), wired_keyboard_files)

    def test_stopping_continues_past_failing_devices(self):
        persistence_file = self.sysfs.root.parent / (self.sysfs.root.name + '.persistence')
        persistence_file.write_text('')
        self.addCleanup(remove_path, persistence_file)
        driver_mode_mouse = '0003:1532:00B9.0008'
        self.sysfs.add_fake_driver(driver_mode_mouse, 'razerbasiliskv3xhyperspeed', 'XX00000000B9')
        self.sysfs.add_mouse(RECEIVER_MOUSE, RECEIVER_MOUSE_NAME)
        self.sysfs.add_keyboard(RECEIVER_KEYBOARD, RECEIVER_KEYBOARD_NAME, serial=None)
        self.start_daemon(persistence_file=str(persistence_file))
        self.assertTrue(self.timers.active)
        notifiers = battery_notifiers(RECEIVER_MOUSE_NAME)
        self.assertEqual(len(notifiers), 1)
        # Resuming and closing set the device mode, writing persistence fails
        self.sysfs.break_attribute(driver_mode_mouse, 'device_mode')
        persistence_file.unlink()
        persistence_file.mkdir()
        self.sysfs.clear(RECEIVER_MOUSE, 'matrix_brightness')

        self.remote('/org/razer').stop(dbus_interface='razer.daemon', timeout=CALL_TIMEOUT)

        self.assertTrue(self.udev.monitor_observer.send_stop.called)
        self.assertFalse(self.timers.active)
        # The Turret was still resumed and closed
        self.assertNotEqual(self.sysfs.read(RECEIVER_MOUSE, 'matrix_brightness'), '')
        self.assertFalse(any(thread.is_alive() for thread in notifiers))
        # Still stopped: nothing new is exported
        self.sysfs.set_serial(RECEIVER_KEYBOARD, KEYBOARD_SERIAL)
        self.sysfs.add_keyboard(WIRED_KEYBOARD, WIRED_KEYBOARD_NAME, serial='TURRETKBD0002')
        self.plug(WIRED_KEYBOARD)
        self.assertEqual(self.serials(), [MOUSE_SERIAL, 'XX00000000B9'])

        self.sysfs.restore_attribute(driver_mode_mouse, 'device_mode')
        persistence_file.rmdir()
        persistence_file.write_text('')

    def test_retries_back_off_and_skip_answered_roles(self):
        self.sysfs.add_mouse(RECEIVER_MOUSE, RECEIVER_MOUSE_NAME)
        self.sysfs.add_keyboard(RECEIVER_KEYBOARD, RECEIVER_KEYBOARD_NAME, serial=None)
        self.sysfs.add_input_only(RECEIVER_INPUT)
        self.start_daemon()
        self.sysfs.clear(RECEIVER_MOUSE, 'matrix_effect_spectrum')
        # A verified serial is not queried again for the same interface
        self.sysfs.set_serial(RECEIVER_MOUSE, None)

        # The asleep keyboard is queried at growing intervals, at most once per minute
        queried = []
        for tick in range(60):
            self.sysfs.set_serial(RECEIVER_KEYBOARD, None)
            before = self.clock.now
            with unittest.mock.patch('builtins.open', wraps=open) as opened:
                self.timers.fire()
            if any(call.args and str(call.args[0]).endswith('0904.0005/device_serial') for call in opened.call_args_list):
                queried.append(before - 1000.0)
            self.assertFalse(any(call.args and str(call.args[0]).endswith('0904.0004/device_serial') for call in opened.call_args_list))

        intervals = [b - a for a, b in zip(queried, queried[1:])]
        self.assertEqual(intervals[:4], [10.0, 20.0, 40.0, 60.0])
        self.assertEqual(set(intervals[4:]), {60.0})
        self.assertEqual(self.roles(), {MOUSE_SERIAL: RECEIVER_MOUSE_NAME})
        self.assertEqual(self.sysfs.read(RECEIVER_MOUSE, 'matrix_effect_spectrum'), '')

    def test_missing_or_malformed_serials_are_never_exported(self):
        self.sysfs.add_mouse(RECEIVER_MOUSE, RECEIVER_MOUSE_NAME)
        self.sysfs.add_keyboard(RECEIVER_KEYBOARD, RECEIVER_KEYBOARD_NAME)
        self.sysfs.add_input_only(RECEIVER_INPUT)
        self.sysfs.add_keyboard(WIRED_KEYBOARD, WIRED_KEYBOARD_NAME, serial='TURRETKBD0002')
        self.sysfs.add_mouse(WIRED_MOUSE, WIRED_MOUSE_NAME, serial='TURRETMOUSE0002')
        self.start_daemon()
        self.assertEqual(len(self.serials()), 4)

        for serial in ('', 'Default string', b'\x00' * 22, b'\xff\xfe', 'turret kbd', None):
            with self.subTest(serial=serial):
                for name in (RECEIVER_MOUSE, RECEIVER_KEYBOARD, WIRED_KEYBOARD, WIRED_MOUSE):
                    self.sysfs.set_serial(name, serial)
                self.unplug(RECEIVER_MOUSE, RECEIVER_KEYBOARD, RECEIVER_INPUT, WIRED_KEYBOARD, WIRED_MOUSE)
                self.sysfs.add_mouse(RECEIVER_MOUSE, RECEIVER_MOUSE_NAME, serial=serial)
                self.sysfs.add_keyboard(RECEIVER_KEYBOARD, RECEIVER_KEYBOARD_NAME, serial=serial)
                self.sysfs.add_input_only(RECEIVER_INPUT)
                self.sysfs.add_keyboard(WIRED_KEYBOARD, WIRED_KEYBOARD_NAME, serial=serial)
                self.sysfs.add_mouse(WIRED_MOUSE, WIRED_MOUSE_NAME, serial=serial)

                self.plug(RECEIVER_MOUSE, RECEIVER_KEYBOARD, RECEIVER_INPUT, WIRED_KEYBOARD, WIRED_MOUSE)
                self.timers.fire()

                self.assertEqual(self.serials(), [])
                self.assertEqual(self.exported_paths(), [])
                self.assertTrue(self.timers.active)

    def test_failed_initialisation_is_cleaned_up_and_retried(self):
        self.sysfs.add_mouse(RECEIVER_MOUSE, RECEIVER_MOUSE_NAME)
        self.sysfs.add_keyboard(RECEIVER_KEYBOARD, RECEIVER_KEYBOARD_NAME)
        self.sysfs.add_input_only(RECEIVER_INPUT)
        # Restoring the effect after export fails
        self.sysfs.break_attribute(RECEIVER_KEYBOARD, 'matrix_effect_spectrum')

        self.start_daemon()
        self.assertEqual(self.serials(), [MOUSE_SERIAL])
        self.assertEqual(self.exported_paths(), [MOUSE_SERIAL])

        self.sysfs.restore_attribute(RECEIVER_KEYBOARD, 'matrix_effect_spectrum')
        for _ in range(ROLE_RETRY_TICKS):
            self.timers.fire()
        self.assertEqual(self.roles(), {KEYBOARD_SERIAL: RECEIVER_KEYBOARD_NAME, MOUSE_SERIAL: RECEIVER_MOUSE_NAME})

    def test_failed_hotplug_initialisation_does_not_block_later_hotplug(self):
        self.start_daemon()
        self.sysfs.add_fake_driver(LEGACY_MOUSE, 'razerdeathadderv2', LEGACY_SERIAL)
        self.sysfs.break_attribute(LEGACY_MOUSE, 'logo_matrix_effect_spectrum')
        # A plain file cannot turn binary DPI writes into the text read back while closing
        self.sysfs.break_attribute(LEGACY_MOUSE, 'dpi')
        self.sysfs.add_keyboard(WIRED_KEYBOARD, WIRED_KEYBOARD_NAME)
        self.sysfs.break_attribute(WIRED_KEYBOARD, 'matrix_effect_spectrum')
        self.plug(LEGACY_MOUSE, WIRED_KEYBOARD)
        self.assertEqual(self.serials(), [])
        self.assertEqual(self.exported_paths(), [])

        self.sysfs.add_mouse(WIRED_MOUSE, WIRED_MOUSE_NAME)
        self.plug(WIRED_MOUSE)
        self.assertEqual(self.roles(), {MOUSE_SERIAL: WIRED_MOUSE_NAME})

    def test_handover_when_receiver_role_no_longer_answers(self):
        self.sysfs.add_mouse(RECEIVER_MOUSE, RECEIVER_MOUSE_NAME)
        self.sysfs.add_input_only(RECEIVER_INPUT)
        self.start_daemon()
        # Closing reads the current DPI back
        self.sysfs.break_attribute(RECEIVER_MOUSE, 'dpi')

        self.sysfs.add_mouse(WIRED_MOUSE, WIRED_MOUSE_NAME)
        self.plug(WIRED_MOUSE)

        self.assertEqual(self.roles(), {MOUSE_SERIAL: WIRED_MOUSE_NAME})
        self.assertEqual(self.exported_paths(), [MOUSE_SERIAL])

    def test_handover_releases_a_role_with_unexpected_dpi_text(self):
        self.sysfs.add_mouse(RECEIVER_MOUSE, RECEIVER_MOUSE_NAME)
        self.start_daemon()
        receiver_notifiers = battery_notifiers(RECEIVER_MOUSE_NAME)
        self.assertEqual(len(receiver_notifiers), 1)
        (self.sysfs.path(RECEIVER_MOUSE) / 'dpi').write_text('unexpected\n')

        self.sysfs.add_mouse(WIRED_MOUSE, WIRED_MOUSE_NAME)
        self.plug(WIRED_MOUSE)

        self.assertEqual(self.roles(), {MOUSE_SERIAL: WIRED_MOUSE_NAME})
        # The replaced object stopped its battery notifier
        self.assertFalse(any(thread.is_alive() for thread in receiver_notifiers))

    def test_handover_when_persistence_cannot_be_written(self):
        persistence_file = self.sysfs.root.parent / (self.sysfs.root.name + '.persistence')
        persistence_file.write_text('')
        self.addCleanup(remove_path, persistence_file)
        self.sysfs.add_mouse(RECEIVER_MOUSE, RECEIVER_MOUSE_NAME)
        self.start_daemon(persistence_file=str(persistence_file))
        persistence_file.unlink()
        persistence_file.mkdir()

        self.sysfs.add_mouse(WIRED_MOUSE, WIRED_MOUSE_NAME)
        self.plug(WIRED_MOUSE)
        self.assertEqual(self.roles(), {MOUSE_SERIAL: WIRED_MOUSE_NAME})
        self.unplug(WIRED_MOUSE)
        self.assertEqual(self.roles(), {MOUSE_SERIAL: RECEIVER_MOUSE_NAME})

        persistence_file.rmdir()
        persistence_file.write_text('')

    def test_failed_unexport_keeps_the_object_and_is_retried(self):
        self.sysfs.add_mouse(RECEIVER_MOUSE, RECEIVER_MOUSE_NAME)
        self.start_daemon()
        real_remove = dbus.service.Object.remove_from_connection
        failures = [RuntimeError('unregister failed')]

        def failing_remove(obj, *args, **kwargs):
            if failures:
                raise failures.pop()
            return real_remove(obj, *args, **kwargs)

        self.sysfs.add_mouse(WIRED_MOUSE, WIRED_MOUSE_NAME)
        with unittest.mock.patch.object(dbus.service.Object, 'remove_from_connection', failing_remove):
            self.plug(WIRED_MOUSE)
            # Still exported and known, no second object for the same path
            self.assertEqual(self.roles(), {MOUSE_SERIAL: RECEIVER_MOUSE_NAME})
            self.assertEqual(self.exported_paths(), [MOUSE_SERIAL])
            self.assertTrue(self.timers.active)

            self.timers.fire()

        self.assertEqual(self.roles(), {MOUSE_SERIAL: WIRED_MOUSE_NAME})
        self.assertFalse(self.timers.active)

    def failing_unexports(self, count):
        """Make the next D-Bus unregistrations fail; count None fails all of them."""
        real_remove = dbus.service.Object.remove_from_connection
        remaining = [count]

        def failing_remove(obj, *args, **kwargs):
            if remaining[0] is None or remaining[0] > 0:
                if remaining[0] is not None:
                    remaining[0] -= 1
                raise RuntimeError('unregister failed')
            return real_remove(obj, *args, **kwargs)
        return unittest.mock.patch.object(dbus.service.Object, 'remove_from_connection', failing_remove)

    def vid_pid(self, serial):
        return [int(i) for i in self.call(serial, 'getVidPid')]

    def test_failed_unexport_on_unplug_is_retried(self):
        self.sysfs.add_mouse(WIRED_MOUSE, WIRED_MOUSE_NAME)
        self.sysfs.add_mouse(RECEIVER_MOUSE, RECEIVER_MOUSE_NAME)
        self.start_daemon()

        # The removal event tries twice, then one retry fails too
        with self.failing_unexports(3):
            self.unplug(WIRED_MOUSE)
            # The unplugged object is still exported, the receiver role waits for its path
            self.assertEqual(self.exported_paths(), [MOUSE_SERIAL])
            self.assertEqual(self.vid_pid(MOUSE_SERIAL), [0x1532, 0x0075])
            self.assertTrue(self.timers.active)

            self.timers.fire()
            self.assertEqual(self.vid_pid(MOUSE_SERIAL), [0x1532, 0x0075])

            self.timers.fire()

        self.assertEqual(self.roles(), {MOUSE_SERIAL: RECEIVER_MOUSE_NAME})
        self.assertFalse(self.timers.active)

    def test_failing_unexport_retries_until_stopped(self):
        self.sysfs.add_mouse(WIRED_MOUSE, WIRED_MOUSE_NAME)
        self.start_daemon()

        with self.failing_unexports(None):
            self.unplug(WIRED_MOUSE)
            for _ in range(3):
                self.timers.fire()
                self.assertEqual(self.exported_paths(), [MOUSE_SERIAL])
                self.assertTrue(self.timers.active)
            self.remote('/org/razer').stop(dbus_interface='razer.daemon', timeout=CALL_TIMEOUT)
            self.assertFalse(self.timers.active)

        # A repeated udev event removes it
        self.udev.emit('remove', WIRED_MOUSE)
        self.assertEqual(self.exported_paths(), [])

    def test_readded_interface_waits_for_its_old_object(self):
        self.sysfs.add_keyboard(RECEIVER_KEYBOARD, RECEIVER_KEYBOARD_NAME)
        self.start_daemon()

        # Failing during the removal event and the next batch
        with self.failing_unexports(3):
            self.unplug(RECEIVER_KEYBOARD)
            # Same HID name, now paired with another keyboard
            self.sysfs.add_keyboard(RECEIVER_KEYBOARD, RECEIVER_KEYBOARD_NAME, serial='TURRETKBD0002')
            self.plug(RECEIVER_KEYBOARD)
            self.assertEqual(self.serials(), [KEYBOARD_SERIAL])

            self.timers.fire()

        self.assertEqual(self.roles(), {'TURRETKBD0002': RECEIVER_KEYBOARD_NAME})
        self.assertEqual(self.exported_paths(), ['TURRETKBD0002'])
        self.assertFalse(self.timers.active)

    def test_batched_hotplug_prefers_wired_in_any_order(self):
        self.start_daemon()
        for reverse in (False, True):
            with self.subTest(reverse=reverse):
                self.add_all_turrets()
                self.plug(*sorted(self.sysfs.names(), reverse=reverse))

                self.assertEqual(self.roles(), {KEYBOARD_SERIAL: WIRED_KEYBOARD_NAME, MOUSE_SERIAL: WIRED_MOUSE_NAME})
                self.assertEqual(self.sysfs.read(RECEIVER_KEYBOARD, 'matrix_effect_spectrum'), '')
                self.assertEqual(self.sysfs.read(RECEIVER_MOUSE, 'matrix_effect_spectrum'), '')

                self.unplug(*self.sysfs.names())
                self.assertEqual(self.serials(), [])

    def test_unchanged_devices_are_not_reinitialised_or_queried(self):
        self.add_all_turrets()
        self.sysfs.add_fake_driver(LEGACY_MOUSE, 'razerdeathadderv2', LEGACY_SERIAL)
        self.start_daemon()
        for name in (WIRED_KEYBOARD, WIRED_MOUSE):
            self.sysfs.clear(name, 'matrix_effect_spectrum')
            self.sysfs.set_serial(name, None)
        for name in (RECEIVER_KEYBOARD, RECEIVER_MOUSE):
            self.sysfs.set_serial(name, None)

        # Repeated events for present interfaces and a later unrelated device
        self.plug(*self.sysfs.names())
        self.sysfs.add_input_only('0003:046D:C52B.0008')
        self.plug('0003:046D:C52B.0008')
        for _ in range(ROLE_RETRY_TICKS):
            self.timers.fire()

        self.assertEqual(self.roles(), {KEYBOARD_SERIAL: WIRED_KEYBOARD_NAME, LEGACY_SERIAL: 'Razer DeathAdder V2',
                                        MOUSE_SERIAL: WIRED_MOUSE_NAME})
        for name in (WIRED_KEYBOARD, WIRED_MOUSE):
            self.assertEqual(self.sysfs.read(name, 'matrix_effect_spectrum'), '')
        self.assertFalse(self.timers.active)

    def test_legacy_devices_are_discovered_as_before(self):
        self.sysfs.add_fake_driver(LEGACY_MOUSE, 'razerdeathadderv2', LEGACY_SERIAL)
        self.add_all_turrets()
        self.start_daemon()
        self.assertEqual(self.name_of(LEGACY_SERIAL), 'Razer DeathAdder V2')

        self.unplug(LEGACY_MOUSE)
        self.assertEqual(self.serials(), [KEYBOARD_SERIAL, MOUSE_SERIAL])

        self.sysfs.add_fake_driver(LEGACY_MOUSE, 'razerdeathadderv2', LEGACY_SERIAL)
        self.plug(LEGACY_MOUSE)
        self.assertEqual(self.serials(), [KEYBOARD_SERIAL, MOUSE_SERIAL, LEGACY_SERIAL])
        # Legacy devices keep generated serials for invalid values
        self.unplug(LEGACY_MOUSE)
        self.sysfs.add_fake_driver(LEGACY_MOUSE, 'razerdeathadderv2', 'Default string')
        self.plug(LEGACY_MOUSE)
        self.assertEqual(self.serials(), [KEYBOARD_SERIAL, MOUSE_SERIAL, 'UNKNOWN_15320084_0000'])

    def test_roles_never_use_device_mode(self):
        self.sysfs.add_mouse(RECEIVER_MOUSE, RECEIVER_MOUSE_NAME)
        self.sysfs.add_keyboard(RECEIVER_KEYBOARD, RECEIVER_KEYBOARD_NAME)
        self.start_daemon()
        for serial in (KEYBOARD_SERIAL, MOUSE_SERIAL):
            self.call(serial, 'suspendDevice')
            self.call(serial, 'resumeDevice')
            # The drivers have no device_mode attribute for these devices
            with self.assertRaises(dbus.exceptions.DBusException):
                self.call(serial, 'getDeviceMode')

        # Handing over closes the receiver role objects, which reads the DPI back as text
        self.sysfs.restore_dpi(RECEIVER_MOUSE)
        self.sysfs.add_keyboard(WIRED_KEYBOARD, WIRED_KEYBOARD_NAME)
        self.sysfs.add_mouse(WIRED_MOUSE, WIRED_MOUSE_NAME)
        wired_modes = {name: self.sysfs.snapshot(name)['device_mode'] for name in (WIRED_KEYBOARD, WIRED_MOUSE)}
        self.plug(WIRED_KEYBOARD, WIRED_MOUSE)
        self.assertEqual(self.roles(), {KEYBOARD_SERIAL: WIRED_KEYBOARD_NAME, MOUSE_SERIAL: WIRED_MOUSE_NAME})
        for serial in (KEYBOARD_SERIAL, MOUSE_SERIAL):
            self.call(serial, 'suspendDevice')
            self.call(serial, 'resumeDevice')

        # The wired drivers have device_mode like other devices; the daemon leaves it alone
        self.assertEqual({name: self.sysfs.snapshot(name)['device_mode'] for name in wired_modes}, wired_modes)
        self.assertEqual([name for name in self.sysfs.names() if self.sysfs.exists(name, 'device_mode')],
                         sorted(wired_modes))
        self.assertEqual(len(self.sysfs.names()), 4)


if __name__ == '__main__':
    unittest.main()
