"""
Hardware collection
"""

from razer_daemon.hardware.keyboards import RazerBlackWidow2013, RazerBlackWidowChroma, RazerBlackWidowChromaTournamentEdition, RazerBladeStealth, RazerTartarus, RazerBlackWidow2016
from razer_daemon.hardware.mouse_mat import RazerFireFly
from razer_daemon.hardware.mouse import RazerMambaChromaWireless, RazerAbyssus, RazerMambaChromaTE, RazerMambaChromaWired, RazerImperiator, RazerOrochiWired

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


