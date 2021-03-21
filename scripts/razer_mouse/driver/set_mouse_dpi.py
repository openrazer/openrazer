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
    parser = argparse.ArgumentParser(description="Set the mouse DPI")
    parser.add_argument('-d', '--device', type=str, help="Device string like \"0003:1532:0045.000C\"")

    parser.add_argument('--dpi_x', required=True, type=int, help="DPI on the X axis (100, 20000)")
    parser.add_argument('--dpi_y', type=int, help="DPI on the Y axis (if omitted, dpi_x is used), (100, 20000)")

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

    dpi_x = clamp_to_min_max(args.dpi_x, 100, 20000)
    if args.dpi_y is None:
        byte_string = struct.pack(">H", dpi_x)
    else:
        dpi_y = clamp_to_min_max(args.dpi_y, 100, 20000)
        byte_string = struct.pack(">HH", dpi_x, dpi_y)

    set_mouse_dpi_filepath = os.path.join(mouse_dir, "dpi")
    with open(set_mouse_dpi_filepath, 'wb') as set_mouse_dpi_file:
        set_mouse_dpi_file.write(byte_string)
    print("Done")


if __name__ == '__main__':
    run()
