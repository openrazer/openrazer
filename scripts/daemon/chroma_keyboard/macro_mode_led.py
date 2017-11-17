import sys
import argparse

import openrazer.client

choices = ('get', 'on', 'off', 'flashing', 'static', 'effect_state')
parser = argparse.ArgumentParser()
parser.add_argument('action', choices=choices)
args = parser.parse_args()

device_manager = openrazer.client.DeviceManager()
keyboard = None

for device in device_manager.devices:
    if device.type == 'keyboard':
        keyboard = device
        break
else:
    print("Could not find suitable keyboard", file=sys.stderr)
    sys.exit(1)


# Check keyboard has macro_mode
if not keyboard.has('macro_mode_led'):
    print("Keyboard doesn't have macro mode", file=sys.stderr)
    sys.exit(1)


if args.action == 'get':
    if keyboard.macro_mode_led:
        print("Macro mode LED: On")
    else:
        print("Macro mode LED: Off")

elif args.action == 'effect_state':
    if keyboard.macro_mode_led_effect == openrazer.client.constants.MACRO_LED_BLINK:
        print("Macro mode LED: Blinking")
    else:
        print("Macro mode LED: Static")

elif args.action == 'on':
    keyboard.macro_mode_led = True
    print("Turned macro mode LED on")

elif args.action == 'flashing':
    keyboard.macro_mode_led = True
    keyboard.macro_mode_led_effect = openrazer.client.constants.MACRO_LED_BLINK
    print("Turned macro mode LED on (flashing)")

elif args.action == 'static':
    keyboard.macro_mode_led = True
    keyboard.macro_mode_led_effect = openrazer.client.constants.MACRO_LED_STATIC
    print("Turned macro mode LED on (static)")

else:  # Off
    keyboard.macro_mode_led = False
    print("Turned macro mode LED off")
