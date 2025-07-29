from dataclasses import dataclass
from typing import Callable
from openrazer_daemon.misc.ripple_effect import RippleEffectThread
from openrazer_daemon.misc.complex_reactive_effect import ReactiveEffectThread
from openrazer_daemon.misc.utils import capitalize_first_char

COMPLEX_EFFECT_THREADS = [RippleEffectThread, ReactiveEffectThread]



def template_set_effect(f_name, effect_name):
    def _template(self, red, green, blue, refresh_rate):
        """
        :param red: Red component
        :type red: int

        :param green: Green component
        :type green: int

        :param blue: Blue component
        :type blue: int

        :param refresh_rate: Refresh rate
        :type refresh_rate: int
        """
        self.logger.debug(f"DBus call {f_name}")

        # Notify others
        self.send_effect_event(f_name, red, green, blue, refresh_rate)

        # remember effect
        self.set_persistence("backlight", "effect", effect_name)
        self.zone["backlight"]["colors"][0:3] = int(red), int(green), int(blue)
    return _template

def template_set_effect_random(f_name, effect_name):
    def _template(self, refresh_rate):
        """
        :param refresh_rate: Refresh rate
        :type refresh_rate: int
        """
        self.logger.debug(f"DBus call {f_name}RandomColour")

        # Notify others
        self.send_effect_event(f_name, refresh_rate)

        # remember effect
        self.set_persistence("backlight", "effect", effect_name + 'RandomColour')
    return _template

def template_set_effect_config_file(f_name, effect_name):
    def _template(self, red, green, blue, matrix, refresh_rate):
        """
        :param red: Red component
        :type red: int

        :param green: Green component
        :type green: int

        :param blue: Blue component
        :type blue: int

        :param matrix: Matrix file
        :type matrix: str

        :param refresh_rate: Refresh rate
        :type refresh_rate: int
        """
        self.logger.debug(f"DBus call {f_name}ConfigFile")

        # Notify others
        self.send_effect_event(f_name, red, green, blue, matrix, refresh_rate)

        # remember effect
        self.set_persistence("backlight", "effect", effect_name + 'ConfigFile')
        self.zone["backlight"]["colors"][0:3] = int(red), int(green), int(blue)
    return _template

@dataclass
class ComplexEffectDbusDefinition:
    interface: str
    name: str
    in_sig: str
    function: Callable

COMPLEX_EFFECTS_METHODS = {}
for x in COMPLEX_EFFECT_THREADS:
    canonical_f_name = f_name = f'set{capitalize_first_char(x.EFFECT_NAME)}'
    COMPLEX_EFFECTS_METHODS[f"set_{x.EFFECT_NAME}_effect"] = ComplexEffectDbusDefinition(
        'razer.device.lighting.custom',
        f_name,
        'yyyd',
        template_set_effect(canonical_f_name, x.EFFECT_NAME)
    )

    f_name = f'set{capitalize_first_char(x.EFFECT_NAME)}RandomColor'
    COMPLEX_EFFECTS_METHODS[f"set_{x.EFFECT_NAME}_random_color_effect"] = ComplexEffectDbusDefinition(
        'razer.device.lighting.custom',
        f_name,
        'd',
        template_set_effect_random(canonical_f_name, x.EFFECT_NAME)
    )
    f_name = f'set{capitalize_first_char(x.EFFECT_NAME)}ConfigFile'
    COMPLEX_EFFECTS_METHODS[f"set_{x.EFFECT_NAME}_config_file_effect"] = ComplexEffectDbusDefinition(
        'razer.device.lighting.custom',
        f_name,
        'yyysd',
        template_set_effect_config_file(canonical_f_name, x.EFFECT_NAME)
    )
