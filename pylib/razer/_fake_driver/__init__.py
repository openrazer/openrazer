import configparser
import glob
import os
import shutil

SPECS = {os.path.splitext(os.path.basename(spec_file))[0]: spec_file for spec_file in glob.glob(os.path.join(os.path.dirname(__file__), '*.cfg'))}


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
            open(path, 'w').write(str(default))
        else:
            touch(path)
        os.chmod(path, chmod)

    def __init__(self, spec_name, serial=None, tmp_dir='/tmp'):

        if spec_name not in SPECS:
            raise ValueError("Spec {0} not in SPECS".format(spec_name))

        self.spec_name = spec_name
        self._config = configparser.ConfigParser()
        self._config.read(SPECS[spec_name])
        self._serial = serial

        self._tmp_dir = os.path.join(tmp_dir, self._config.get('device', 'dir_name'))
        os.makedirs(self._tmp_dir, exist_ok=True)

        self.endpoints = {}
        self.create_endpoints()

        if serial is not None:
            self.set('get_serial', serial)

    def _get_endpoint_path(self, endpoint):
        return os.path.join(self._tmp_dir, endpoint)

    def create_endpoints(self):
        for endpoint_line in self._config.get('device', 'files').splitlines():
            chmod, name, default, orig_perm = self.parse_endpoint_line(endpoint_line)
            path = self._get_endpoint_path(name)

            if name == 'get_serial' and self._serial is not None:
                default = self._serial
            self.endpoints[name] = (chmod, default, orig_perm)
            self.create_endpoint(path, chmod, default)

    def get(self, endpoint, binary=False):
        if endpoint not in self.endpoints:
            raise ValueError("Endpoint {0} does not exist".format(endpoint))

        path = self._get_endpoint_path(endpoint)

        if binary:
            read_mode = 'rb'
        else:
            read_mode = 'r'

        with open(path, read_mode) as open_endpoint:
            result = open_endpoint.read()

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


    def __del__(self):
        self.close()

    def close(self):
        if os.path.exists(self._tmp_dir):
            # Allow deletion
            for endpoint in self.endpoints:
                path = os.path.join(self._tmp_dir, endpoint)
                os.chmod(path, 0o660)

            shutil.rmtree(self._tmp_dir)

