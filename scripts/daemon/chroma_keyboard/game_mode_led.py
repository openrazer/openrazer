import sys
import argparse

import openrazer.client

choices = ('get', 'on', 'off')
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


# Check keyboard has game_mode
if not keyboard.has('game_mode_led'):
    print("Keyboard doesn't have game mode", file=sys.stderr)
    sys.exit(1)


if args.action == 'get':
    if keyboard.game_mode_led:
        print("Game mode LED: On")
    else:
        print("Game mode LED: Off")
elif args.action == 'on':
    keyboard.game_mode_led = True
    print("Turned game mode LED on")
else:
    keyboard.game_mode_led = False
    print("Turned game mode LED off")
