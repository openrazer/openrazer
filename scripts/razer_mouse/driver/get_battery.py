#!/usr/bin/python3
import argparse
import glob
import os
import sys


def parse_args():
    parser = argparse.ArgumentParser(description="Get battery level")
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

    battery_filepath = os.path.join(mouse_dir, "get_battery")
    with open(battery_filepath, 'r') as battery_file:
        # Battery percentage, before being scaled to 0-100
        try:
            battery_percentage_ns = int(battery_file.read().strip())
            scaled_percentage = (100 / 255) * battery_percentage_ns

            print("{0:.1f}%".format(scaled_percentage))

        except ValueError as ex:
            print("Failed to get battery percentage.\n{0}".format(ex), file=sys.stderr)
            sys.exit(1)


if __name__ == '__main__':
    run()
