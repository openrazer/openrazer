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
    parser = argparse.ArgumentParser(description="Set the breathing effect")
    parser.add_argument('-d', '--device', type=str, help="Device string like \"0003:1532:0045.000C\"")

    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('--random', action="store_true", help="Random breathing effect")
    group.add_argument('--single', nargs=3, metavar=("R", "G", "B"), type=int, help="Single colour breathing effect")
    group.add_argument('--dual', nargs=6, metavar=("R1", "G1", "B1", "R2", "G2", "B2"), type=int, help="Dual colour breathing effect")

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

    if args.random:
        byte_string = struct.pack(">B", 0x01)
    elif args.single is not None:
        values = map(clamp_to_u8, args.single)
        byte_string = struct.pack(">BBB", *values)
    elif args.dual is not None:
        values = map(clamp_to_u8, args.dual)
        byte_string = struct.pack(">BBBBBB", *values)
    else:
        # Should never get here
        byte_string = struct.pack(">B", 0x01)

    breathing_mode_filepath = os.path.join(mouse_dir, "mode_breath")
    with open(breathing_mode_filepath, 'wb') as breathing_mode_file:
        breathing_mode_file.write(byte_string)
    print("Done")


if __name__ == '__main__':
    run()
