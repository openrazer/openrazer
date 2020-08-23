from openrazer.client import DeviceManager
from evdev import ecodes

# Create a DeviceManager. This is used to get specific devices
device_manager = DeviceManager()

print("Found {} Razer devices".format(len(device_manager.devices)))
print()

# Disable daemon effect syncing.
# Without this, the daemon will try to set the lighting effect to every device.
device_manager.sync_effects = False

# Iterate over each device
for device in device_manager.devices:
    print("Device Profiles: {}".format(device.binding.get_profiles()))

    print("Maps for Profile Default: {}".format(device.binding.get_maps("Default")))

    # pylint: disable=no-member
    # Add an key action that replaces the 1 key with 2
    device.binding.add_action("Default", "Default", ecodes.KEY_1, "key", str(ecodes.KEY_2))

    # Clear all actions for that key
    # device.binding.clear_actions("Default", "Default", 2)
