from openrazer.client import DeviceManager
from openrazer.client import constants as razer_constants

# Create a DeviceManager. This is used to get specific devices
device_manager = DeviceManager()


print("Found {} Razer devices".format(len(device_manager.devices)))
print()

# Disable daemon effect syncing.
# Without this, the daemon will try to set the lighting effect to every device.
device_manager.sync_effects = False

# Iterate over each device and set the wave effect
for device in device_manager.devices:
    print("Setting {} to wave".format(device.name))

    # Set the effect to wave.
    # wave requires a direction, but different effect have different arguments.
    device.fx.wave(razer_constants.WAVE_LEFT)
