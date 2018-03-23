import sys

import openrazer.client

device_manager = openrazer.client.DeviceManager()
keyboard = None


def on_off(value, true='On', false='Off'):
    if value:
        return true
    else:
        return false


for device in device_manager.devices:
    if device.type == 'keyboard':
        keyboard = device
        break
else:
    print("Could not find suitable keyboard", file=sys.stderr)
    sys.exit(1)

print("Name: {0}".format(keyboard.name))
print("Type: {0}".format(keyboard.type))
print("Serial: {0}".format(keyboard.serial))
print("Firmware Version: {0}".format(keyboard.firmware_version))
print("------------------------")
print("Brightness: {0}".format(keyboard.brightness))
if keyboard.has("game_mode_led"):
    print("Game Mode LED: {0}".format(on_off(keyboard.game_mode_led)))
if keyboard.has("macro_mode_led"):
    macro_led = on_off(keyboard.macro_mode_led)
    macro_led_state = on_off(keyboard.macro_mode_led_effect == openrazer.client.constants.MACRO_LED_STATIC, true='Static', false='Blinking')
    print("Macro Mode LED: {0} ({1})".format(macro_led, macro_led_state))
print("------------------------")
print("Capabilities:")
for key, value in sorted(keyboard.capabilities.items(), key=lambda x: x[0]):
    print("  {0}: {1}".format(key, value))
