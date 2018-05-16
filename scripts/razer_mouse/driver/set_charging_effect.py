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
    parser = argparse.ArgumentParser(description="Set the wave effect")
    parser.add_argument('-d', '--device', type=str, help="Device string like \"0003:1532:0045.000C\"")

    parser.add_argument('--effect', required=True, type=int, help="Set whether or not to use static colour or last mouse effect for charging. 0 use mouse effect, 1 use colour")

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

    number = clamp_to_min_max(args.effect, 0, 1)

    if number:
        byte_string = struct.pack(">B", 0x01)
    else:
        byte_string = struct.pack(">B", 0x00)

    set_charging_effect_filepath = os.path.join(mouse_dir, "set_charging_effect")
    with open(set_charging_effect_filepath, 'wb') as set_charging_effect_file:
        set_charging_effect_file.write(byte_string)
    print("Done")


if __name__ == '__main__':
    run()
