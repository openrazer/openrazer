#!/usr/bin/python3
import argparse
import glob
import os
import time
import random
import sys


def clamp_to_min_max(value, min, max):
    if value > max:
        value = max
    elif value < min:
        value = min
    return value


def clamp_to_u8(value):
    return clamp_to_min_max(value, 0, 255)


def parse_args():
    parser = argparse.ArgumentParser(description="Set a custom colour effect")
    parser.add_argument('-d', '--device', type=str, help="Device string like \"0003:1532:0045.000C\"")

    args = parser.parse_args()
    return args


def run():
    args = parse_args()

    if args.device is None:
        mouse_dirs = glob.glob(os.path.join('/sys/bus/hid/drivers/razermouse/', "*:*:*.*"))

        if len(mouse_dirs) > 1:
            print("Multiple mouse directories found. Rerun with -d", file=sys.stderr)
            sys.exit(1)
        if len(mouse_dirs) < 1:
            print("No mouse directories found. Make sure the driver is binded", file=sys.stderr)
            sys.exit(1)

        mouse_dir = mouse_dirs[0]
    else:
        mouse_dir = os.path.join('/sys/bus/hid/drivers/razermouse/', args.device)

    if not os.path.isdir(mouse_dir):
        print("Multiple mouse directories found. Rerun with -d", file=sys.stderr)
        sys.exit(1)

    #choices = [b'\xff\x00\x00', b'\xff\xff\x00', b'\x00\xff\x00', b'\x00\xff\xff', b'\x00\x00\xff', b'\xff\x00\xff']
    choices = [b'\xff\x00\x00']

    for repeat in range(0, 10):
        payload = b''
        for i in range(0, 15):
            payload += random.choice(choices)

        set_colour_filename = os.path.join(mouse_dir, "set_key_row")
        set_custom_mode_filename = os.path.join(mouse_dir, "mode_custom")
        with open(set_colour_filename, 'wb') as set_colour_file:
            set_colour_file.write(payload)
        with open(set_custom_mode_filename, 'w') as set_custom_mode_file:
            set_custom_mode_file.write("1")
        time.sleep(0.2)

    print("Done")


if __name__ == '__main__':
    run()
