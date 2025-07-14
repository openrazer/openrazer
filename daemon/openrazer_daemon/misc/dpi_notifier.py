import logging
import threading
import datetime
import time
import subprocess


class DpiNotifier(threading.Thread):
    """
    Thread to notify about dpi changes
    """

    def __init__(self, parent, device_id, device_name):
        super().__init__()
        self._logger = logging.getLogger('razer.device{0}.dpinotifier'.format(device_id))

        self.event = threading.Event()

        self._shutdown = False
        self._device_name = device_name

        self._get_dpi_xy_func = parent.getDPI
        self._last_dpi = self._get_dpi_xy_func()
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

    def show_notification(self, summary: str, message: str) -> None:
        try:
            subprocess.run(["notify-send", "-a", "OpenRazer", "-t", "4000", summary, message],
                           check=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        except subprocess.CalledProcessError as e:
            self._logger.warning(f"Failed to show notification: {e.output.strip()}")

    def notify_dpi(self):
        dpi = self._get_dpi_xy_func()
        if dpi == self._last_dpi:
            return

        self._last_dpi = dpi

        # Sometimes due to various issues we don't get the percentage correctly.
        # Just ignore them and don't show a bogus notification.
        # See also: https://github.com/openrazer/openrazer/issues/2122
        # if battery_level in (0.0, -1.0):
        #     self._logger.debug("Got bogus battery value: {0}, ignoring.".format(battery_level))
        #     # Since we don't update _last_notify_time here we're going to retry very soon again.
        #     # Sleep a bit so we don't spam the device with requests.
        #     time.sleep(10)
        #     return
        # Update the last notified time so that we alert in the configured frequency.

        title = self._device_name
        message = "DPI is {0}".format(dpi)
        self._logger.debug("{0} DPI at {1}".format(self._device_name, dpi))

        self.show_notification(summary=title, message=message)

    def run(self):
        """
        Main thread function
        """

        while not self._shutdown:
            if self.event.is_set():
                self.notify_dpi()

            time.sleep(0.1)

        self._logger.debug("Shutting down dpi notifier")


class DpiManager:
    """
    Class which manages the overall process of notifing battery levels
    """

    def __init__(self, parent, device_number, device_name):
        self._logger = logging.getLogger('razer.device{0}.dpimanager'.format(device_number))
        self._parent = parent

        self._dpi_thread = DpiNotifier(parent, device_number, device_name)

        self._dpi_thread.start()

        self._is_closed = False

    def close(self):
        """
        Close the manager, stop ripple thread
        """
        if not self._is_closed:
            self._logger.debug("Closing Dpi Manager")
            self._is_closed = True

            self._dpi_thread.shutdown = True
            self._dpi_thread.join(timeout=2)
            if self._dpi_thread.is_alive():
                self._logger.error("Could not stop DpiNotifier thread")

    def __del__(self):
        self.close()

    @property
    def active(self):
        return self._dpi_thread.event.is_set()

    @active.setter
    def active(self, value):
        if value:
            self._dpi_thread.event.set()
        else:
            self._dpi_thread.event.clear()

