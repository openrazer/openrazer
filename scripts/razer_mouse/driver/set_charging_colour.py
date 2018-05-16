#!/usr/bin/python3
import argparse
import glob
import os
import struct
import sys


def clamp_to_u8(value):
    if value > 255:
        value = 255
    elif value < 0:
        value = 0
    return value


def parse_args():
    parser = argparse.ArgumentParser(description="Set the charging colour")
    parser.add_argument('-d', '--device', type=str, help="Device string like \"0003:1532:0045.000C\"")

    parser.add_argument('--colour', required=True, nargs=3, metavar=("R", "G", "B"), type=int, help="Charging colour")

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

    values = map(clamp_to_u8, args.colour)
    byte_string = struct.pack(">BBB", *values)

    set_charging_colour_filepath = os.path.join(mouse_dir, "set_charging_colour")
    with open(set_charging_colour_filepath, 'wb') as set_charging_colour_file:
        set_charging_colour_file.write(byte_string)
    print("Done")


if __name__ == '__main__':
    run()
