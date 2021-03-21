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


def touch(fname, times=None):
    with open(fname, 'a'):
        os.utime(fname, times)


class FakeDevice(object):
    @staticmethod
    def parse_endpoint_line(line):
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
            chmod = 0o660
        elif chmod == 'w':
            chmod = 0o660
        else:
            chmod = 0o660

        return chmod, name, default, orig_perm

    @staticmethod
    def create_endpoint(path, chmod, default=None):
        if os.path.exists(path):
            os.chmod(path, 0o660)
            os.remove(path)

        if default is not None:
            # Convert to bytes
            if default.startswith("0x"):
                default = bytes.fromhex(default[2:])
            else:
                default = default.encode('UTF-8')

            with open(path, 'wb') as f:
                f.write(default)
        else:
            touch(path)
        os.chmod(path, chmod)

    def __init__(self, spec_name, serial=None, tmp_dir=os.environ.get('TMPDIR', '/tmp')):

        if spec_name not in SPECS:
            raise ValueError("Spec {0} not in SPECS".format(spec_name))

        self._tick = 1
        self.spec_name = spec_name
        self._config = configparser.ConfigParser()
        self._config.read(SPECS[spec_name])
        self._serial = serial

        self._tmp_dir = os.path.join(tmp_dir, self._config.get('device', 'dir_name'))
        os.makedirs(self._tmp_dir, exist_ok=True)

        self.endpoints = {}
        self.events = {}
        self.create_endpoints()
        self.create_events()

        if serial is not None:
            self.set('device_serial', serial)

    def _get_endpoint_path(self, endpoint):
        return os.path.join(self._tmp_dir, endpoint)

    def _get_event_path(self, event):
        return os.path.join(self._tmp_dir, 'input', event)

    def create_events(self):
        """
        Goes through event files and creates them as needed
        """
        event_files = self._config.get('device', 'event', fallback=None)
        if event_files is None:
            event_files = []
        else:
            event_files = event_files.splitlines()

        for index, event_file in enumerate(event_files):
            path = self._get_event_path(event_file)

            os.makedirs(os.path.dirname(path), exist_ok=True)

            if not os.path.exists(path):
                os.mkfifo(path)

            file_object = os.open(path, os.O_RDWR)

            self.events[str(index)] = (event_file, file_object)

    def create_endpoints(self):
        for endpoint_line in self._config.get('device', 'files').splitlines():
            chmod, name, default, orig_perm = self.parse_endpoint_line(endpoint_line)
            path = self._get_endpoint_path(name)

            if name == 'device_serial' and self._serial is not None:
                default = self._serial
            self.endpoints[name] = (chmod, default, orig_perm)
            self.create_endpoint(path, chmod, default)

    def get(self, endpoint, binary=False):
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

        return result

    def set(self, endpoint, value, binary=False):
        if endpoint not in self.endpoints:
            raise ValueError("Endpoint {0} does not exist".format(endpoint))

        path = self._get_endpoint_path(endpoint)

        if binary:
            write_mode = 'wb'
        else:
            write_mode = 'w'

        with open(path, write_mode) as open_endpoint:
            open_endpoint.write(value)

    def emit_kb_event(self, file_id, key_code, value):

        if file_id not in self.events:
            raise ValueError("file_id {0} does not exist".format(file_id))

        if value in KEY_ACTION:
            value = KEY_ACTION[value]
        else:
            value = 0x00

        event_binary = struct.pack(EVENT_FORMAT, self._tick, 0, EV_KEY, key_code, value)
        pipe_fd = self.events[file_id][1]
        os.write(pipe_fd, event_binary)

        return len(event_binary)

    def close(self):
        if os.path.exists(self._tmp_dir):
            # Allow deletion
            for endpoint in self.endpoints:
                path = os.path.join(self._tmp_dir, endpoint)
                os.chmod(path, 0o660)

            shutil.rmtree(self._tmp_dir)


if __name__ == '__main__':

    a = FakeDevice('razertartarus')

    print()

    a.emit_kb_event('0', 62, 'up')

    print()
