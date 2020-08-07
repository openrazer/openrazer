import colorsys
import random

from openrazer.client import DeviceManager
from openrazer.client import constants as razer_constants

# Create a DeviceManager. This is used to get specific devices
device_manager = DeviceManager()


print("Found {} Razer devices".format(len(device_manager.devices)))
print()

# Disable daemon effect syncing.
# Without this, the daemon will try to set the lighting effect to every device.
device_manager.sync_effects = False

# List of effect I've chosen to make an example for
effects = [
    'fan_mode',
    'fan_speed'
]

mode_manual = 0x01
mode_automatic = 0x01

game_mode_balanced = 0x00
game_mode_gaming = 0x01
game_mode_creator = 0x02
game_mode_custom = 0x04  # CPU boost and GPU boost power
high_speed = 0x36
low_speed = 0x2b

# first read state, then write new state
for device in device_manager.devices:
    # Check which effect this device supports.
    device_effects = [effect for effect in effects if device.fx.has(effect)]
    # print("{} supports {}".format(device.name, device_effects))

    if len(device_effects) == 0:
        print("Device {} doesn't support any of the effects".format(device.name))
        continue
    effect = random.choice(device_effects)
    print("Setting {} to effect {}".format(device.name, effect))

    # Ad an example for each effect
    if effect == 'fan_mode':
        device.fx.fan_mode(mode_automatic, game_mode_balanced)

    elif effect == 'fan_speed':
        result = device.fx.fan_speed()
        print("current fan_speed: {0}" % (result))
        device.fx.fan_speed(high_speed)

        result = device.fx.fan_mode()
        print("current fan mode: {0}" % (result))
        device.fx.fan_mode(mode_manual, game_mode_custom)
