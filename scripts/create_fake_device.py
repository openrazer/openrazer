#!/usr/bin/env python3
import argparse
import atexit
import cmd
import os
import shutil
import sys
import tempfile

PYLIB = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'pylib')
sys.path.insert(1, PYLIB)

import razer._fake_driver as fake_driver


class FakeDevicePrompt(cmd.Cmd):

    def __init__(self, device, *args, **kwargs):
        super(FakeDevicePrompt, self).__init__(*args, **kwargs)

        self._device = device

        self._ep = {}
        for endpoint, details in self._device.endpoints.items():
            self._ep[endpoint] = details[2]

        self._read = [endpoint for endpoint, perm in self._ep.items() if perm in ('r', 'rw')]
        self._write = [endpoint for endpoint, perm in self._ep.items() if perm in ('w', 'rw')]

        self.prompt = self._device.spec_name + "> "

    def do_list(self, arg):
        """List available device files"""
        print('Device files')
        print('------------')
        for endpoint, permission in self._ep.items():
            if permission in ('r', 'rw'):
                print("  {0:-<2}-  {1}".format(permission, endpoint))
            else:
                print("  {0:->2}-  {1}".format(permission, endpoint))

    def do_ls(self, arg):
        """List available device files"""
        self.do_list(arg)

    def do_read(self, arg, binary=False):
        """Read ASCII from given device file"""
        if arg in self._ep and self._ep[arg] in ('r', 'rw'):
            result = self._device.get(arg, binary=binary)

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
        try:
            device_file, data = arg.split(' ', 1)

            if device_file in self._ep and self._ep[device_file] in ('w', 'rw'):
                if len(data) > 0:
                    self._device.set(device_file, data)

                    print("{0}: {1}".format(device_file, self._device.get(device_file)))
            elif device_file in self._ep:
                print('Device endpoint not writable')
            else:
                print("Device endpoint not found")

        except ValueError:
            print("Must specify a device enpoint then a space then data to write")

    def complete_write(self, text, line, begidx, endidx):
        if not text:
            completions = self._write
        else:
            completions = [item for item in self._write if item.startswith(text)]

        return completions



    def do_exit(self, arg):
        """Exit"""
        return True

    def do_EOF(self, arg):
        """Press Ctrl+D to exit"""
        return True


def create_envionment(device_name, destination):
    os.makedirs(destination, exist_ok=True)

    try:
        fake_device = fake_driver.FakeDevice(device_name, tmp_dir=destination)
        return fake_device
    except ValueError:
        print('Device {0}.cfg not found'.format(device_name))

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('device', metavar='DEVICE', help='Device config name')
    parser.add_argument('--dest', metavar='DESTDIR', required=False, default=None, help='Directory to create driver files in. If omitted then a tmp directory is used')
    parser.add_argument('--non-interactive', dest='interactive', action='store_false', help='Dont display prompt, just hang until killed')

    return parser.parse_args()

def run():
    args = parse_args()

    if args.dest is None:
        destination = tempfile.mkdtemp(prefix='tmp_', suffix='_fakerazer')
    else:
        destination = args.dest

    device = create_envionment(args.device, destination)

    # Register cleanup
    if args.dest is None:
        atexit.register(lambda: shutil.rmtree(destination, ignore_errors=True))
    else:
        atexit.register(device.close)

    print("Device test directory: {0}".format(destination))

    try:
        if not args.interactive:
            input()
        else:
            FakeDevicePrompt(device).cmdloop()
    except KeyboardInterrupt:
        pass











if __name__ == '__main__':
    run()