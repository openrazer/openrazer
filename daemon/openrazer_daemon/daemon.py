# SPDX-License-Identifier: GPL-2.0-or-later

"""
Daemon class

This class is the main core of the daemon, this serves a basic dbus module to control the main bit of the daemon
"""
__version__ = '3.12.1'

import configparser
import logging
import logging.handlers
import os
import re
import sys
import signal
import time
import setproctitle
import dbus.mainloop.glib
import dbus.service
from gi.repository import GLib
from pyudev import Context, Monitor, MonitorObserver
import grp
import getpass
import json
import threading

import openrazer_daemon.hardware
from openrazer_daemon.dbus_services.service import DBusService
from openrazer_daemon.device import DeviceCollection
from openrazer_daemon.misc.screensaver_monitor import ScreensaverMonitor
from openrazer_daemon.misc.autosave_persistence import PersistenceAutoSave

# Retry interval for role interfaces without a readable serial, e.g. an asleep receiver role
ROLE_RETRY_SECONDS = 5
ROLE_RETRY_MAX_SECONDS = 60


class RoleInterface(object):
    """
    HID interface of a wired device or of a receiver role that reports the same serial

    The serial is verified once per interface and kept until udev removes it.
    """

    def __init__(self, device_class, sys_path, additional_interfaces):
        self.device_class = device_class
        self.sys_path = sys_path
        self.additional_interfaces = additional_interfaces
        self.serial = None
        self.retry_at = 0.0
        self.retry_delay = ROLE_RETRY_SECONDS

    @property
    def is_receiver(self):
        return self.device_class.DEVICE_TYPE is not None

    def defer(self, now):
        """
        Back off exponentially, so asleep roles are rarely queried
        """
        self.retry_at = now + self.retry_delay
        self.retry_delay = min(self.retry_delay * 2, ROLE_RETRY_MAX_SECONDS)


