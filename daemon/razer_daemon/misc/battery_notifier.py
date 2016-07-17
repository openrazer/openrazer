"""
This will do until I can be bothered to create indicator applet to do battery level
"""
import logging
import threading
import datetime
import time
import notify2


# TODO add python3-notify2 to dependencies
# TODO https://askubuntu.com/questions/110969/notify-send-ignores-timeout
INTERVAL_FREQ = 60 * 10
NOTIFY_TIMEOUT = 4000


class BatteryNotifier(threading.Thread):
    """
    Thread to notify about battery
    """
    def __init__(self, parent, device_id, device_name):
        super(BatteryNotifier, self).__init__()

        notify2.init('razer_daemon')

        self._logger = logging.getLogger('razer.device{0}.batterynotifier'.format(device_id))

        self._shutdown = False
        self._device_name = device_name

        # Could save reference to parent but only need battery level function
        self._get_battery_func = parent.getBattery

        self._notification = notify2.Notification(summary="{0}")
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

        if (now - self._last_notify_time).seconds > INTERVAL_FREQ:
            # Update last notified
            self._last_notify_time = now

            battery_level = self._get_battery_func()
            if battery_level < 10.0:
                self._notification.update(summary="{0} Battery at {1:.1f}%".format(self._device_name, battery_level), message='Please charge your device', icon='notification-battery-low')
            else:
                self._notification.update(summary="{0} Battery at {1:.1f}%".format(self._device_name, battery_level))

            self._notification.show()


    def run(self):
        """
        Main thread function
        """

        while not self._shutdown:
            self.notify_battery()

            time.sleep(0.1)


        self._logger.debug("Shutting down battery notifier")

class BatteryManager(object):
    """
    Class which manages the overall process of notifing battery levels
    """
    def __init__(self, parent, device_number, device_name):
        self._logger = logging.getLogger('razer.device{0}.ripplemanager'.format(device_number))
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