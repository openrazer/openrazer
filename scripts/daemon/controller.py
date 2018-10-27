#!/usr/bin/python3
import argparse
import openrazer.client

import sys


def _clamp_u8(value):
    if value > 255:
        return 255
    elif value < 0:
        return 0
    else:
        return value


def _print_table(header_list, rows):
    column_lengths = [len(header_item) for header_item in header_list]

    for row in rows:
        for index, column in enumerate(row):
            column = str(column)

            cell_length = len(column)

            try:
                if column_lengths[index] < cell_length:
                    column_lengths[index] = cell_length
            except IndexError:
                pass

    # spaces in between columns + total column length
    max_line_length = ((len(column_lengths) - 1) * 4) + sum(column_lengths)

    # Got maximum column widths

    result = ''
    for index, header_item in enumerate(header_list):
        pad = ' ' * (column_lengths[index] - len(header_item))

        result += '{0}{1}    '.format(header_item, pad)

    # Remove trailing space, add newline
    result += (' ' * (max_line_length - len(result))) + '\n'
    # Add ----- separator and newline
    result += ('-' * max_line_length) + '\n'

    for row in rows:
        line = ''
        for index, column in enumerate(row):
            column = str(column)

            pad = ' ' * (column_lengths[index] - len(column))

            line += '{0}{1}    '.format(column, pad)
        line += (' ' * (max_line_length - len(line))) + '\n'
        result += line

    print(result)


def _get_devices() -> list:
    """
    Gets devices ordered by serial

    :return: List of devices ordered by serial
    :rtype: list
    """
    device_manager = openrazer.client.DeviceManager()

    devices = sorted(device_manager.devices, key=lambda x: (str(x.serial), str(x.type)))

    return devices


def _get_devices_by_serial() -> dict:
    device_manager = openrazer.client.DeviceManager()

    devices = {device.serial: device for device in device_manager.devices}

    return devices


def _get_devices_by_type() -> dict:
    device_manager = openrazer.client.DeviceManager()

    devices = {}

    for device in device_manager.devices:
        dev_type = device.type

        try:
            devices[dev_type].append(device)
        except KeyError:
            devices[dev_type] = [device]

    # Sort devices
    for key in devices:
        devices[key] = sorted(devices[key], key=lambda x: str(x.serial))

    return devices


def _get_device_from_filter(args):
    if args.serial is not None:
        device = _get_devices_by_serial().get(args.serial, None)
    elif '--keyboard' in sys.argv:
        if args.keyboard is None:
            args.keyboard = 0
        try:
            device = _get_devices_by_type().get('keyboard', [])[args.keyboard]
        except IndexError:
            device = None
    elif '--mouse' in sys.argv:
        if args.mouse is None:
            args.mouse = 0
        try:
            device = _get_devices_by_type().get('mouse', [])[args.mouse]
        except IndexError:
            device = None
    elif '--firefly' in sys.argv:
        if args.firefly is None:
            args.firefly = 0
        try:
            device = _get_devices_by_type().get('firefly', [])[args.firefly]
        except IndexError:
            device = None
    elif '--tartarus' in sys.argv:
        if args.tartarus is None:
            args.tartarus = 0
        try:
            device = _get_devices_by_type().get('tartarus', [])[args.tartarus]
        except IndexError:
            device = None
    else:
        # Theoretically impossible to land here
        device = None

    if device is None:
        print("Could not find device")
        sys.exit(1)
    else:
        return device


def list_devices(args):
    """
    Subcommand to list all devices

    :param args: Argparse arguments
    """
    devices = _get_devices()

    headers = ['ID', 'Device Name', 'Device Type', 'Serial']

    rows = []
    for index, device in enumerate(devices):
        rows.append([
            index,
            device.name,
            device.type.title(),
            device.serial
        ])

    _print_table(headers, rows)


def brightness_func(args):
    device = _get_device_from_filter(args)

    if args.set is None:
        # Get brightness
        if args.raw:
            print(str(device.brightness))
        else:
            print("Brightness: {0}%".format(device.brightness))
    else:
        brightness_value = float(_clamp_u8(args.set))

        if not args.raw:
            print("Setting brightness to {0}%".format(brightness_value))

        device.brightness = brightness_value


def parse_args():
    def add_filter_group(sub):
        group = sub.add_mutually_exclusive_group(required=True)
        group.add_argument('--serial', help='Select device via its serial')
        group.add_argument('--keyboard', nargs='?', default=0, type=int, help='Select keyboard, if ID is omitted the first is used')
        group.add_argument('--mouse', nargs='?', default=0, type=int, help='Select mouse, if ID is omitted the first is used')
        group.add_argument('--firefly', nargs='?', default=0, type=int, help='Select Firefly, if ID is omitted the first is used')
        group.add_argument('--tartarus', nargs='?', default=0, type=int, help='Select Tartarus, if ID is omitted the first is used')

    def add_raw(sub):
        sub.add_argument('--raw', action='store_true', help="Raw output")

    parser = argparse.ArgumentParser()
    subparser = parser.add_subparsers(dest='command', help='commands')
    subparser.required = True

    help_parser = subparser.add_parser('help', help='The help command will display help, running "help <command>" will display more detailed help')
    help_parser.add_argument('help', nargs='?', metavar='COMMAND', default=None, type=str)

    # No need to assign to a var as it has no args
    subparser.add_parser('list', help='Lists Razer Devices')

    # Brightness
    brightness_parser = subparser.add_parser('brightness', help='Get or set the brightness')
    add_filter_group(brightness_parser)
    add_raw(brightness_parser)
    brightness_parser.add_argument('--set', metavar='BRIGHTNESS', type=float, default=None, help='Gets brightness if omitted')

    # Macro
    macro_parser = subparser.add_parser('macro', help='Manage macros')
    add_filter_group(macro_parser)
    macro_exclusive_group = macro_parser.add_mutually_exclusive_group(required=True)
    macro_exclusive_group.add_argument('--list', action='store_true', help="List active macros")
    macro_exclusive_group.add_argument('--add-script', nargs=2, type=str, metavar=('BIND_KEY', 'SCRIPT_PATH'), help="Bind the given script to the given macro key. If you require script arguments either create a wrapper or use the API direct.")
    macro_exclusive_group.add_argument('--add-url', nargs=2, type=str, metavar=('BIND_KEY', 'URL'), help="Bind the given URL to the given macro key, so that xdg-open will open a tab.")
    macro_exclusive_group.add_argument('--add-keys', nargs='+', type=str, metavar=('BIND_KEY', 'KEYS'), help="Bind the given key string to the given macro key.")

    args = parser.parse_args()

    if args.command == 'help':
        if args.help == 'brightness':
            brightness_parser.print_help()
        elif args.help == 'macro':
            macro_parser.print_help()
        else:
            parser.print_help()
        sys.exit(0)

    return args


CMD_MAP = {
    'list': list_devices,
    'brightness': brightness_func
}


def run():
    args = parse_args()

    if args.command in CMD_MAP:
        CMD_MAP[args.command](args)
    else:
        print('Someone forgot to add mapping for command "{0}"'.format(args.command))

    print()


if __name__ == '__main__':
    run()
