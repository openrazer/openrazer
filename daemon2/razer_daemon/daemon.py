"""
Daemon class

This class is the main core of the daemon, this serves a basic dbus module to control the main bit of the daemon
"""
import logging
import os
import signal
import time

from dbus.mainloop.glib import DBusGMainLoop, threads_init
from gi.repository import GObject

import razer_daemon.hardware
from razer_daemon.dbus_services.service import DBusService
from razer_daemon.device import DeviceCollection
from razer_daemon.misc.screensaver_thread import ScreensaverThread

DEVICE_CHECK_INTERVAL = 5000 # Milliseconds

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
        threads_init()
        DBusGMainLoop(set_as_default=True)
        DBusService.__init__(self, self.BUS_PATH, '/org/razer')

        self._main_loop = GObject.MainLoop()

        self.logger = logging.getLogger('razer')
        self.logger.setLevel(logging_level)
        formatter = logging.Formatter('%(asctime)s | %(name)-30s | %(levelname)-8s | %(message)s', datefmt='%Y-%m-%d %H:%M:%S')
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

        self._razer_devices = DeviceCollection()
        self._load_devices()

        # Add DBus methods
        self.logger.info("Adding razer.devices.getDevices method to DBus")
        self.add_dbus_method('razer.devices', 'getDevices', self.get_serial_list, out_signature='as')
        self.logger.info("Adding razer.devices.enableTurnOffOnScreensaver method to DBus")
        self.add_dbus_method('razer.devices', 'enableTurnOffOnScreensaver', self.enable_turn_off_on_screensaver)
        self.logger.info("Adding razer.devices.disableTurnOffOnScreensaver method to DBus")
        self.add_dbus_method('razer.devices', 'disableTurnOffOnScreensaver', self.disable_turn_off_on_screensaver)
        self.logger.info("Adding razer.devices.syncEffects method to DBus")
        self.add_dbus_method('razer.devices', 'syncEffects', self.sync_effects, in_signature='b')

        # TODO remove
        self.sync_effects(True)
        # TODO ======

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

    def _load_devices(self):
        """
        Go through supported devices and load them

        Loops through the available hardware classes, loops through
        each device in the system and adds it if needs be.
        """
        devices = os.listdir('/sys/bus/hid/devices')
        classes = razer_daemon.hardware.get_device_classes()

        device_number = 0
        for device_class in classes:
            for device_id in devices:
                if device_id in self._razer_devices:
                    continue

                if device_class.match(device_id):
                    self.logger.info('Found device.%d: %s', device_number, device_id)
                    device_path = os.path.join('/sys/bus/hid/devices', device_id)
                    razer_device = device_class(device_path, device_number)
                    device_serial = razer_device.get_serial()
                    self._razer_devices.add(device_id, device_serial, razer_device)

                    device_number += 1

    def _remove_devices(self):
        """
        Go through the list of current devices and if they no longer exist then remove them
        """
        devices_to_remove = []

        for device in self._razer_devices:
            device_path = os.path.join('/sys/bus/hid/devices', device.device_id)
            if not os.path.exists(device_path):
                # Remove from DBus
                device.dbus.remove_from_connection()
                devices_to_remove.append(device.device_id)

        for device_id in devices_to_remove:
            # Remove device
            self.logger.warning("Device %s is missing. Removing from DBus", device_id)
            del self._razer_devices[device_id]

    def run(self):
        """
        Run the daemon
        """
        self.logger.info('Serving DBus')

        # Counter for managing periodic tasks
        counter = 0

        # Can't just use mainloop.run() as that blocks and
        # then signaling exit doesn't work
        main_loop_context = self._main_loop.get_context()
        while self._main_loop is not None:
            if main_loop_context.pending():
                main_loop_context.iteration()
            else:
                time.sleep(0.001)

            if counter > DEVICE_CHECK_INTERVAL: # Time sleeps 1ms so DEVICE_CHECK_INTERVAL is in milliseconds
                self._remove_devices()
                self._load_devices()
                counter = 0
            counter += 1

    def quit(self, signum, frame):
        """
        Quit by stopping the main loop and screensaver thread
        """
        # pylint: disable=unused-argument
        self.logger.info('Stopping daemon.')
        self._main_loop = None

        # Stop screensaver
        self._screensaver_thread.shutdown = True
        self._screensaver_thread.join(timeout=2)
        if self._screensaver_thread.is_alive():
            self.logger.warning('Could not stop the screensaver thread')

        for device in self._razer_devices:
            device.dbus.close()


if __name__ == '__main__':
    # pylint: disable=invalid-name
    daemon = Daemon(logging_level=logging.DEBUG)

    try:
        daemon.run()
    except Exception as err:
        daemon.logger.exception("Caught exception", exc_info=err)



