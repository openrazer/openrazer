# SPDX-License-Identifier: GPL-2.0-or-later

"""
This will do until I can be bothered to create indicator applet to do battery level
"""
import logging
import threading
import datetime
import time

try:
    import notify2
except ImportError:
    notify2 = None


# TODO https://askubuntu.com/questions/110969/notify-send-ignores-timeout
NOTIFY_TIMEOUT = 4000


class BatteryNotifier(threading.Thread):
    """
    Thread to notify about battery
    """

    def __init__(self, parent, device_id, device_name):
        super().__init__()
        self._logger = logging.getLogger('razer.device{0}.batterynotifier'.format(device_id))
        self._notify2 = notify2 is not None

        self.event = threading.Event()
        self.frequency = 0
        self.percent = 0

        if self._notify2:
            try:
                notify2.init('OpenRazer')
            except Exception as err:
                self._logger.warning("Failed to init notification daemon, err: {0}".format(err))
                self._notify2 = False

        self._shutdown = False
        self._device_name = device_name

        # Could save reference to parent but only need battery level function
        self._get_battery_func = parent.getBattery

        if self._notify2:
            self._notification = notify2.Notification(summary=device_name)
            self._notification.set_timeout(NOTIFY_TIMEOUT)

        self._last_notify_time = datetime.datetime(1970, 1, 1)

    @property
    def shutdown(self):
        """
        Get the shutdown flag
        """
        return self._shutdown

    @shutdown.setter
    def shutdown(self, value):
        """
        Set the shutdown flag

        :param value: Shutdown
        :type value: bool
        """
        self._shutdown = value

    def notify_battery(self):
        now = datetime.datetime.now()

        if (now - self._last_notify_time).seconds > self.frequency:
            # Update last notified
            self._last_notify_time = now

            battery_level = self._get_battery_func()
            battery_percent = int(round(battery_level, 0))

            # Sometimes on wifi don't get batt
            if battery_level == -1.0:
                time.sleep(0.2)
                battery_level = self._get_battery_func()

            title = self._device_name
            message = "Battery is {0}%".format(battery_percent)
            icon = "battery-full"

            if battery_level == 0.0:
                # Do nothing
                pass

            elif battery_level <= 10.0:
                message = "Battery is low ({0}%). Please charge your device".format(battery_percent)
                icon = "battery-empty"

            elif battery_level <= 30.0:
                icon = "battery-low"

            elif battery_level <= 70.0:
                icon = "battery-good"

            elif battery_level == 100.0:
                message = "Battery is fully charged ({0}%)".format(battery_percent)

            if self._notify2:
                self._logger.debug("{0} Battery at {1}%".format(self._device_name, battery_percent))

                if battery_level <= self.percent:
                    self._notification.update(summary=title, message=message, icon=icon)
                    self._notification.show()

    def run(self):
        """
        Main thread function
        """

        while not self._shutdown:
            if self.event.is_set() and self.frequency > 0:
                self.notify_battery()

            time.sleep(0.1)

        self._logger.debug("Shutting down battery notifier")


class BatteryManager(object):
    """
    Class which manages the overall process of notifing battery levels
    """

    def __init__(self, parent, device_number, device_name):
        self._logger = logging.getLogger('razer.device{0}.batterymanager'.format(device_number))
        self._parent = parent

        self._battery_thread = BatteryNotifier(parent, device_number, device_name)
        self._battery_thread.start()

        self._is_closed = False

    def close(self):
        """
        Close the manager, stop ripple thread
        """
        if not self._is_closed:
            self._logger.debug("Closing Battery Manager")
            self._is_closed = True

            self._battery_thread.shutdown = True
            self._battery_thread.join(timeout=2)
            if self._battery_thread.is_alive():
                self._logger.error("Could not stop BatteryNotify thread")

    def __del__(self):
        self.close()

    @property
    def active(self):
        return self._battery_thread.event.is_set()

    @active.setter
    def active(self, value):
        if value:
            self._battery_thread.event.set()
        else:
            self._battery_thread.event.clear()

    @property
    def frequency(self):
        return self._battery_thread.frequency

    @frequency.setter
    def frequency(self, frequency):
        self._battery_thread.frequency = frequency

    @property
    def percent(self):
        return self._battery_thread.percent

    @percent.setter
    def percent(self, percent):
        self._battery_thread.percent = percent
