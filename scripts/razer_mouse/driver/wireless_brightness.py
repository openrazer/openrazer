#!/usr/bin/python3
import argparse
import glob
import os
import struct
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
    parser = argparse.ArgumentParser(description="Set the wireless brightness")
    parser.add_argument('-d', '--device', type=str, help="Device string like \"0003:1532:0045.000C\"")

    parser.add_argument('--brightness', required=True, type=int, help="Brightness (0-100)")

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

    brightness = clamp_to_min_max(args.brightness, 0, 100)
    brightness_scaled = clamp_to_u8(int(round((255 / 100) * brightness, 0)))
    byte_string = bytes(str(brightness_scaled), 'utf-8')  # Convert string to bytestring

    wireless_brightness_filepath = os.path.join(mouse_dir, "set_wireless_brightness")
    with open(wireless_brightness_filepath, 'wb') as wireless_brightness_file:
        wireless_brightness_file.write(byte_string)
    print("Done")


if __name__ == '__main__':
    run()
