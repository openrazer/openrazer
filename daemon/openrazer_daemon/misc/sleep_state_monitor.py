import logging
import threading
import time
import typing
from enum import Enum

if typing.TYPE_CHECKING:
    from openrazer_daemon.hardware.device_base import RazerDevice


class SleepState(Enum):
    AWAKE = 0
    ASLEEP = 1
    UNKNOWN = 2




class SleepStateMonitor(threading.Thread):
    """
    Thread to watch sleep state
    """

    def __init__(self, parent: "RazerDevice", device_id: str, device_name: str):
        super().__init__()
        self._logger = logging.getLogger(
            "razer.device{0}.sleepmonitor".format(device_id)
        )

        self.interval = 0.1
        self.state = SleepState.UNKNOWN

        self._shutdown = False
        self._device_name = device_name

        self.parent: "RazerDevice" = parent

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
    
    def close(self):
        if not self._shutdown:
            self._logger.debug("Closing sleep state monitor")
            self.shutdown=True
            self.join(timeout = 2)
            if self.is_alive():
                self._logger.error("Failed to stop sleep state monitoring for %s.", self._device_name)

    def run(self):
        """
        Main thread function
        """

        while not self._shutdown:
            new_state = SleepState(self.parent.getSleepState())
            if self.state != new_state:
                self.state = new_state
                self._logger.info(
                    "Sleep state for %s updated to %s", self._device_name, self.state.name
                )
                if new_state == SleepState.AWAKE:
                    self.parent.resume_device()
            time.sleep(self.interval)

        self._logger.debug("Shutting down Sleep State Monitor")
