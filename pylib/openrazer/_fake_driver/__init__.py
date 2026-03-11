# SPDX-License-Identifier: GPL-2.0-or-later

import struct
import configparser
import glob
import os
import shutil

SPECS = {os.path.splitext(os.path.basename(spec_file))[0]: spec_file for spec_file in glob.glob(os.path.join(os.path.dirname(__file__), '*.cfg'))}
EVENT_FORMAT = '@llHHI'
EV_KEY = 0x01

KEY_ACTION = {
    'up': 0x00,
    'down': 0x01,
    'repeat': 0x02
}


def touch(fname: str) -> None:
    with open(fname, 'a'):
        os.utime(fname)


class FakeDevice(object):
    @staticmethod
    def parse_endpoint_line(line: str) -> tuple[int, str, str | None, str]:
        components = line.split(',')

        if len(components) == 2:
            chmod, name = components
            default = None
        elif len(components) == 3:
            chmod, name, default = components
        else:
            raise ValueError('Invalid config line "{0}"'.format(line))

        orig_perm = chmod

        if chmod == 'r':
            chmod_int = 0o660
        elif chmod == 'w':
            chmod_int = 0o660
        else:
            chmod_int = 0o660

        return chmod_int, name, default, orig_perm

    @staticmethod
    def create_endpoint(path: str, chmod: int, default: str | None = None) -> None:
        if os.path.exists(path):
            os.chmod(path, 0o660)
            os.remove(path)

        if default is not None:
            # Convert to bytes
            if default.startswith("0x"):
                default_bin = bytes.fromhex(default[2:])
            else:
                default_bin = default.encode('UTF-8')

            with open(path, 'wb') as f:
                f.write(default_bin)
        else:
            touch(path)
        os.chmod(path, chmod)

    def __init__(self, spec_name: str, serial: str | None = None, tmp_dir: str=os.environ.get('TMPDIR', '/tmp')):

        if spec_name not in SPECS:
            raise ValueError("Spec {0} not in SPECS".format(spec_name))

        self._tick = 1
        self.spec_name = spec_name
        self._config = configparser.ConfigParser()
        self._config.read(SPECS[spec_name])
        self._serial = serial

        self._tmp_dir = os.path.join(tmp_dir, self._config.get('device', 'dir_name'))
        os.makedirs(self._tmp_dir, exist_ok=True)

        self.endpoints: dict[str, tuple[int, str | None, str]] = {}
        self.events: dict[str, tuple[str, int]] = {}
        self.create_endpoints()
        self.create_events()

        if serial is not None:
            self.set('device_serial', serial)

        # Disallow write in directory, so disallow creating new files after setup is done.
        os.chmod(self._tmp_dir, 0o555)

    def _get_endpoint_path(self, endpoint: str) -> str:
        return os.path.join(self._tmp_dir, endpoint)

    def _get_event_path(self, event: str) -> str:
        return os.path.join(self._tmp_dir, 'input', event)

    def create_events(self) -> None:
        """
        Goes through event files and creates them as needed
        """
        event_files: str | None = self._config.get('device', 'event', fallback=None)
        if event_files is None:
            event_files_list = []
        else:
            event_files_list = event_files.splitlines()

        for index, event_file in enumerate(event_files_list):
            path = self._get_event_path(event_file)

            os.makedirs(os.path.dirname(path), exist_ok=True)

            if not os.path.exists(path):
                os.mkfifo(path)

            file_object = os.open(path, os.O_RDWR)

            self.events[str(index)] = (event_file, file_object)

    def create_endpoints(self) -> None:
        for endpoint_line in self._config.get('device', 'files').splitlines():
            chmod, name, default, orig_perm = self.parse_endpoint_line(endpoint_line)
            path = self._get_endpoint_path(name)

            if name == 'device_serial' and self._serial is not None:
                default = self._serial
            self.endpoints[name] = (chmod, default, orig_perm)
            self.create_endpoint(path, chmod, default)

    def get(self, endpoint: str, binary: bool=False) -> str | bytes:
        """
        Gets a value from a given endpoint

        :param endpoint: Endpoint to read from
        :type endpoint: str

        :param binary: Is binary data being read
        :type binary: bool

        :return: Result
        :rtype: str or bytes
        """
        if endpoint not in self.endpoints:
            raise ValueError("Endpoint {0} does not exist".format(endpoint))

        path = self._get_endpoint_path(endpoint)

        if binary:
            read_mode = 'rb'
        else:
            read_mode = 'r'

        try:
            with open(path, read_mode) as open_endpoint:
                result = open_endpoint.read()
        except UnicodeDecodeError as e:
            return str(e)

        if not isinstance(result, bytes) and not isinstance(result, str):
            raise ValueError(f"Got invalid type in result: {type(result)}")

        return result

    def set(self, endpoint: str, value: str, binary: bool=False) -> None:
        if endpoint not in self.endpoints:
            raise ValueError("Endpoint {0} does not exist".format(endpoint))

        path = self._get_endpoint_path(endpoint)

        if binary:
            write_mode = 'wb'
        else:
            write_mode = 'w'

        with open(path, write_mode) as open_endpoint:
            open_endpoint.write(value)

    def emit_kb_event(self, file_id: str, key_code: int, value: str) -> int:

        if file_id not in self.events:
            raise ValueError("file_id {0} does not exist".format(file_id))

        if value in KEY_ACTION:
            value_int = KEY_ACTION[value]
        else:
            value_int = 0x00

        event_binary = struct.pack(EVENT_FORMAT, self._tick, 0, EV_KEY, key_code, value_int)
        pipe_fd = self.events[file_id][1]
        os.write(pipe_fd, event_binary)

        return len(event_binary)

    def close(self) -> None:
        if os.path.exists(self._tmp_dir):
            # Allow deletion
            for endpoint in self.endpoints:
                path = os.path.join(self._tmp_dir, endpoint)
                os.chmod(path, 0o660)

            # Allow write in directory again for cleanup
            os.chmod(self._tmp_dir, 0o755)

            shutil.rmtree(self._tmp_dir)


if __name__ == '__main__':

    a = FakeDevice('razertartarus')

    print()

    a.emit_kb_event('0', 62, 'up')

    print()
