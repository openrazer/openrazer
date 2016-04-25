from razer_daemon.dbus import endpoint, endpoint2

@endpoint2('device.misc', out_sig='s')
def get_firmware(self):
    driver_path = self.get_driver_path('get_firmware_version')

    with open(driver_path, 'r') as driver_file:
        return driver_file.read()


# Becomes endpoint(int_name, out_sig)(getfirmware)