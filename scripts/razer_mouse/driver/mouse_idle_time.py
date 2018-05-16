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
    parser = argparse.ArgumentParser(description="Set the mouse idle time")
    parser.add_argument('-d', '--device', type=str, help="Device string like \"0003:1532:0045.000C\"")

    parser.add_argument('--timeout', required=True, type=int, help="Seconds (60-900)")

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

    seconds = clamp_to_min_max(args.seconds, 60, 900)
    byte_string = bytes(str(seconds), 'utf-8')  # Convert string to bytestring

    mouse_idle_time_filepath = os.path.join(mouse_dir, "set_idle_time")
    with open(mouse_idle_time_filepath, 'wb') as mouse_idle_time_file:
        mouse_idle_time_file.write(byte_string)
    print("Done")


if __name__ == '__main__':
    run()