class RazerDaemon(DBusService):
    """
    Daemon class

    This class sets up the main run loop which serves DBus messages. The logger is initialised
    in this module as well as finding and initialising devices.

    Serves the following functions via DBus
    * getDevices - Returns a list of serial numbers
    * enableTurnOffOnScreensaver - Starts/Continues the run loop on the screensaver thread
    * disableTurnOffOnScreensaver - Pauses the run loop on the screensaver thread
    """

    def __init__(self, verbose=False, log_dir=None, console_log=False, run_dir=None, config_file=None, persistence_file=None, test_dir=None):

        setproctitle.setproctitle('openrazer-daemon')  # pylint: disable=no-member

        # Expanding ~ as python doesn't do it by default, also creating dirs if needed
        try:
            if log_dir is not None:
                log_dir = os.path.expanduser(log_dir)
                os.makedirs(log_dir, exist_ok=True)
            if run_dir is not None:
                run_dir = os.path.expanduser(run_dir)
                os.makedirs(run_dir, exist_ok=True)
        except NotADirectoryError as e:
            print("Failed to create {}".format(e.filename), file=sys.stderr)
            sys.exit(1)

        if config_file is not None:
            config_file = os.path.expanduser(config_file)
            if not os.path.exists(config_file):
                print("Config file {} does not exist.".format(config_file), file=sys.stderr)
                sys.exit(1)

        if persistence_file is not None:
            persistence_file = os.path.expanduser(persistence_file)
            if not os.path.exists(persistence_file):
                print("Persistence file {} does not exist.".format(persistence_file), file=sys.stderr)
                sys.exit(1)

        self._test_dir = test_dir
        self._run_dir = run_dir

        self._config_file = config_file
        self._config = configparser.ConfigParser()
        self.read_config(config_file)

        # Logging
        log_level = logging.INFO
        if verbose or self._config.getboolean('General', 'verbose_logging'):
            log_level = logging.DEBUG
        self.logger = self._create_logger(log_dir, log_level, console_log)

        self._persistence_file = persistence_file
        self._persistence = configparser.ConfigParser()
        self._persistence.status = {"changed": False}
        self.read_persistence(persistence_file)

        # map of vid+pid to counter for serial numbers for unknown devices
        self._unknown_serial_counter: dict[tuple[int, int], int] = {}

        # Check for plugdev group
        if not self._check_plugdev_group():
            self.logger.critical("User is not a member of the plugdev group")
            self.logger.critical("Please run the command 'sudo gpasswd -a $USER plugdev' and then reboot!")
            sys.exit(1)

        # Setup DBus to use gobject main loop
        dbus.mainloop.glib.threads_init()
        dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
        super().__init__('/org/razer')

        self._init_signals()
        self._main_loop = GLib.MainLoop()

        # Listen for input events from udev
        self._init_udev_monitor()

        # Load Classes
        self._device_classes = openrazer_daemon.hardware.get_device_classes()

        # Receiver role classes and the wired classes they share serials with
        receiver_classes = [cls for cls in self._device_classes if cls.DEVICE_TYPE is not None]
        self._role_classes = receiver_classes + [base for cls in receiver_classes for base in cls.__mro__[1:] if base in self._device_classes]
        self._role_interfaces: dict[str, RoleInterface] = {}
        self._role_retry_source = None
        self._role_removal_failed = False
        # Objects still exported after udev removed their interface, by device ID
        self._orphaned_devices = {}

        # Guards device (un)exporting between hotplug threads and the main loop
        self._devices_lock = threading.RLock()
        # Set by quit, nothing is exported or scheduled afterwards
        self._stopping = False
        self._collecting_udev = False
        self._collecting_udev_devices = []

        self.logger.info("Initialising Daemon (v%s). Pid: %d", __version__, os.getpid())
        self._init_screensaver_monitor()

        self._razer_devices = DeviceCollection()
        self._load_devices(first_run=True)

        # Add DBus methods
        methods = {
            # interface, method, callback, in-args, out-args
            ('razer.devices', 'getDevices', self.get_serial_list, None, 'as'),
            ('razer.devices', 'supportedDevices', self.supported_devices, None, 's'),
            ('razer.devices', 'enableTurnOffOnScreensaver', self.enable_turn_off_on_screensaver, 'b', None),
            ('razer.devices', 'getOffOnScreensaver', self.get_off_on_screensaver, None, 'b'),
            ('razer.devices', 'syncEffects', self.sync_effects, 'b', None),
            ('razer.devices', 'getSyncEffects', self.get_sync_effects, None, 'b'),
            ('razer.daemon', 'version', self.version, None, 's'),
            ('razer.daemon', 'stop', self.stop, None, None),
        }

        for m in methods:
            self.logger.debug("Adding {}.{} method to DBus".format(m[0], m[1]))
            self.add_dbus_method(m[0], m[1], m[2], in_signature=m[3], out_signature=m[4])

        self._init_autosave_persistence()

        # TODO remove
        self.sync_effects(self._config.getboolean('Startup', 'sync_effects_enabled'))
        # TODO ======

    @dbus.service.signal('razer.devices')
    def device_removed(self):
        self.logger.debug("Emitted Device Remove Signal")

    @dbus.service.signal('razer.devices')
    def device_added(self):
        self.logger.debug("Emitted Device Added Signal")

    def _create_logger(self, log_dir, log_level, want_console_log):
        """
        Initializes a logger and returns it.

        :param log_dir: If not None, specifies the directory to create the
        log file in
        :param log_level: The log level of messages to print
        :param want_console_log: True if we should print to the console

        :rtype:logging.Logger
        """
        logger = logging.getLogger('razer')
        logger.setLevel(log_level)
        formatter = logging.Formatter('%(asctime)s | %(name)-30s | %(levelname)-8s | %(message)s', datefmt='%Y-%m-%d %H:%M:%S')
        # Don't propagate to default logger
        logger.propagate = 0

        if want_console_log:
            console_logger = logging.StreamHandler()
            console_logger.setLevel(log_level)
            console_logger.setFormatter(formatter)
            logger.addHandler(console_logger)

        if log_dir is not None:
            log_file = os.path.join(log_dir, 'razer.log')
            file_logger = logging.handlers.RotatingFileHandler(log_file, maxBytes=1048576, backupCount=1)  # 1 MiB
            file_logger.setLevel(log_level)
            file_logger.setFormatter(formatter)
            logger.addHandler(file_logger)

        return logger

    def _check_plugdev_group(self):
        """
        Check if the user is a member of the plugdev group. For the root
        user, this always returns True

        :rtype: bool
        """
        if getpass.getuser() == 'root':
            return True

        try:
            return grp.getgrnam('plugdev').gr_gid in os.getgroups()
        except KeyError:
            pass

        return False

    def _init_udev_monitor(self):
        self._udev_context = Context()
        udev_monitor = Monitor.from_netlink(self._udev_context)
        udev_monitor.filter_by(subsystem='hid')
        self._udev_observer = MonitorObserver(udev_monitor, callback=self._udev_input_event, name='device-monitor')

    def _init_screensaver_monitor(self):
        try:
            self._screensaver_monitor = ScreensaverMonitor(self)
            self._screensaver_monitor.monitoring = self._config.getboolean('Startup', 'devices_off_on_screensaver')
        except dbus.exceptions.DBusException as e:
            self.logger.error("Failed to init ScreensaverMonitor: {}".format(e))

    def _init_autosave_persistence(self):
        if not self._persistence:
            self.logger.debug("Persistence unspecified. Will not create auto save thread")
            return

        self._autosave_persistence = PersistenceAutoSave(self._persistence, self._persistence_file, self._persistence.status, self.logger, 10, self.write_persistence)
        self._autosave_persistence.thread = threading.Thread(target=self._autosave_persistence.watch)
        self._autosave_persistence.thread.daemon = True
        self._autosave_persistence.thread.start()

    def _init_signals(self):
        """
        Heinous hack to properly handle signals on the mainloop. Necessary
        if we want to use the mainloop run() functionality.
        """
        def signal_action(signum):
            """
            Action to take when a signal is trapped
            """
            self.quit(signum)

        def idle_handler():
            """
            GLib idle handler to propagate signals
            """
            GLib.idle_add(signal_action, priority=GLib.PRIORITY_HIGH)

        def handler(*args):
            """
            Unix signal handler
            """
            signal_action(args[0])

        def install_glib_handler(sig):
            """
            Choose a compatible method and install the handler
            """
            unix_signal_add = None

            if hasattr(GLib, "unix_signal_add"):
                unix_signal_add = GLib.unix_signal_add
            elif hasattr(GLib, "unix_signal_add_full"):
                unix_signal_add = GLib.unix_signal_add_full

            if unix_signal_add:
                unix_signal_add(GLib.PRIORITY_HIGH, sig, handler, sig)
            else:
                print("Can't install GLib signal handler!")

        for sig in signal.SIGINT, signal.SIGTERM, signal.SIGHUP:
            signal.signal(sig, idle_handler)
            GLib.idle_add(install_glib_handler, sig, priority=GLib.PRIORITY_HIGH)

    def read_config(self, config_file):
        """
        Read in the config file and set the defaults

        :param config_file: Config file
        :type config_file: str or None
        """
        # Generate sections as trying to access a value even if a default exists will die if the section does not
        for section in ('General', 'Startup'):
            self._config[section] = {}

        self._config['General'] = {
            'verbose_logging': False,
        }
        self._config['Startup'] = {
            'sync_effects_enabled': True,
            'devices_off_on_screensaver': True,
            'restore_persistence': True,
            'persistence_dual_boot_quirk': False,
        }

        if config_file is not None and os.path.exists(config_file):
            self._config.read(config_file)

        # Compatibility to older configs
        if 'mouse_battery_notifier' in self._config['Startup']:
            self._config['Startup']['battery_notifier'] = self._config['Startup']['mouse_battery_notifier']
        if 'mouse_battery_notifier_freq' in self._config['Startup']:
            self._config['Startup']['battery_notifier_freq'] = self._config['Startup']['mouse_battery_notifier_freq']

    def read_persistence(self, persistence_file):
        """
        Read the persistence file and set states into memory

        :param persistence_file: Persistence file
        :type persistence_file: str or None
        """
        if persistence_file is not None and os.path.exists(persistence_file):
            try:
                self._persistence.read(persistence_file)
            except (configparser.Error, UnicodeDecodeError):
                self.logger.warning('Failed to read persistence config, resetting!', exc_info=True)
                with open(persistence_file, "w") as f:
                    f.writelines("")

    def write_persistence(self, persistence_file):
        """
        Write in the persistence file

        :param persistence_file: Persistence file
        :type persistence_file: str or None
        """
        if not persistence_file:
            return

        self.logger.debug('Writing persistence config')

        for device in self._razer_devices:
            self._persistence[device.dbus.storage_name] = {}
            if 'set_dpi_xy' in device.dbus.METHODS or 'set_dpi_xy_byte' in device.dbus.METHODS:
                dpi_x = int(device.dbus.dpi[0])
                dpi_y = int(device.dbus.dpi[1])
                # When Y is not greater than 0 check for a DPI X only device, a device with 'available_dpi' and a Y value of 0
                if dpi_x > 0 and (dpi_y > 0 or ('available_dpi' in device.dbus.METHODS and dpi_y == 0)):
                    self._persistence[device.dbus.storage_name]['dpi_x'] = str(dpi_x)
                    self._persistence[device.dbus.storage_name]['dpi_y'] = str(dpi_y)

            if 'set_poll_rate' in device.dbus.METHODS:
                self._persistence[device.dbus.storage_name]['poll_rate'] = str(device.dbus.poll_rate)

            for i in device.dbus.ZONES:
                if device.dbus.zone[i]["present"]:
                    self._persistence[device.dbus.storage_name][i + '_active'] = str(device.dbus.zone[i]["active"])
                    self._persistence[device.dbus.storage_name][i + '_brightness'] = str(device.dbus.zone[i]["brightness"])
                    self._persistence[device.dbus.storage_name][i + '_effect'] = device.dbus.zone[i]["effect"]
                    self._persistence[device.dbus.storage_name][i + '_colors'] = ' '.join(str(i) for i in device.dbus.zone[i]["colors"])
                    self._persistence[device.dbus.storage_name][i + '_speed'] = str(device.dbus.zone[i]["speed"])
                    self._persistence[device.dbus.storage_name][i + '_wave_dir'] = str(device.dbus.zone[i]["wave_dir"])

        with open(persistence_file, 'w') as cf:
            self._persistence.write(cf)

    def get_off_on_screensaver(self):
        """
        Returns if turn off on screensaver

        :return: Result
        :rtype: bool
        """
        return self._screensaver_monitor.monitoring

    def enable_turn_off_on_screensaver(self, enable):
        """
        Enable the turning off of devices when the screensaver is active
        """
        self._screensaver_monitor.monitoring = enable

    def supported_devices(self):
        result = {cls.__name__: (cls.USB_VID, cls.USB_PID) for cls in self._device_classes}

        return json.dumps(result)

    def version(self):
        """
        Get the daemon version

        :return: Version string
        :rtype: str
        """
        return __version__

    def suspend_devices(self):
        """
        Suspend all devices
        """
        for device in self._razer_devices:
            device.dbus.suspend_device()

    def resume_devices(self):
        """
        Resume all devices
        """
        for device in self._razer_devices:
            device.dbus.resume_device()

    def get_serial_list(self):
        """
        Get list of devices serials
        """
        serial_list = self._razer_devices.serials()
        self.logger.debug('DBus called get_serial_list')
        return serial_list

    def sync_effects(self, enabled):
        """
        Sync the effects across the devices

        :param enabled: True to sync effects
        :type enabled: bool
        """
        # Todo perhaps move logic to device collection
        for device in self._razer_devices.devices:
            device.dbus.effect_sync = enabled

    def get_sync_effects(self):
        """
        Sync the effects across the devices

        :return: True if any devices sync effects
        :rtype: bool
        """
        result = False

        for device in self._razer_devices.devices:
            result |= device.dbus.effect_sync

        return result

    def _load_devices(self, first_run=False):
        """
        Go through supported devices and load them

        Loops through the available hardware classes, loops through
        each device in the system and adds it if needs be.
        """
        if first_run:
            # Just some pretty output
            max_name_len = max([len(cls.__name__) for cls in self._device_classes]) + 2
            for cls in self._device_classes:
                format_str = 'Loaded device specification: {0:-<' + str(max_name_len) + '} ({1:04x}:{2:04X})'

                self.logger.debug(format_str.format(cls.__name__ + ' ', cls.USB_VID, cls.USB_PID))

        if self._test_dir is not None:
            device_list = os.listdir(self._test_dir)
            test_mode = True
        else:
            device_list = list(self._udev_context.list_devices(subsystem='hid'))
            test_mode = False

        device_number = 0
        for device in device_list:
            if self._add_role_interface(device, device_list, test_mode):
                continue

            for device_class in self._device_classes:
                # Interoperability between generic list of 0000:0000:0000.0000 and pyudev
                if test_mode:
                    sys_name = device
                    sys_path = os.path.join(self._test_dir, device)
                else:
                    sys_name = device.sys_name
                    sys_path = device.sys_path

                if sys_name in self._razer_devices:
                    continue

                if device_class.match(sys_name, sys_path):  # Check it matches sys/ ID format and has device_type file
                    self.logger.info('Found device.%d: %s', device_number, sys_name)

                    # TODO add testdir support
                    # Basically find the other usb interfaces
                    device_match = sys_name.split('.')[0]
                    additional_interfaces = []
                    if not test_mode:
                        double_device = False
                        for alt_device in self._razer_devices:
                            if device_match in alt_device.device_id and alt_device.device_id != sys_name and sys_path in alt_device.dbus.additional_interfaces:
                                self.logger.warning('BUG: Device %s has already been found with interface %s. Skipping', sys_name, alt_device.device_id)
                                double_device = True
                        if double_device:
                            continue

                        for alt_device in device_list:
                            if device_match in alt_device.sys_name and alt_device.sys_name != sys_name:
                                additional_interfaces.append(alt_device.sys_path)

                    # Checking permissions
                    test_file = os.path.join(sys_path, 'device_type')
                    file_group_id = os.stat(test_file).st_gid
                    file_group_name = grp.getgrgid(file_group_id)[0]

                    if os.getgid() != file_group_id and file_group_name != 'plugdev':
                        self.logger.critical("Could not access {0}/device_type, file is not owned by plugdev".format(sys_path))
                        break

                    razer_device = device_class(device_path=sys_path, device_number=device_number, config=self._config,
                                                persistence=self._persistence, testing=self._test_dir is not None,
                                                additional_interfaces=sorted(additional_interfaces),
                                                additional_methods=[],
                                                unknown_serial_counter=self._unknown_serial_counter)

                    # Wireless devices sometimes don't listen
                    count = 0
                    while count < 3:
                        # Loop to get serial, exit early if it gets one
                        device_serial = razer_device.get_serial()
                        if len(device_serial) > 0:
                            break
                        time.sleep(0.1)
                        count += 1
                    else:
                        logging.warning("Could not get serial for device {0}. Skipping".format(sys_name))
                        continue

                    self._razer_devices.add(sys_name, device_serial, razer_device)

                    device_number += 1

        self._reconcile_roles()

    def _add_role_interface(self, device, device_list=None, test_mode=False):
        """
        Remember a wired or receiver role interface, exported later by _reconcile_roles

        :return: True if the interface belongs to a role class
        :rtype: bool
        """
        if test_mode:
            sys_name = device
            sys_path = os.path.join(self._test_dir, device)
        else:
            sys_name = device.sys_name
            sys_path = device.sys_path

        device_class = next((cls for cls in self._role_classes if cls.match(sys_name, sys_path)), None)
        if device_class is None:
            return False

        # Like other wired devices, but receiver roles are independent of their siblings
        additional_interfaces = []
        if device_list is not None and not test_mode and device_class.DEVICE_TYPE is None:
            device_match = sys_name.split('.')[0]
            additional_interfaces = sorted(alt.sys_path for alt in device_list if device_match in alt.sys_name and alt.sys_name != sys_name)

        with self._devices_lock:
            if sys_name not in self._role_interfaces:
                self.logger.info('Found %s interface: %s', device_class.__name__, sys_name)
                self._role_interfaces[sys_name] = RoleInterface(device_class, sys_path, additional_interfaces)
        return True

    def _read_role_serial(self, sys_path, attempts):
        """
        Read a serial before any object is created for it

        :return: Serial, or None when missing, unreadable or malformed
        :rtype: str or None
        """
        for attempt in range(attempts):
            if attempt > 0:
                time.sleep(0.1)
            try:
                with open(os.path.join(sys_path, 'device_serial'), 'rb') as serial_file:
                    serial = serial_file.read().decode('ascii').strip()
            except (OSError, UnicodeDecodeError) as err:
                self.logger.debug('Could not read serial of %s: %s', sys_path, err)
                continue

            # Same check as RazerDevice.get_serial, but never a generated serial
            if re.fullmatch(r'[\dA-Z]+', serial):
                return serial
            self.logger.warning('Invalid serial %r for %s', serial, sys_path)
        return None

    def _reconcile_roles(self, limit=None):
        """
        Export one object per serial across role interfaces, preferring wired

        Serials are read without holding the lock. Only interfaces still present
        afterwards are selected, so results for removed ones are ignored.

        :param limit: Maximum number of pending interfaces to read
        :type limit: int or None
        """
        now = time.monotonic()
        with self._devices_lock:
            if self._stopping:
                return
            pending = sorted((iface.retry_at, name, iface) for name, iface in self._role_interfaces.items()
                             if iface.serial is None and iface.retry_at <= now and not self._role_waits(iface))
        pending = pending[:limit]

        # Periodic retries read once; hotplug allows for a slow wireless answer
        attempts = 1 if limit else 3
        results = [(iface, self._read_role_serial(iface.sys_path, attempts)) for _retry_at, _name, iface in pending]

        with self._devices_lock:
            if self._stopping:
                return  # Stopped during the reads

            for device_id, device_dbus in list(self._orphaned_devices.items()):
                if device_id not in self._razer_devices or self._razer_devices[device_id].dbus is not device_dbus or self._unexport_device(device_id):
                    del self._orphaned_devices[device_id]

            for iface, serial in results:
                if iface.serial is not None:
                    continue  # Already answered through a concurrent call
                if serial is None:
                    iface.defer(now)
                else:
                    iface.serial = serial
                    iface.retry_delay = ROLE_RETRY_SECONDS

            winners = {}
            for name, iface in self._role_interfaces.items():
                if iface.serial is not None:
                    rank = (iface.is_receiver, name not in self._razer_devices, name)
                    if iface.serial not in winners or rank < winners[iface.serial][0]:
                        winners[iface.serial] = (rank, name)
            selected = {name for _rank, name in winners.values()}

            # Remove replaced objects first, their serial is the new object's D-Bus path
            self._role_removal_failed = False
            for name in list(self._role_interfaces):
                if name in self._razer_devices and name not in selected:
                    if not self._unexport_device(name):
                        self._role_removal_failed = True

            for serial, (_rank, name) in sorted(winners.items()):
                if name in self._razer_devices:
                    continue  # Exported, or its old object still is
                if serial in self._razer_devices:
                    self.logger.warning('Serial %s of %s is already in use. Skipping', serial, name)
                    continue
                self._export_role(name, self._role_interfaces[name], now)

            if self._role_retry_source is None and self._roles_need_retry():
                self._role_retry_source = GLib.timeout_add_seconds(ROLE_RETRY_SECONDS, self._retry_roles)

    def _roles_need_retry(self):
        return self._role_removal_failed or bool(self._orphaned_devices) or \
            any(iface.serial is None and not self._role_waits(iface) for iface in self._role_interfaces.values())

    def _role_waits(self, iface):
        """
        A receiver role isn't queried while a wired device of its kind answers

        That device is most likely the one missing from the receiver. Unplugging
        it queries the receiver role again at once.
        """
        return iface.is_receiver and any(not other.is_receiver and other.serial is not None and issubclass(iface.device_class, other.device_class)
                                         for other in self._role_interfaces.values())

    def _create_device(self, device_class, **kwargs):
        """
        Create a device object, removing it from D-Bus again if initialisation fails

        :return: Device object or None
        :rtype: openrazer_daemon.hardware.device_base.RazerDevice or None
        """
        # Keep a reference to the partially initialised object for cleanup
        razer_device = device_class.__new__(device_class)
        try:
            razer_device.__init__(config=self._config, persistence=self._persistence, testing=self._test_dir is not None,
                                  additional_methods=[], unknown_serial_counter=self._unknown_serial_counter, **kwargs)
        except Exception:  # pylint: disable=broad-except
            self.logger.exception('Failed to initialise %s', kwargs['device_path'])
            self._discard_device(razer_device)
            return None
        return razer_device

    @staticmethod
    def _discard_device(razer_device):
        """
        Remove a device object that never became part of the collection
        """
        try:
            razer_device.remove_from_connection()
        except (LookupError, AttributeError):
            pass  # Failed before it was exported
        try:
            razer_device.close()
        except Exception:  # pylint: disable=broad-except
            pass

    def _export_role(self, name, iface, now):
        """
        Create the D-Bus object of a role interface
        """
        razer_device = self._create_device(iface.device_class, device_path=iface.sys_path, device_number=len(self._razer_devices),
                                           additional_interfaces=iface.additional_interfaces, serial=iface.serial)
        if razer_device is None:
            # Verify the serial again first, e.g. the role may have gone to sleep
            iface.serial = None
            iface.defer(now)
            return

        self._razer_devices.add(name, iface.serial, razer_device)
        self.device_added()

    def _retry_roles(self):
        """
        GLib timeout: retry one pending role interface per call
        """
        try:
            self._reconcile_roles(limit=1)
        except Exception:  # pylint: disable=broad-except
            self.logger.exception('Failed to retry role interfaces')

        with self._devices_lock:
            if not self._stopping and self._roles_need_retry():
                return True
            self._role_retry_source = None
            return False

    def _add_device(self, device):
        """
        Add device event from udev

        :param device: Udev Device
        :type device: pyudev.device._device.Device
        """
        # Exported after the whole batch
        if self._add_role_interface(device):
            return

        device_number = len(self._razer_devices)
        for device_class in self._device_classes:
            sys_name = device.sys_name
            sys_path = device.sys_path

            if sys_name in self._razer_devices:
                continue

            if device_class.match(sys_name, sys_path):  # Check it matches sys/ ID format and has device_type file
                self.logger.info('Found valid device.%d: %s', device_number, sys_name)
                # Created and added in one step: quit either comes first or waits for it
                with self._devices_lock:
                    if self._stopping:
                        return
                    razer_device = self._create_device(device_class, device_path=sys_path, device_number=device_number,
                                                       additional_interfaces=None)
                    if razer_device is None:
                        return

                    # Its a udev event so currently the device hasn't been chmodded yet
                    time.sleep(0.2)

                    # Wireless devices sometimes don't listen
                    device_serial = razer_device.get_serial()

                    if len(device_serial) > 0:
                        # Add Device
                        self._razer_devices.add(sys_name, device_serial, razer_device)
                        self.device_added()
                    else:
                        logging.warning("Could not get serial for device {0}. Skipping".format(sys_name))
            else:
                # Basically find the other usb interfaces
                device_match = sys_name.split('.')[0]
                with self._devices_lock:
                    for d in self._razer_devices:
                        # Receiver roles are separate devices
                        if d.dbus.DEVICE_TYPE is not None:
                            continue
                        if device_match in d.device_id and d.device_id != sys_name:
                            if not sys_path in d.dbus.additional_interfaces:
                                d.dbus.additional_interfaces.append(sys_path)
                                return

    def _remove_device(self, device):
        """
        Remove device event from udev

        :param device: Udev Device
        :type device: pyudev.device._device.Device
        """
        device_id = device.sys_name

        with self._devices_lock:
            iface = self._role_interfaces.pop(device_id, None)

            # It will return "extra" events for the additional usb interfaces bound to the driver
            if device_id in self._razer_devices:
                device_dbus = self._razer_devices[device_id].dbus
                if self._unexport_device(device_id):
                    self._orphaned_devices.pop(device_id, None)
                else:
                    self._orphaned_devices[device_id] = device_dbus

            if iface is not None:
                # Verify a remaining interface with this serial before it takes over. A device
                # unplugged from its cable may also connect to an asleep receiver role soon.
                for other_id, other in self._role_interfaces.items():
                    if other_id in self._razer_devices:
                        continue
                    if other.serial == iface.serial or (other.serial is None and not iface.is_receiver):
                        other.serial = None
                        other.retry_at = 0.0
                        other.retry_delay = ROLE_RETRY_SECONDS

        if iface is not None or device_id in self._orphaned_devices:
            self._reconcile_roles()

    def _unexport_device(self, device_id):
        """
        Close a device and remove it from D-Bus

        :return: False if it is still exported, it stays in the collection then
        :rtype: bool
        """
        device = self._razer_devices[device_id]

        # Remove it from D-Bus anyway, its path may be needed for a replacement
        try:
            device.dbus.close()
        except Exception:  # pylint: disable=broad-except
            self.logger.exception('Failed to close %s', device_id)
        try:
            device.dbus.remove_from_connection()
        except LookupError:
            pass  # Not exported
        except Exception:  # pylint: disable=broad-except
            self.logger.exception('Failed to remove %s from D-Bus', device_id)
            return False
        try:
            self.write_persistence(self._persistence_file)
        except Exception:  # pylint: disable=broad-except
            self.logger.exception('Failed to write persistence')
        self.logger.warning("Removing %s", device_id)

        del self._razer_devices[device.device_id]
        self.device_removed()
        return True

    def _udev_input_event(self, device):
        """
        Function called by the Udev monitor (#observerPattern)

        :param device: Udev device
        :type device: pyudev.device._device.Device
        """
        self.logger.debug('Device event [%s]: %s', device.action, device.device_path)
        if device.action == 'add':
            with self._devices_lock:
                if self._stopping:
                    return
                self._collecting_udev_devices.append(device)
                if self._collecting_udev:
                    return
                self._collecting_udev = True
            t = threading.Thread(target=self._collecting_udev_method, args=(device,))
            t.start()
        elif device.action == 'remove':
            self._remove_device(device)

    def _collecting_udev_method(self, device):
        time.sleep(2)  # delay to let udev add all devices that we want
        try:
            while True:
                # Events arriving meanwhile are added in the next pass
                with self._devices_lock:
                    devices = self._collecting_udev_devices
                    self._collecting_udev_devices = []
                    if not devices:
                        self._collecting_udev = False
                        return

                # Sort the devices
                devices.sort(key=lambda x: x.sys_path, reverse=True)
                for d in devices:
                    try:
                        self._add_device(d)
                    except Exception:  # pylint: disable=broad-except
                        self.logger.exception('Failed to add device %s', d.sys_name)
                self._reconcile_roles()
        except BaseException:
            # Never leave later events waiting for this thread
            with self._devices_lock:
                self._collecting_udev = False
            raise

    def run(self):
        """
        Run the daemon
        """
        self.logger.info('Serving DBus')

        # Start listening for device changes
        self._udev_observer.start()

        # Start the mainloop
        try:
            self._main_loop.run()
        except KeyboardInterrupt:
            self.logger.debug('Shutting down')

    def stop(self):
        """
        Wrapper for quit
        """
        self.quit(None)

    def quit(self, signum):
        """
        Quit by stopping the main loop, observer, and screensaver thread
        """
        # pylint: disable=unused-argument
        if signum is None:
            self.logger.info('Stopping daemon.')
        else:
            self.logger.info('Stopping daemon on signal %d', signum)

        # Hotplug threads and retries wait for this, then stop exporting devices
        with self._devices_lock:
            if self._stopping:
                return
            self._stopping = True
            self._collecting_udev_devices = []
            if self._role_retry_source is not None:
                GLib.source_remove(self._role_retry_source)
                self._role_retry_source = None

            # "Resume" all devices, in case they're still "suspended"
            # (lights off because of screensaver). One failing device must not stop the rest.
            for device in self._razer_devices:
                try:
                    device.dbus.resume_device()
                except Exception:  # pylint: disable=broad-except
                    self.logger.exception('Failed to resume %s', device.device_id)

            self._main_loop.quit()

            # Stop udev monitor
            self._udev_observer.send_stop()

            for device in self._razer_devices:
                try:
                    device.dbus.close()
                except Exception:  # pylint: disable=broad-except
                    self.logger.exception('Failed to close %s', device.device_id)

            # Write config
            try:
                self.write_persistence(self._persistence_file)
            except Exception:  # pylint: disable=broad-except
                self.logger.exception('Failed to write persistence')
