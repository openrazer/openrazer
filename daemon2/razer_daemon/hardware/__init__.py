"""
Hardware collection
"""

from razer_daemon.hardware.keyboards import RazerBlackWidow2013, RazerBlackWidowChroma
from razer_daemon.hardware.mouse_mat import RazerFireFly
from razer_daemon.hardware.mouse import RazerMambaChroma

def get_device_classes():
    """
    Get a list of hardware classes

    :return: List of RazerDevice subclasses
    :rtype: list of callable
    """
    classes = []

    for class_name, class_object in globals().items():
        if class_name.startswith('__') or class_name in ('RazerDevice', 'DBusService'):
            continue

        if isinstance(class_object, type):
            classes.append(class_object)

    return classes



if __name__ == '__main__':
    # pylint: disable=wrong-import-position,wrong-import-order,invalid-name
    from gi.repository import GObject
    from dbus.mainloop.glib import DBusGMainLoop
    import os

    DBusGMainLoop(set_as_default=True)

    razer_devices = []

    devices = os.listdir('/sys/bus/hid/devices')
    for device_id_link in devices:
        if RazerBlackWidow2013.match(device_id_link):
            print("Found device: {0}".format(device_id_link))
            device_path = os.path.join('/sys/bus/hid/devices', device_id_link)
            razer_device = RazerBlackWidow2013(device_path, 0)
            razer_devices.append(razer_device)

    print("Running main loop")
    GObject.MainLoop().run()

