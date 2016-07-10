"""
Class to manage syncing of effects
"""
import inspect
import logging

class EffectSync(object):
    """
    Class which deals with receiving effect events from other devices
    """
    def __init__(self, parent, device_number):
        self._logger = logging.getLogger('razer.device{0}.effect_sync'.format(device_number))
        self._parent = parent
        self._parent.register_observer(self)

    def __del__(self):
        self.close()

    def close(self):
        self._parent.remove_observer(self)

    def notify(self, msg):
        """
        Receive notificatons from the device (we only care about effects)

        :param msg: Notification
        :type msg: tuple
        """
        if not isinstance(msg, tuple):
            self._logger.warning("Got msg that was not a tuple")
        elif msg[0] == 'effect':
            # We have a message directed at us
            # MSG format
            #  0         1       2             3
            # ('effect', Device, 'effectName', 'effectparams'...)
            # Device is the device the msg originated from (could be parent device)
            if msg[1] is not self._parent:
                # Msg from another device
                self.run_effect(msg[2], *msg[3:])

    def run_effect(self, effect_name, *args):
        """
        Run the specified effect with the given arguments

        :param effect_name: Name of the effect
        :type effect_name: str

        :param args: Arguments for the specified effect
        :type args: list
        """
        # Disable notifications
        self._parent.disable_notify = True

        try:
            # Does parent have method
            effect_func = getattr(self._parent, effect_name, None)
            if effect_func is not None:
                # We have method, does it have the correct num arguments
                actual_args = self.get_num_arguments(effect_func)
                if actual_args == len(args):
                    # method should be same
                    effect_func(*args)
                else:
                    # Method same but wrong args, try alternatives
                    if effect_name == 'setStatic':
                        # Could be static from chroma to non chroma
                        if actual_args == 0:
                            # Chroma -> BW
                            effect_func()
                        else:
                            # BW -> Chroma
                            effect_func(0x0, 0xFF, 0x00) # Green

            else:
                # We dont have method
                if effect_name == 'setPulsate':
                    # BW -> Chroma?
                    effect_func = getattr(self._parent, 'setBreathSingle', None)
                    if effect_func is not None:
                        effect_func(0x00, 0xFF, 0x00) # Green
                if effect_name in ('setBreathSingle', 'setBreathRandom', 'setBreathDual'):
                    # Chroma -> BW?
                    effect_func = getattr(self._parent, 'setPulsate', None)
                    if effect_func is not None:
                        effect_func() # Pulsate

        except Exception as err:
            self._logger.exception("Caught exception trying to sync effects.", exc_info=err)

        # Reenable notifications
        self._parent.disable_notify = False

    @staticmethod
    def get_num_arguments(func):
        """
        Get number of arguments in a function

        :param func: Function
        :type func: callable

        :return: Number of arguments
        :rtype: int
        """
        func_sig = inspect.signature(func)
        return len(func_sig.parameters)
