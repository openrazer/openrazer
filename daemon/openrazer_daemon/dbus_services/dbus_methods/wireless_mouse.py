from openrazer_daemon.dbus_services import endpoint


@endpoint('razer.device.misc', 'getSleepState', out_sig='y')
def get_sleep_state(self):
    """
    Get the device's current sleep state

    :return: The device's current sleep state (0 = awake, 1 = asleep, 2 = unknown)
    :rtype: int
    """
    self.logger.debug("DBus call get_sleep_state")

    driver_path = self.get_driver_path('sleep_state')

    with open(driver_path, 'r') as driver_file:
        return int(driver_file.read().strip())

