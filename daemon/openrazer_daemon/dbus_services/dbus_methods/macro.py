"""
Macro recording methods, placed in a separate file because they don't use the binding_manager
"""

from openrazer_daemon.dbus_services import endpoint
# pylint: disable=protected-access,too-many-arguments


@endpoint('razer.device.macro', 'startMacroRecording', in_sig='ssy')
def start_macro_recording(self, profile, mapping, key_code):
    """
    Starts recording recording key actions to the given key

    :param profile: The profile name
    :type: str

    :param mapping: The map name
    :type: str

    :param key_code: The key code
    :type: int
    """
    self.logger.debug("DBus call start_macro_recording")

    self.key_manager.macro_mode = True
    self.key_manager._macro_profile = profile
    self.key_manager._macro_map = mapping
    self.key_manager.macro_key = key_code


@endpoint('razer.device.macro', 'stopMacroRecording')
def stop_macro_recording(self):
    """
    Stops macro recording
    """
    self.logger.debug("DBus call stop_macro_recording")

    self.key_manager.macro_mode = False


@endpoint('razer.device.macro', 'getMacroRecordingState', out_sig='b')
def get_macro_recording_state(self):
    """
    Returns the state of the macro recorder

    :return: The recording state
    :rtype: bool
    """
    self.logger.debug("DBus call get_macro_recording_state")

    return self.key_manager.macro_mode


@endpoint('razer.device.macro', 'getMacroKey', out_sig='y')
def get_macro_key(self):
    """
    Returns the key being recorded to or None.

    :return: The macro key
    :rtype: int
    """
    self.logger.debug("DBus call get_macro_key")

    return self.key_manager.macro_key
