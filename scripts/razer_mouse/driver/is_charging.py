#!/usr/bin/python3
import argparse
import glob
import os
import sys


def parse_args():
    parser = argparse.ArgumentParser(description="Get charging status")
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

    is_charging_filepath = os.path.join(mouse_dir, "is_charging")
    with open(is_charging_filepath, 'r') as is_charging_file:
        try:
            is_charging = int(is_charging_file.read().strip())

            if is_charging:
                print("charging")
            else:
                print("not charging")

        except ValueError as ex:
            print("Failed to get charging status.\n{0}".format(ex), file=sys.stderr)
            sys.exit(1)


if __name__ == '__main__':
    run()
