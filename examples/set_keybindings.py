from openrazer.client import DeviceManager

# Create a DeviceManager. This is used to get specific devices
device_manager = DeviceManager()


print("Found {} Razer devices".format(len(device_manager.devices)))
print()


# Iterate over each device and pretty out some standard information about each
for device in device_manager.devices:
    if device.type == 'keypad':
        print("{}:".format(device.name))
        print("   type: {}".format(device.type))
        print("   serial: {}".format(device.serial))
        print("   firmware version: {}".format(device.firmware_version))
        print("   driver version: {}".format(device.driver_version))
        device.translations = {
            'KEY_1': 'KEY_A',
            'KEY_2': 'KEY_B',
            'KEY_3': 'KEY_C',
        }
        # device.translations = {
        #     'KEY_1': 'KEY_X',
        #     'KEY_2': 'KEY_Y',
        #     'KEY_3': 'KEY_Z',
        # }
        # clear bindings
        # device.translations = {}
        print()
