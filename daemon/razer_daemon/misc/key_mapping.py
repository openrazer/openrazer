import os
import glob
import logging
import subprocess
import json

KEYMAPS_DIRS = '/usr/share/razer-service/keymaps', '~/.razer-service/keymaps'


class KeymapManager(object):
    def __init__(self, device_number, load_all_keymaps=False):
        self._device_number = device_number
        self._logger = logging.getLogger('razer.device{0}.KeymapManager'.format(device_number))

        # Can have multiple for when I can be bothered to make use of it
        self.keymaps = {}
        self._first_keymap = None

        self._load_keymaps(load_all_keymaps=load_all_keymaps)

    def _get_layouts(self):
        setxkbmap_proc = subprocess.Popen(('setxkbmap', '-query'), stdout=subprocess.PIPE)
        setxkbmap_stdout = setxkbmap_proc.communicate()[0].decode().splitlines()
        for line in setxkbmap_stdout:
            if 'layout' in line:
                return tuple(map(str.strip, line.split(':')[1].split(',')))
        else:
            self._logger.warning("Can't get keymap from setxkbmap, loading gb instead")
            return 'gb',

    def _load_keymaps(self, load_all_keymaps=False):
        current_keymaps = self._get_layouts()

        for keymap_dir in KEYMAPS_DIRS:
            full_path = os.path.join(os.path.expanduser(keymap_dir), '*.json')

            json_files = glob.glob(full_path)

            for file in json_files:
                if not load_all_keymaps and os.path.basename(os.path.splitext(file)[0]) not in allowed_keymaps:
                    continue

                try:
                    keymap = Keymap(file)
                    self.keymaps[keymap.name] = keymap
                    if keymap.name == current_keymaps[0]:
                        self._first_keymap = keymap.name
                    self._logger.warning("Loaded keymap '{0}'".format(file))
                except Exception as err:
                    self._logger.warning("Failed to load keymap '{0}', caught exception {1}".format(file, err))

    def keysym_to_keycode(self, keysym):
        try:
            return self.keymaps[self._first_keymap].keysym_to_keycode(keysym)
        except KeyError:
            raise KeyError("No keymaps found")

    def keycode_to_keysym(self, keycode):
        try:
            return self.keymaps[self._first_keymap].keycode_to_keysym(keycode)
        except KeyError:
            raise KeyError("No keymaps found")


class Keymap(object):
    def __init__(self, file):
        self.name = os.path.basename(os.path.splitext(file)[0])
        self.xkb_rules = None
        self.xkb_model = None
        self.xkb_layout = None
        self.xkb_variant = None

        self.keycodes = {}
        self.keysyms = {}

        data = json.loads(file)

        if data['keymap_metadata']['version'] == 1.0:
            self._parse_v1_0(data)

    def _parse_v1_0(self, data):
        # Convert string key to int
        self.keycodes = {int(key): value for key, value in data['keycodes'].items()}
        self.keysyms = data['keysyms']

        self.xkb_rules = data['metadata']['rules']
        self.xkb_model = data['metadata']['model']
        self.xkb_layout = data['metadata']['layout']
        self.xkb_variant = data['metadata']['variant']

    def keysym_to_keycode(self, keysym):
        try:
            return self.keysyms[keysym]
        except KeyError:
            raise ValueError("keysym not found")

    def keycode_to_keysym(self, keycode):
        try:
            return self.keysyms[keycode]
        except KeyError:
            raise ValueError("keycode not found")




