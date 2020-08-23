from openrazer.client import DeviceManager
from evdev import ecodes

# Create a DeviceManager. This is used to get specific devices
device_manager = DeviceManager()

print("Found {} Razer devices".format(len(device_manager.devices)))

devices = device_manager.devices
for device in devices:
    if not device.fx.advanced:
        print("Skipping device " + device.name + " (" + device.serial + ")")
        devices.remove(device)

print()

# Disable daemon effect syncing.
# Without this, the daemon will try to set the lighting effect to every device.
device_manager.sync_effects = False

# Iterate over each device
for device in device_manager.devices:
    print("Device Profiles: {}".format(device.binding.get_profiles()))
    print("Maps for Profile Default: {}".format(device.binding.get_maps("Default")))

    # Add a new map named Example
    device.binding.add_map("Default", "Example")

    # Add a custom matrix effect to the default map
    device.fx.advanced.matrix[1, 1] = (0, 255, 0)
    device.fx.advanced.matrix[1, 2] = (0, 255, 0)
    device.fx.advanced.matrix[1, 3] = (0, 255, 0)
    device.fx.advanced.matrix[1, 4] = (0, 255, 0)
    device.binding.set_matrix("Default", "Default", device.fx.advanced.matrix.to_dict())

    # Add a custom matrix to our new map
    device.fx.advanced.matrix[1, 1] = (255, 0, 0)
    device.fx.advanced.matrix[1, 2] = (255, 0, 0)
    device.fx.advanced.matrix[1, 3] = (255, 0, 0)
    device.fx.advanced.matrix[1, 4] = (255, 0, 0)
    device.binding.set_matrix("Default", "Example", device.fx.advanced.matrix.to_dict())

    # pylint: disable=no-member
    # Add an key action that changes the map when you press the 1 key
    device.binding.add_action("Default", "Default", ecodes.KEY_1, "map", "Example")
    device.binding.add_action("Default", "Example", ecodes.KEY_1, "map", "Default")
