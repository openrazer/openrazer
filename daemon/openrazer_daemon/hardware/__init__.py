"""
Hardware collection
"""
import os
from openrazer_daemon.hardware.device_base import RazerDevice

# Hack to get a list of hardware modules to import
HARDWARE_MODULES = ['openrazer_daemon.hardware.' + os.path.splitext(hw_file)[0] for hw_file in os.listdir(os.path.dirname(__file__)) if hw_file not in ('device_base.py', '__init__.py') and hw_file.endswith('.py')]

# List of classes to exclude from the class finding
EXCLUDED_CLASSES = ('RazerDevice', 'RazerDeviceBrightnessSuspend')


def get_device_classes():
    """
    Get a list of hardware classes

    :return: List of RazerDevice subclasses
    :rtype: list of callable
    """
    classes = []

    for hw_module in HARDWARE_MODULES:
        imported_module = __import__(hw_module, globals=globals(), locals=locals(), fromlist=['*'], level=0)

        for class_name, class_instance in imported_module.__dict__.items():
            if class_name in EXCLUDED_CLASSES or class_name.startswith('_') or not isinstance(class_instance, type):
                continue

            classes.append(class_instance)

    return sorted(classes, key=lambda cls: cls.__name__)
