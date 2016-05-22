import datetime
import logging
import threading
import time
import math


from razer.keyboard import KeyboardColour

class RippleEffectThread(threading.Thread):
    def __init__(self, parent, device_number):
        super(RippleEffectThread, self).__init__()

        self._logger = logging.getLogger('razer.device{0}.ripplethread'.format(device_number))
        self._parent = parent

        self._colour = (255, 0, 255)
        self._refresh_rate = 0.100

        self._shutdown = False
        self._active = False

        self._kerboard_grid = KeyboardColour()

    @property
    def shutdown(self):
        return self._shutdown
    @shutdown.setter
    def shutdown(self, value):
        self._shutdown = value
    @property
    def active(self):
        return self._active
    @property
    def key_list(self):
        return self._parent.key_list

    def enable(self, colour, refresh_rate):
        """
        Enable the ripple effect

        :param colour: Colour tuple like (0, 255, 255)
        :type colour: tuple

        :param refresh_rate: Refresh rate in seconds
        :type refresh_rate: float
        """
        self._colour = colour
        self._refresh_rate = refresh_rate
        self._active = True

    def disable(self):
        """
        Disable the ripple effect
        """
        self._active = False

    def run(self):
        """
        Event loop
        """
        expire_diff = datetime.timedelta(seconds=2)

        # TODO time execution and then sleep for _refresh_rate - time_taken
        while not self._shutdown:
            if self._active:
                # Clear keyboard
                self._kerboard_grid.reset_rows()

                now = datetime.datetime.now()

                radiuses = []

                for expire_time, (key_row, key_col) in self.key_list:
                    event_time = expire_time - expire_diff

                    now_diff = now - event_time

                    # Current radius is based off a time metric
                    radiuses.append((key_row, key_col, now_diff.total_seconds() * 12))

                for row in range(0,7):
                    for col in range(0,22):
                        if row == 0 and col == 20:
                            continue
                        if row == 6:
                            if col != 11:
                                continue
                            else:
                                # To account for logo placement
                                for cy, cx, rad in radiuses:
                                    radius = math.sqrt(math.pow(cy-row, 2) + math.pow(cx-col, 2))
                                    if rad >= radius >= rad-1:
                                        self._kerboard_grid.set_key_colour(0, 20, self._colour)
                                        break
                        else:
                            for cy, cx, rad in radiuses:
                                radius = math.sqrt(math.pow(cy-row, 2) + math.pow(cx-col, 2))
                                if rad >= radius >= rad-1:
                                    self._kerboard_grid.set_key_colour(row, col, self._colour)
                                    break

                payload = self._kerboard_grid.get_total_binary()

                self._parent.set_rgb_matrix(payload)
                self._parent.refresh_keyboard()

            time.sleep(self._refresh_rate)



class RippleManager(object):
    def __init__(self, parent, device_number):
        self._logger = logging.getLogger('razer.device{0}.ripplemanager'.format(device_number))
        self._parent = parent
        self._parent.register_observer(self)

        self._is_closed = False

        self._ripple_thread = RippleEffectThread(self, device_number)
        self._ripple_thread.start()

    @property
    def key_list(self):
        result = []
        if hasattr(self._parent, 'key_manager'):
            result = self._parent.key_manager.temp_key_store

        return result

    def set_rgb_matrix(self, payload):
        self._parent.setKeyRow(payload)

    def refresh_keyboard(self):
        self._parent.setCustom()

    def notify(self, msg):
        if not isinstance(msg, tuple):
            self._logger.warning("Got msg that was not a tuple")
        elif msg[0] == 'effect':
            # We have a message directed at us
            # MSG format
            #  0         1       2             3
            # ('effect', Device, 'effectName', 'effectparams'...)
            # Device is the device the msg originated from (could be parent device)
            if msg[2] == 'setRipple':
                # Get (red, green, blue) tuple (args 3:6), and refreshrate arg 6
                self._parent.key_manager.temp_key_store_state = True
                self._ripple_thread.enable(msg[3:6], msg[6])
            else:
                # Effect other than ripple so stop
                self._ripple_thread.disable()

                self._parent.key_manager.temp_key_store_state = False

    def close(self):
        if not self._is_closed:
            self._logger.debug("Closing Ripple Manager")
            self._is_closed = True

            self._ripple_thread.shutdown = True
            self._ripple_thread.join(timeout=2)
            if self._ripple_thread.is_alive():
                self._logger.error("Could not stop RippleEffect thread")

    def __del__(self):
        self.close()