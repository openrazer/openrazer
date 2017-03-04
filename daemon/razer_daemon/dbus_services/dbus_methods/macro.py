"""
BlackWidow Macro accessors
"""
from razer_daemon.dbus_services import endpoint


@endpoint('razer.device.macro', 'getMacros', out_sig='s')
def get_macros(self):
    """
    Get macros

    :return: JSON of macros
    :rtype: str
    """
    self.logger.debug("DBus call get_macros")

    return self.macro_service.dbus_get_macros()


@endpoint('razer.device.macro', 'deleteMacro', in_sig='y')
def delete_macro(self, macro_key):
    """
    Delete macro from key

    :param macro_key: Macro key to delete bound macro from
    :type macro_key: int
    """
    self.logger.debug("DBus call delete_macro")

    self.macro_service.dbus_delete_macro(macro_key)


@endpoint('razer.device.macro', 'addMacro', in_sig='ys')
def add_macro(self, macro_bind_key, macro_json):
    """
    Add macro to key

    The macro_json should be JSON form of a list of dictionaries
    :param macro_bind_key: Macro key to delete bound macro from
    :type macro_bind_key: int

    :param macro_json: JSON list
    :type macro_json: str
    """
    self.logger.debug("DBus call add_macro")

    self.macro_service.dbus_add_macro(macro_bind_key, macro_json)
