# SPDX-License-Identifier: GPL-2.0-or-later

"""
Watches a mouse's activity/idle transitions.

Some wireless mice (e.g. the Naga V3 Pro) stop responding to USB control
transfers while idle, so any hardware write attempted during that time (like
setting "driver mode") raises a TimeoutError.

Right after waking up, the receiver can also answer commands with an empty
"busy" response for a while. The driver reports those as success, so reads
return empty data (e.g. an empty serial number) and writes are silently
dropped.

Activity is read from the driver's "device_last_activity" file, which reports
how long ago the kernel driver last saw an input report from the mouse.
This module is used for:

* Block device initialisation until the mouse is in use and answers control
  transfers with real data, so the driver doesn't try to talk to a sleeping
  device.
* Watch for the mouse waking up from its own idle state at runtime and
  re-apply "driver mode", since some devices silently drop back to "device
  mode" while idle.
"""
import logging
import os
import re
import threading
import time

POLL_INTERVAL = 0.2
IDLE_TIME_REFRESH = 30.0
NO_ACTIVITY = -1


def _read_last_activity(last_activity_path):
    """
    Read the driver's "device_last_activity" file.

    :param last_activity_path: Path of the driver's device_last_activity file
    :type last_activity_path: str

    :return: Milliseconds since the device's last input report, NO_ACTIVITY if
             it hasn't reported anything yet, or None if the file is unreadable
    :rtype: int
    """
    try:
        with open(last_activity_path, 'r') as driver_file:
            return int(driver_file.read().strip())
    except (OSError, ValueError):
        return NO_ACTIVITY


def _is_device_serial_ready(device_path):
    """
    Check whether the device answers control transfers with real data.

    A "busy" response from the receiver carries no data, so reading the serial
    number tells it apart from a real answer.

    :param device_path: Device path
    :type device_path: str

    :return: True if the device returned a valid serial number
    :rtype: bool
    """
    try:
        with open(os.path.join(device_path, 'device_serial'), 'r') as driver_file:
            serial = driver_file.read().strip()
    except (OSError, UnicodeDecodeError):
        return False

    return re.fullmatch(r"[\dA-Z]+", serial) is not None


def wait_until_ready(device_path, device_name):
    """
    Block until the mouse is awake and answers control transfers.

    Logs once that the device is idle. Once the driver has seen a recent input
    report, polls the device until it answers with real data, giving up after
    READY_TIMEOUT seconds of being awake so a device that never returns a
    valid serial number still gets initialised.

    :param device_path: Device path
    :type device_path: str

    :param device_name: Device name, used for the log messages
    :type device_name: str
    """
    logger = logging.getLogger('razer.misc.mousemonitor')
    last_activity_path = os.path.join(device_path, 'device_last_activity')

    awake_window = 5000
    logged_idle = False
    deadline_timeout = 10.0
    deadline = None
    while True:
        last_activity = _read_last_activity(last_activity_path)
        if last_activity != NO_ACTIVITY and last_activity < awake_window:
            if _is_device_serial_ready(device_path):
                return

            if deadline is None:
                deadline = time.monotonic() + deadline_timeout
            elif time.monotonic() >= deadline:
                logger.warning("%s is not answering control transfers, initialising anyway", device_name)
                return

        elif not logged_idle:
            logger.info("%s in idle state", device_name)
            logged_idle = True

        time.sleep(POLL_INTERVAL)


class MouseMonitor(threading.Thread):
    """
    Thread that watches the driver's "device_last_activity" file, and once the
    device wakes up after being idle for longer than its configured idle time,
    re-applies "driver mode" if the device is meant to be in it.
    """

    def __init__(self, device_id, parent):
        super().__init__()

        self._logger = logging.getLogger('razer.device{0}.mousemonitor'.format(device_id))
        self._parent = parent
        self._shutdown = False

        self._last_sample = None
        self._idle = False
        self._pending_driver_mode = False
        self._idle_time = 0
        self._idle_time_read = 0.0

    @property
    def shutdown(self):
        """
        Thread shutdown condition
        """
        return self._shutdown

    @shutdown.setter
    def shutdown(self, value):
        self._shutdown = value

    def _get_idle_time(self):
        """
        Read the device's configured idle time, in seconds.

        Reading it costs a USB control transfer, so the value is cached and
        only refreshed every IDLE_TIME_REFRESH seconds.
        """
        now = time.monotonic()
        if now - self._idle_time_read < IDLE_TIME_REFRESH:
            return self._idle_time

        try:
            with open(self._parent.get_driver_path('device_idle_time'), 'r') as driver_file:
                self._idle_time = int(driver_file.read().strip())
        except (OSError, ValueError):
            self._idle_time = 0

        self._idle_time_read = now
        return self._idle_time

    def _apply_driver_mode(self):
        """
        Re-apply "driver mode".

        The device can still be asleep right after it starts reporting again,
        in which case the write times out. Keep it pending so the next poll
        tries again instead of leaving the device in "device mode".
        """
        try:
            self._parent.set_device_mode(0x03, 0x00)
        except OSError as error:
            self._logger.debug('Device not ready for "driver mode" yet: %s', error)
            return

        self._pending_driver_mode = False
        self._logger.info('Re-applied "driver mode" after idle')

    def run(self):
        device_name = self._parent.__class__.__name__

        while not self._shutdown:
            time.sleep(POLL_INTERVAL)

            sample = _read_last_activity(self._parent.get_driver_path('device_last_activity'))
            if sample == NO_ACTIVITY:
                continue

            activity = self._last_sample is None or sample < self._last_sample
            self._last_sample = sample

            if activity and self._idle:
                self._idle = False
                self._pending_driver_mode = self._parent.DRIVER_MODE

            if self._pending_driver_mode:
                self._apply_driver_mode()

            if activity:
                continue

            if self._idle:
                continue

            idle_time = self._get_idle_time()
            if idle_time and sample > idle_time * 1000:
                self._idle = True
                self._logger.info("%s in idle state", device_name)
