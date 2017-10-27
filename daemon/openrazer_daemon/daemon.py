"""
Daemon class

This class is the main core of the daemon, this serves a basic dbus module to control the main bit of the daemon
"""
__version__ = '2.0.0'

import configparser
import logging
import logging.handlers
import os
import subprocess
import sys
import signal
import time
import tempfile
import setproctitle
import dbus.mainloop.glib
import dbus.service
import gi
gi.require_version('Gdk', '3.0')
import gi.repository
from gi.repository import GObject, GLib
from pyudev import Context, Monitor, MonitorObserver
import grp
import getpass
import json

import openrazer_daemon.hardware
from openrazer_daemon.dbus_services.service import DBusService
from openrazer_daemon.device import DeviceCollection
from openrazer_daemon.misc.screensaver_monitor import ScreensaverMonitor

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

    BUS_NAME = 'org.razer'

    def __init__(self, verbose=False, log_dir=None, console_log=False, run_dir=None, config_file=None, test_dir=None, bustype='session'):

        self._initialized = False

        setproctitle.setproctitle('openrazer-daemon')

        # We override this now so anything created by this class uses the
        # same bus type
        DBusService.BUS_TYPE = bustype

        # Expanding ~ as python doesnt do it by default, also creating dirs if needed
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

        # Check for plugdev group
        if not self._check_plugdev_group():
            self.logger.critical("User is not a member of the plugdev group")
            sys.exit(1)

        # Setup DBus to use gobject main loop
        dbus.mainloop.glib.threads_init()
        dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
        try:
            DBusService.__init__(self, self.BUS_NAME, '/org/razer')
        except dbus.exceptions.DBusException as e:
            self.logger.critical("Failed to start DBus service: {}".format(e.get_dbus_message()))
            return

        self._init_signals()
        self._main_loop = GObject.MainLoop()

        # Listen for input events from udev
        self._init_udev_monitor()

        # Load Classes
        self._device_classes = openrazer_daemon.hardware.get_device_classes()

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

        # TODO remove
        self.sync_effects(self._config.getboolean('Startup', 'sync_effects_enabled'))
        # TODO ======

        self._initialized = True

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
        # Dont propagate to default logger
        logger.propagate = 0

        if want_console_log:
            console_logger = logging.StreamHandler()
            console_logger.setLevel(log_level)
            console_logger.setFormatter(formatter)
            logger.addHandler(console_logger)

        if log_dir is not None:
            log_file = os.path.join(log_dir, 'razer.log')
            file_logger = logging.handlers.RotatingFileHandler(log_file, maxBytes=16777216, backupCount=10) # 16MiB
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
        user = getpass.getuser()
        if user == 'root':
            return True

        try:
            plugdev_group = grp.getgrnam('plugdev')

            if user in plugdev_group.gr_mem:
                return True
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
        for section in ('General', 'Startup', 'Statistics'):
            self._config[section] = {}

        self._config['DEFAULT'] = {
            'verbose_logging': True,
            'sync_effects_enabled': True,
            'devices_off_on_screensaver': True,
            'key_statistics': False,
        }

        if config_file is not None and os.path.exists(config_file):
            self._config.read(config_file)

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

                if device_class.match(sys_name, sys_path):  # Check it matches sys/ ID format and is supported
                    self.logger.info('Found device.%d: %s', device_number, sys_name)

                    # TODO add testdir support
                    # Basically find the other usb interfaces
                    device_match = sys_name.split('.')[0]
                    additional_interfaces = []
                    if not test_mode:
                        for alt_device in device_list:
                            if device_match in alt_device.sys_name and alt_device.sys_name != sys_name:
                                additional_interfaces.append(alt_device.sys_path)

                    if not device_class.USE_HIDRAW:
                        # Checking permissions
                        test_file = os.path.join(sys_path, 'device_type')
                        file_group_id = os.stat(test_file).st_gid
                        file_group_name = grp.getgrgid(file_group_id)[0]

                        if os.getgid() != file_group_id and file_group_name != 'plugdev':
                            self.logger.critical("Could not access {0}/device_type, file is not owned by plugdev".format(sys_path))
                            break

                    razer_device = device_class(sys_path, device_number, self._config, testing=self._test_dir is not None, additional_interfaces=sorted(additional_interfaces))

                    # Wireless devices sometimes dont listen
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

    def _add_device(self, device):
        """
        Add device event from udev

        :param device: Udev Device
        :type device: pyudev.device._device.Device
        """
        device_number = len(self._razer_devices)
        for device_class in self._device_classes:
            sys_name = device.sys_name
            sys_path = device.sys_path

            if sys_name in self._razer_devices:
                continue

            if device_class.match(sys_name, sys_path):  # Check it matches sys/ ID format and is supported
                self.logger.info('Found valid device.%d: %s', device_number, sys_name)
                razer_device = device_class(sys_path, device_number, self._config, testing=self._test_dir is not None)

                # Its a udev event so currently the device hasn't been chmodded yet
                time.sleep(0.2)

                # Wireless devices sometimes dont listen
                device_serial = razer_device.get_serial()

                if len(device_serial) > 0:
                    # Add Device
                    self._razer_devices.add(sys_name, device_serial, razer_device)
                    self.device_added()
                else:
                    logging.warning("Could not get serial for device {0}. Skipping".format(sys_name))

    def _remove_device(self, device):
        """
        Remove device event from udev

        :param device: Udev Device
        :type device: pyudev.device._device.Device
        """
        device_id = device.sys_name

        try:
            device = self._razer_devices[device_id]

            device.dbus.close()
            device.dbus.remove_from_connection()
            self.logger.warning("Removing %s", device_id)

            # Delete device
            del self._razer_devices[device.device_id]
            self.device_removed()

        except IndexError:  # Why didnt i set it up as KeyError
            # It will return "extra" events for the additional usb interfaces bound to the driver
            pass

    def _udev_input_event(self, device):
        """
        Function called by the Udev monitor (#observerPattern)

        :param device: Udev device
        :type device: pyudev.device._device.Device
        """
        self.logger.debug('Device event [%s]: %s', device.action, device.device_path)
        if device.action == 'add':
            self._add_device(device)
        elif device.action == 'remove':
            self._remove_device(device)

    def run(self):
        """
        Run the daemon
        """
        if not self._initialized:
            return

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

        self._main_loop.quit()

        # Stop udev monitor
        self._udev_observer.send_stop()

        for device in self._razer_devices:
            device.dbus.close()
