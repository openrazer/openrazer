#!/usr/bin/python3
import argparse
import glob
import os
import time
import random
import struct


COLOURS = (b'\xFF\x00\x00', b'\x00\xFF\x00', b'\x00\x00\xFF', b'\xFF\xFF\x00', b'\xFF\x00\xFF', b'\x00\xFF\xFF')


def write_binary(driver_path, device_file, payload):
    with open(os.path.join(driver_path, device_file), 'wb') as open_file:
        open_file.write(payload)


def read_string(driver_path, device_file):
    with open(os.path.join(driver_path, device_file), 'r') as open_file:
        return open_file.read().rstrip('\n')


def write_string(driver_path, device_file, payload):
    with open(os.path.join(driver_path, device_file), 'w') as open_file:
        open_file.write(payload)


def find_devices(vid, pid):
    driver_paths = glob.glob(os.path.join('/sys/bus/hid/drivers/razermouse', '*:{0:04X}:{1:04X}.*'.format(vid, pid)))

    for driver_path in driver_paths:
        device_type_path = os.path.join(driver_path, 'device_type')

        if os.path.exists(device_type_path):
            yield driver_path


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--skip-standard', action='store_true')
    parser.add_argument('--skip-custom', action='store_true')
    parser.add_argument('--skip-dpi', action='store_true')
    parser.add_argument('--skip-charge-effect', action='store_true')
    parser.add_argument('--skip-idle-test', action='store_true')

    return parser.parse_args()


if __name__ == '__main__':
    args = parse_args()

    found_chroma = False

    for index, driver_path in enumerate(find_devices(0x1532, 0x0045), start=1):
        found_chroma = True

        print("Mamba Chroma {0}\n".format(index))

        print("Driver version: {0}".format(read_string(driver_path, 'version')))
        print("Driver firmware version: {0}".format(read_string(driver_path, 'firmware_version')))
        print("Device serial: {0}".format(read_string(driver_path, 'device_serial')))
        print("Device type: {0}".format(read_string(driver_path, 'device_type')))
        print("Device mode: {0}".format(read_string(driver_path, 'device_mode')))
        print("Battery level: {0:.2f}%".format(float(read_string(driver_path, 'charge_level')) * 100 / 255))
        print("Charging: {0}".format('yes' if read_string(driver_path, 'charge_status') == '1' else 'no'))

        # Set to static red so that we have something standard
        write_binary(driver_path, 'matrix_effect_static', b'\xFF\x00\x00')

        if not args.skip_standard:
            print("Starting brightness test. Press enter to begin.")
            input()
            print("Max brightness...", end='')
            write_string(driver_path, 'matrix_brightness', '255')
            time.sleep(1)
            print("brightness ({0})".format(read_string(driver_path, 'matrix_brightness')))
            time.sleep(1)
            print("Half brightness...", end='')
            write_string(driver_path, 'matrix_brightness', '128')
            time.sleep(1)
            print("brightness ({0})".format(read_string(driver_path, 'matrix_brightness')))
            time.sleep(1)
            print("Zero brightness...", end='')
            write_string(driver_path, 'matrix_brightness', '0')
            time.sleep(1)
            print("brightness ({0})".format(read_string(driver_path, 'matrix_brightness')))
            time.sleep(1)
            write_string(driver_path, 'matrix_brightness', '255')

            print("Starting other colour effect tests. Press enter to begin.")
            input()
            print("Green Static")
            write_binary(driver_path, 'matrix_effect_static', b'\x00\xFF\x00')
            time.sleep(5)
            print("Cyan Static")
            write_binary(driver_path, 'matrix_effect_static', b'\x00\xFF\xFF')
            time.sleep(5)
            print("Spectrum")
            write_binary(driver_path, 'matrix_effect_spectrum', b'\x00')
            time.sleep(10)
            print("None")
            write_binary(driver_path, 'matrix_effect_none', b'\x00')
            time.sleep(5)
            print("Wave Left")
            write_string(driver_path, 'matrix_effect_wave', '1')
            time.sleep(5)
            print("Wave Right")
            write_string(driver_path, 'matrix_effect_wave', '2')
            time.sleep(5)
            print("Breathing random")
            write_binary(driver_path, 'matrix_effect_breath', b'\x00')
            time.sleep(10)
            print("Breathing red")
            write_binary(driver_path, 'matrix_effect_breath', b'\xFF\x00\x00')
            time.sleep(10)
            print("Breathing blue-green")
            write_binary(driver_path, 'matrix_effect_breath', b'\x00\xFF\x00\x00\x00\xFF')
            time.sleep(10)

        if not args.skip_custom:
            # Custom LEDs all rows
            payload_all = b'\x00\x00\x0E'
            for i in range(0, 15):  # 15 colours 0x00-0x0E
                payload_all += random.choice(COLOURS)

            # Custom LEDs M1-5
            payload_m1_5 = b'\x00\x0E\x0E' + b'\xFF\xFF\xFF'

            print("Custom LED matrix colours test. Press enter to begin.")
            input()
            write_binary(driver_path, 'matrix_custom_frame', payload_all)
            write_binary(driver_path, 'matrix_effect_custom', b'\x00')
            print("Custom LED matrix partial colours test. Setting scroll wheel to white. Press enter to begin.")
            input()
            write_binary(driver_path, 'matrix_custom_frame', payload_m1_5)
            write_binary(driver_path, 'matrix_effect_custom', b'\x00')
            time.sleep(0.5)

        if not args.skip_dpi:
            # DPI test
            print("Starting DPI test. Press enter to begin.")
            input()
            print("Setting X:800 Y:800")
            write_binary(driver_path, 'dpi', struct.pack(">HH", 800, 800))
            time.sleep(10)
            print("Setting X:8000 Y:8000")
            write_binary(driver_path, 'dpi', struct.pack(">HH", 8000, 8000))
            time.sleep(10)
            print("Setting X:100 Y:2000")
            write_binary(driver_path, 'dpi', struct.pack(">HH", 100, 2000))
            time.sleep(10)

            write_binary(driver_path, 'dpi', struct.pack(">H", 800))

        if not args.skip_charge_effect:
            print("Starting charge effect tests. Place the mouse on the dock. Press enter to when ready.")
            input()
            write_string(driver_path, 'matrix_effect_wave', '1')

            print("Setting charge effect to use current mouse effect")
            write_binary(driver_path, 'charge_effect', b'\x00')
            time.sleep(10)
            print("Setting charge effect to be static red. If you take the mouse off charge it should revert to wave effect")

            write_binary(driver_path, 'charge_colour', b'\xFF\x00\x00')  # Also changes dock type
            time.sleep(10)

        if not args.skip_idle_test:
            print("Starting device idle test. This will take roughly 5 minutes. Best to literally move away from the desk, mouse can be rather sensitive. Press enter to start.")
            input()
            print("Setting idle timeout to 60 seconds.")
            write_string(driver_path, 'device_idle_time', '60')
            time.sleep(90)

            print("Mouse")
            write_string(driver_path, 'device_idle_time', '300')

        print("Finished")

    if not found_chroma:
        print("No Mamaba found")
