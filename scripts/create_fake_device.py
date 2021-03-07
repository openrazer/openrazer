#!/usr/bin/python3
import argparse
import atexit
import cmd
import os
import shutil
import sys
import tempfile
import time

PYLIB = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'pylib')
sys.path.insert(1, PYLIB)

import openrazer._fake_driver as fake_driver


class FakeDevicePrompt(cmd.Cmd):

    def __init__(self, device_map, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self._device_map = device_map
        self._current_device = None

        self._ep = {}
        self._read = []
        self._write = []

        # If only 1 device, auto use that
        if len(self._device_map) == 1:
            self._change_device(list(self._device_map.keys())[0])
        else:
            self._change_device(None)

    def _change_device(self, device_name=None):
        if device_name is not None:
            self._current_device = device_name
            self.prompt = self._current_device + "> "

            for endpoint, details in self._device_map[self._current_device].endpoints.items():
                self._ep[endpoint] = details[2]

            self._read = [endpoint for endpoint, perm in self._ep.items()]
            self._write = [endpoint for endpoint, perm in self._ep.items() if perm in ('w', 'rw')]
        else:
            self._current_device = None
            self.prompt = "> "

    def do_dev(self, arg):
        """
        Change current device
        """
        if arg in self._device_map:
            if arg is None or len(arg) == 0:
                print('Need to specify a device name. One of: {0}'.format(','.join(self._device_map.keys())))
            else:
                self._change_device(arg)
        else:
            print('Invalid device name: {0}'.format(arg))

    def complete_dev(self, text, line, begidx, endidx):
        if not text:
            completions = list(self._device_map.keys())
        else:
            completions = [item for item in list(self._device_map.keys()) if item.startswith(text)]

        return completions

    def do_list(self, arg):
        """List available device files"""
        if self._current_device is not None:
            print('Device files')
            print('------------')
            for endpoint, permission in self._ep.items():
                if permission in ('r', 'rw'):
                    print("  {0:-<2}-  {1}".format(permission, endpoint))
                else:
                    print("  {0:->2}-  {1}".format(permission, endpoint))

            print()
            print('Event files')
            print('-----------')
            for event_id, event_value in sorted(self._device_map[self._current_device].events.items(), key=lambda x: x[0]):
                print("  {0: >2}   {1}".format(event_id, event_value[0]))
        else:
            print('Devices')
            print('-------')
            for device in list(self._device_map.keys()):
                print('  {0}'.format(device))

    def do_ls(self, arg):
        """List available device files"""
        self.do_list(arg)

    def do_read(self, arg, binary=False):
        """Read ASCII from given device file"""
        if self._current_device is not None:
            if arg in self._ep:
                result = self._device_map[self._current_device].get(arg, binary=binary)

                print(result)
            elif arg in self._ep:
                print('Device endpoint not readable')
            else:
                print("Device endpoint not found")

    def do_binary_read(self, arg):
        """Read binary from given device file"""
        self.do_read(arg, binary=True)

    def complete_read(self, text, line, begidx, endidx):
        if not text:
            completions = self._read
        else:
            completions = [item for item in self._read if item.startswith(text)]

        return completions

    complete_binary_read = complete_read

    def do_write(self, arg):
        """Write ASCII to device file. DEVICE_FILE DATA"""
        if self._current_device is not None:
            try:
                device_file, data = arg.split(' ', 1)

                if device_file in self._ep:
                    if len(data) > 0:
                        self._device_map[self._current_device].set(device_file, data)

                        print("{0}: {1}".format(device_file, self._device_map[self._current_device].get(device_file)))
                else:
                    print("Device endpoint not found")

            except ValueError:
                print("Must specify a device endpoint then a space then data to write")

    def complete_write(self, text, line, begidx, endidx):
        if not text:
            completions = self._write
        else:
            completions = [item for item in self._write if item.startswith(text)]

        return completions

    def do_event(self, arg):
        """Emit an event, format: EVENT_ID KEY_ID STATE

        Where state in 'up' 'down' and 'repeat'
        """
        if self._current_device is not None:
            try:
                event_file, key_id, value = arg.split(' ')
            except ValueError:
                print("Usage: event event_file key_id value")
                return

            if event_file not in self._device_map[self._current_device].events:
                print("Event ID {0} is invalid".format(event_file))
            else:
                try:
                    bytes_written = self._device_map[self._current_device].emit_kb_event(event_file, int(key_id), value)
                    print("Wrote {0} bytes to {1}".format(bytes_written, self._device_map[self._current_device].events[event_file][0]))
                except ValueError as err:
                    print("Caught exception: {0}".format(err))

    def do_exit(self, arg):
        """Exit"""
        if self._current_device is not None:
            self._change_device(None)
            return False
        else:
            return True

    def do_EOF(self, arg):
        """Press Ctrl+D to exit"""
        self.do_exit(arg)


def create_envionment(device_name, destination):
    os.makedirs(destination, exist_ok=True)

    try:
        fake_device = fake_driver.FakeDevice(device_name, tmp_dir=destination)
        return fake_device
    except ValueError:
        print('Device {0}.cfg not found'.format(device_name))


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('device', metavar='DEVICE', nargs='*', help='Device config name')
    parser.add_argument('--dest', metavar='DESTDIR', required=False, default=None, help='Directory to create driver files in. If omitted then a tmp directory is used')
    parser.add_argument('--all', action='store_true', help='Create all possible fake devices')
    parser.add_argument('--non-interactive', dest='interactive', action='store_false', help='Dont display prompt, just hang until killed')
    parser.add_argument('--clear-dest', action='store_true', help='Clear the destination folder if it exists before starting')

    return parser.parse_args()


def run():
    args = parse_args()

    if args.dest is None:
        destination = tempfile.mkdtemp(prefix='tmp_', suffix='_fakerazer')
    else:
        destination = args.dest

        if args.clear_dest and os.path.exists(destination):
            shutil.rmtree(destination, ignore_errors=True)

    if args.all:
        devices = fake_driver.SPECS
    else:
        devices = args.device

    device_map = {}
    for device in devices:
        # Device name: FakeDriver
        fake_device = create_envionment(device, destination)
        if fake_device is not None:
            device_map[device] = fake_device

    if len(device_map) == 0:
        print("ERROR: No valid devices passed to script, you either need to pass devices as arguments or use '--all'")
        sys.exit(1)

    # Register cleanup
    if args.dest is None:
        atexit.register(lambda: shutil.rmtree(destination, ignore_errors=True))
    else:
        for device in device_map.values():
            # device = FakeDriver
            atexit.register(device.close)

    print("Device test directory: {0}".format(destination))

    try:
        if not args.interactive:
            print("Sleeping forever, use Ctrl-C to exit...")
            while True:
                time.sleep(99999999)
        else:
            FakeDevicePrompt(device_map).cmdloop()
    except KeyboardInterrupt:
        pass


if __name__ == '__main__':
    run()
