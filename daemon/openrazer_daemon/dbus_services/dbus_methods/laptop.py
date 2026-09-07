# SPDX-License-Identifier: GPL-2.0-or-later

"""
DBus methods for laptop devices (Razer Blade).
"""
from openrazer_daemon.dbus_services import endpoint


@endpoint('razer.device.laptop', 'getPowerMode', out_sig='u')
def get_power_mode(self):
    """
    Get the laptop power mode

    :return: Power mode (0 balanced, 1 gaming, 2 creator, 4 custom)
    :rtype: int
    """
    self.logger.debug("DBus call get_power_mode")

    return int(self.power_mode)


@endpoint('razer.device.laptop', 'setPowerMode', in_sig='u')
def set_power_mode(self, power_mode):
    """
    Set the laptop power mode

    :param power_mode: Power mode (0 balanced, 1 gaming, 2 creator, 4 custom)
    :type power_mode: int
    """
    self.logger.debug("DBus call set_power_mode")

    driver_path = self.get_driver_path('power_mode')

    # remember power mode
    self.power_mode = power_mode
    self.set_persistence('laptop', 'power_mode', power_mode)

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(power_mode))


@endpoint('razer.device.laptop', 'getFanRpm', out_sig='u')
def get_fan_rpm(self):
    """
    Get the laptop fan speed

    :return: Fan speed in RPM (0 means auto)
    :rtype: int
    """
    self.logger.debug("DBus call get_fan_rpm")

    return int(self.fan_rpm)


@endpoint('razer.device.laptop', 'setFanRpm', in_sig='u')
def set_fan_rpm(self, fan_rpm):
    """
    Set the laptop fan speed, 0 for automatic

    :param fan_rpm: Fan speed in RPM (0 means auto)
    :type fan_rpm: int
    """
    self.logger.debug("DBus call set_fan_rpm")

    driver_path = self.get_driver_path('fan_rpm')

    # remember fan speed
    self.fan_rpm = fan_rpm
    self.set_persistence('laptop', 'fan_rpm', fan_rpm)

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(fan_rpm))


@endpoint('razer.device.laptop', 'getCpuBoost', out_sig='u')
def get_cpu_boost(self):
    """
    Get the CPU boost level

    :return: CPU boost level
    :rtype: int
    """
    self.logger.debug("DBus call get_cpu_boost")

    return int(self.cpu_boost)


@endpoint('razer.device.laptop', 'setCpuBoost', in_sig='u')
def set_cpu_boost(self, cpu_boost):
    """
    Set the CPU boost level

    :param cpu_boost: CPU boost level
    :type cpu_boost: int
    """
    self.logger.debug("DBus call set_cpu_boost")

    driver_path = self.get_driver_path('cpu_boost')

    # remember CPU boost level
    self.cpu_boost = cpu_boost
    self.set_persistence('laptop', 'cpu_boost', cpu_boost)

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(cpu_boost))


@endpoint('razer.device.laptop', 'getGpuBoost', out_sig='u')
def get_gpu_boost(self):
    """
    Get the GPU boost level

    :return: GPU boost level
    :rtype: int
    """
    self.logger.debug("DBus call get_gpu_boost")

    return int(self.gpu_boost)


@endpoint('razer.device.laptop', 'setGpuBoost', in_sig='u')
def set_gpu_boost(self, gpu_boost):
    """
    Set the GPU boost level

    :param gpu_boost: GPU boost level
    :type gpu_boost: int
    """
    self.logger.debug("DBus call set_gpu_boost")

    driver_path = self.get_driver_path('gpu_boost')

    # remember GPU boost level
    self.gpu_boost = gpu_boost
    self.set_persistence('laptop', 'gpu_boost', gpu_boost)

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(gpu_boost))
