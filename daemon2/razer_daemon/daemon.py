"""
Daemon class

This class is the main core of the daemon, this serves a basic dbus module to control the main bit of the daemon
"""
import logging
import os
import time
import signal
from gi.repository import GObject
from dbus.mainloop.glib import DBusGMainLoop

import razer_daemon.hardware
from razer_daemon.screensaver_thread import ScreensaverThread
from razer_daemon.dbus_services.service import DBusService


class Daemon(DBusService):
    """
    Daemon class

    This class sets up the main run loop which serves DBus messages. The logger is initialised
    in this module as well as finding and initialising devices.

    Serves the following functions via DBus
    * getDevices - Returns a list of serial numbers
    * enableTurnOffOnScreensaver - Starts/Continues the run loop on the screensaver thread
    * disableTurnOffOnScreensaver - Pauses the run loop on the screensaver thread
    """

    BUS_PATH = 'org.razer'

    def __init__(self, logging_level=logging.WARNING, log_file=None, console_log=True, setup_signals=True):
        # Setup DBus to use gobject main loop
        DBusGMainLoop(set_as_default=True)
        DBusService.__init__(self, self.BUS_PATH, '/org/razer')

        self._main_loop = GObject.MainLoop()

        self.logger = logging.getLogger('razer')
        self.logger.setLevel(logging_level)
        formatter = logging.Formatter('%(asctime)s | %(name)-20s | %(levelname)-8s | %(message)s', datefmt='%Y-%m-%d %H:%M:%S')
        # Dont propagate to default logger
        self.logger.propagate = 0

        if console_log:
            console_logger = logging.StreamHandler()
            console_logger.setLevel(logging_level)
            console_logger.setFormatter(formatter)
            self.logger.addHandler(console_logger)

        if log_file is not None:
            file_logger = logging.FileHandler(log_file)
            file_logger.setLevel(logging_level)
            file_logger.setFormatter(formatter)
            self.logger.addHandler(file_logger)

        self.logger.info("Initialising Daemon. Pid: %d", os.getpid())

        # Setup screensaver thread
        self._screensaver_thread = ScreensaverThread(self, active=True)
        self._screensaver_thread.start()

        self._razer_devices = {}
        self._load_devices()

        # Add DBus methods
        self.logger.info("Adding razer.devices.getDevices method to DBus")
        self.add_dbus_method('razer.devices', 'getDevices', self.get_serial_list, out_signature='as')
        self.logger.info("Adding razer.devices.enableTurnOffOnScreensaver method to DBus")
        self.add_dbus_method('razer.devices', 'enableTurnOffOnScreensaver', self.enable_turn_off_on_screensaver)
        self.logger.info("Adding razer.devices.disableTurnOffOnScreensaver method to DBus")
        self.add_dbus_method('razer.devices', 'disableTurnOffOnScreensaver', self.disable_turn_off_on_screensaver)

        # Setup quit signals
        if setup_signals:
            signal.signal(signal.SIGINT, self.quit)
            signal.signal(signal.SIGTERM, self.quit)

    def enable_turn_off_on_screensaver(self):
        """
        Enable the turning off of devices when the screensaver is active
        """
        self._screensaver_thread.active = True

    def disable_turn_off_on_screensaver(self):
        """
        Disable the turning off of devices when the screensaver is active
        """
        self._screensaver_thread.active = False

    def suspend_devices(self):
        """
        Suspend all devices
        """
        for device in self._razer_devices.values():
            device.suspend_device()

    def resume_devices(self):
        """
        Resume all devices
        """
        for device in self._razer_devices.values():
            device.resume_device()

    def get_serial_list(self):
        """
        Get list of devices serials
        """
        devices = list(self._razer_devices.keys())
        self.logger.debug('DBus called get_serial_list')
        return devices

    def _load_devices(self):
        """
        Go through supported devices and load them

        Loops through the available hardware classes, loops through
        each device in the system and adds it if needs be.
        """
        devices = os.listdir('/sys/bus/hid/devices')
        classes = razer_daemon.hardware.get_device_classes()
        self.logger.debug('Finding devices')

        device_number = 0
        for device_class in classes:
            for device_id in devices:
                if device_class.match(device_id):
                    self.logger.info('Found device.%d: %s', device_number, device_id)
                    device_path = os.path.join('/sys/bus/hid/devices', device_id)
                    razer_device = device_class(device_path, device_number)
                    device_serial = razer_device.get_serial()
                    self._razer_devices[device_serial] = razer_device

                    device_number += 1

    def run(self):
        """
        Run the daemon
        """

        self.logger.info('Serving DBus')

        # Can't just use mainloop.run() as that blocks and
        # then signaling exit doesn't work
        main_loop_context = self._main_loop.get_context()
        while self._main_loop is not None:
            if main_loop_context.pending():
                main_loop_context.iteration()
            else:
                time.sleep(0.001)

    def quit(self, signum, frame):
        """
        Quit by stopping the main loop and screensaver thread
        """
        self.logger.info('Stopping daemon')
        self._main_loop = None

        # Stop screensaver
        self._screensaver_thread.shutdown = True
        self._screensaver_thread.join(timeout=2)
        if self._screensaver_thread.is_alive():
            self.logger.warning('Could not stop the screensaver thread')


if __name__ == '__main__':
    # pylint: disable=invalid-name
    daemon = Daemon(logging_level=logging.DEBUG)
    daemon.run()



