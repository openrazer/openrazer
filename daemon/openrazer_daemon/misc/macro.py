"""
Macro stuff

Has objects representing key events
Launching programs etc...
"""
import logging
import subprocess
import threading

# pylint: disable=import-error
from openrazer_daemon.keyboard import XTE_MAPPING

# This determines if the macro keys are executed with their natural spacing
XTE_SLEEP = False


class MacroObject(object):
    """
    Macro base object
    """

    def to_dict(self):
        """
        Convert the object to a dict to be sent over DBus

        :return: Dictionary
        :rtype: dict
        """
        raise NotImplementedError()

    @classmethod
    def from_dict(cls, values_dict):
        """
        Create class from dict

        :param values_dict: Dictionary of values (and a type key)
        :type values_dict: dict

        :return: Class object
        """
        del values_dict['type']
        return cls(**values_dict)


class MacroKey(MacroObject):
    """
    Is an object of a key event used in macros
    """

    def __init__(self, key_id, pre_pause, state):
        self.key_id = key_id
        self.pre_pause = pre_pause
        self.state = state

    def __repr__(self):
        return '{0} {1}'.format(self.key_id, self.state)

    def __str__(self):
        return 'MacroKey|{0}|{1}|{2}'.format(self.key_id, self.pre_pause, self.state)

    def to_dict(self):
        return {
            'type': 'MacroKey',
            'key_id': self.key_id,
            'pre_pause': self.pre_pause,
            'state': self.state
        }

    @property
    def xte_key(self):
        """
        Convert key to XTE compatible name

        :return: XTE Name
        :rtype: str
        """
        return XTE_MAPPING.get(self.key_id, self.key_id)

# If it only opens a new tab in chroma - https://askubuntu.com/questions/540939/xdg-open-only-opens-a-new-tab-in-a-new-chromium-window-despite-passing-it-a-url


class MacroURL(MacroObject):
    """
    Is an object of a key event used in macros
    """

    def __init__(self, url):
        self.url = url

    def __repr__(self):
        return '{0}'.format(self.url)

    def __str__(self):
        return 'MacroURL|{0}'.format(self.url)

    def to_dict(self):
        return {
            'type': 'MacroURL',
            'url': self.url,
        }

    def execute(self):
        """
        Open URL in the browser
        """
        proc = subprocess.Popen(['xdg-open', self.url], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        proc.communicate()


class MacroScript(MacroObject):
    """
    Is an object of a key event used in macros
    """

    def __init__(self, script, args=None):
        self.script = script
        if isinstance(args, str):
            self.args = ' ' + args
        else:
            self.args = ''

    def __repr__(self):
        return '{0}'.format(self.script)

    def __str__(self):
        return 'MacroScript|{0}'.format(self.script)

    def to_dict(self):
        return {
            'type': 'MacroScript',
            'script': self.script,
            'args': self.args
        }

    def execute(self):
        """
        Run script
        """
        proc = subprocess.Popen(self.script + self.args, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        proc.communicate()


class MacroRunner(threading.Thread):
    """
    Thread to run macros
    """

    def __init__(self, device_id, macro_bind, macro_data):
        super().__init__()

        self._logger = logging.getLogger('razer.device{0}.macro{1}'.format(device_id, macro_bind))
        self._macro_data = macro_data
        self._macro_bind = macro_bind

    @staticmethod
    def xte_line(key_event):
        """
        Generate a line to be fet into XTE

        :param key_event: Key event object
        :type key_event: MacroKey

        :return: String XTE script
        :rtype: str
        """
        # Save key here to prevent 5odd dictionary lookups
        key = key_event.xte_key

        cmd = ''

        if key is not None:
            if XTE_SLEEP:
                cmd += 'usleep {0}\n'.format(key_event.pre_pause)

            if key_event.state == 'UP':
                cmd += 'keyup {0}\n'.format(key)
            else:
                cmd += 'keydown {0}\n'.format(key)

        return cmd

    def run(self):
        """
        Main thread function
        """

        # TODO move the xte-munging to the init
        xte = ''

        for event in self._macro_data:
            if isinstance(event, MacroKey):
                xte += self.xte_line(event)
            else:
                if xte != '':
                    proc = subprocess.Popen(['xte'], stdin=subprocess.PIPE)
                    proc.communicate(input=xte.encode('ascii'))
                    xte = ''

                # Now run everything else (this just allows for less calls to xte
                if not isinstance(event, MacroKey):
                    event.execute()

        if xte != '':
            proc = subprocess.Popen(['xte'], stdin=subprocess.PIPE)
            proc.communicate(input=xte.encode('ascii'))

        self._logger.debug("Finished running macro %s", self._macro_bind)


def macro_dict_to_obj(macro_dict):
    """
    Converts a macro string to its relevant object

    :param macro_dict: Macro string
    :type macro_dict: dict

    :return: Macro Object
    :rtype: object

    :raises ValueError: When a type isn't known
    """

    if macro_dict['type'] == 'MacroKey':
        result = MacroKey.from_dict(macro_dict)
    elif macro_dict['type'] == 'MacroURL':
        result = MacroURL.from_dict(macro_dict)
    elif macro_dict['type'] == 'MacroScript':
        result = MacroScript.from_dict(macro_dict)
    else:
        raise ValueError("unknown type")

    return result
