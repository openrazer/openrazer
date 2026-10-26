# SPDX-License-Identifier: GPL-2.0-or-later

import unittest
import unittest.mock

from openrazer_daemon.dbus_services.dbus_methods import chroma_keyboard
from openrazer_daemon.hardware.device_base import RazerDevice
from openrazer_daemon.misc.ripple_effect import RippleEffectThread, RippleManager
import openrazer_daemon.misc.effect_sync

# msg type = effect, arg = orig_device, arg = effect_name, arg.. = arg..
MSG1 = ('effect', None, 'setBrightness', 255)

# Static effect message from blackwidow chroma
MSG2 = ('effect', None, 'setStatic', 255, 255, 0)
# Static effect message from blackwidow standard
MSG3 = ('effect', None, 'setStatic')
# Breathing effect message from blackwidow chroma
MSG4 = ('effect', None, 'setBreathSingle', 255, 255, 0)
# Pulsate effect message from blackwidow standard
MSG5 = ('effect', None, 'setPulsate')


def logger_mock(*args):
    return unittest.mock.MagicMock()


class DummyHardwareDevice(object):
    def __init__(self):
        self.observer_list = []

        self.disable_notify = None

        self.effect_call = None

    def register_observer(self, obs):
        if obs not in self.observer_list:
            self.observer_list.append(obs)

    def remove_observer(self, obs):
        if obs in self.observer_list:
            self.observer_list.remove(obs)

    # Effect functions
    def setBrightness(self, brightness):
        self.effect_call = ('setBrightness', brightness)

    def setStatic(self):
        raise Exception("test")


class DummyHardwareBlackWidowStandard(DummyHardwareDevice):
    def setStatic(self):
        self.effect_call = ('setStatic',)

    def setPulsate(self):
        self.effect_call = ('setPulsate',)


class DummyHardwareBlackWidowChroma(DummyHardwareDevice):
    def setStatic(self, red, green, blue):
        self.effect_call = ('setStatic', red, green, blue)

    def setBreathSingle(self, red, green, blue):
        self.effect_call = ('setBreathSingle', red, green, blue)


class DummyHardwareRipple(DummyHardwareDevice):
    MATRIX_DIMS = (6, 22)
    setRipple = chroma_keyboard.set_ripple_effect
    setRippleRandomColour = chroma_keyboard.set_ripple_effect_random_colour
    notify = RazerDevice.notify

    def __init__(self):
        super().__init__()
        self.logger = unittest.mock.Mock()
        self.key_manager = unittest.mock.Mock(temp_key_store_state=False)
        self._observer_list = self.observer_list
        self.zone = {'backlight': {'colors': [0, 0, 0]}}
        self.persisted_effect = None

    def send_effect_event(self, effect_name, *args):
        if not self.disable_notify:
            for observer in self.observer_list:
                observer.notify(('effect', self, effect_name, *args))

    def set_persistence(self, zone, setting, value):
        self.persisted_effect = (zone, setting, value)


class EffectSyncTest(unittest.TestCase):
    @unittest.mock.patch('openrazer_daemon.misc.effect_sync.logging.getLogger', logger_mock)
    def setUp(self):
        self.hardware_device = DummyHardwareDevice()
        self.effect_sync = openrazer_daemon.misc.effect_sync.EffectSync(self.hardware_device, 1)

    def test_observers(self):

        self.assertIn(self.effect_sync, self.hardware_device.observer_list)

        # Remove observers
        self.effect_sync.close()

        self.assertEqual(len(self.hardware_device.observer_list), 0)

    def test_get_num_arguments(self):

        def func_2_args(x, y): return x + y

        num_args = self.effect_sync.get_num_arguments(func_2_args)

        self.assertEqual(num_args, 2)

    def test_notify_invalid_message(self):
        self.effect_sync.notify("test")

        self.assertTrue(self.effect_sync._logger.warning.called)

    def test_notify_message_from_other(self):
        # Patch run_effect
        self.effect_sync.run_effect = unittest.mock.MagicMock()

        self.effect_sync.notify(MSG1)

        self.assertTrue(self.effect_sync.run_effect.called)

        new_msg = self.effect_sync.run_effect.call_args_list[0][0]
        # Check function is called run_effect('setBrightness', 255)
        self.assertEqual(new_msg[0], MSG1[2])
        self.assertEqual(new_msg[1], MSG1[3])

    def test_notify_run_effect(self):
        self.effect_sync.notify(MSG1)

        self.assertEqual(self.hardware_device.effect_call[0], MSG1[2])
        self.assertEqual(self.hardware_device.effect_call[1], MSG1[3])

        self.assertIsNotNone(self.hardware_device.disable_notify)
        self.assertFalse(self.hardware_device.disable_notify)

    def test_notify_run_effect_edge_case_1(self):
        # Set the parent to a blackwidow (non chroma)
        self.hardware_device = DummyHardwareBlackWidowStandard()
        self.effect_sync._parent = self.hardware_device
        self.hardware_device.register_observer(self.effect_sync)

        self.effect_sync.notify(MSG2)

        self.assertEqual(self.hardware_device.effect_call[0], MSG2[2])

    def test_notify_run_effect_edge_case_2(self):
        # Set the parent to a blackwidow chroma
        self.hardware_device = DummyHardwareBlackWidowChroma()
        self.effect_sync._parent = self.hardware_device
        self.hardware_device.register_observer(self.effect_sync)

        self.effect_sync.notify(MSG3)

        self.assertTupleEqual(self.hardware_device.effect_call, ('setStatic', 0, 255, 0))

    def test_notify_run_effect_edge_case_3(self):
        # Set the parent to a blackwidow (non chroma)
        self.hardware_device = DummyHardwareBlackWidowStandard()
        self.effect_sync._parent = self.hardware_device
        self.hardware_device.register_observer(self.effect_sync)

        self.effect_sync.notify(MSG4)
        # Send setBreath but converted it to setPulsate
        self.assertEqual(self.hardware_device.effect_call[0], 'setPulsate')

    def test_notify_run_effect_edge_case_4(self):
        # Set the parent to a blackwidow chroma
        self.hardware_device = DummyHardwareBlackWidowChroma()
        self.effect_sync._parent = self.hardware_device
        self.hardware_device.register_observer(self.effect_sync)

        self.effect_sync.notify(MSG5)

        self.assertTupleEqual(self.hardware_device.effect_call, ('setBreathSingle', 0, 255, 0))

    def test_notify_run_effect_edge_case_5(self):
        # Exception of standard setStatic() should be caught
        self.effect_sync.notify(MSG3)

        # Logger should have called .exception
        self.assertTrue(self.effect_sync._logger.exception.called)

    def make_ripple_pair(self):
        source = DummyHardwareRipple()
        target = DummyHardwareRipple()
        self.effect_sync._parent = target
        target.register_observer(self.effect_sync)
        source.register_observer(target)
        return source, target

    def test_random_ripple_sync_preserves_random_effect(self):
        # Run the real observer and worker state transitions without a thread.
        with unittest.mock.patch.object(RippleEffectThread, 'start'), unittest.mock.patch.object(RippleEffectThread, 'join'):
            source, target = self.make_ripple_pair()
            ripple = RippleManager(target, 2)
            try:
                source.setRippleRandomColour(0.01)

                self.assertEqual(target.persisted_effect, ('backlight', 'effect', 'rippleRandomColour'))
                self.assertEqual(target.zone['backlight']['colors'], [0, 0, 0])
                self.assertTrue(target.key_manager.temp_key_store_state)
                self.assertTrue(ripple._ripple_thread.active)
                self.assertIsNone(ripple._ripple_thread._colour)
                self.assertEqual(ripple._ripple_thread._refresh_rate, 0.01)
                self.assertFalse(target.disable_notify)
                self.effect_sync._logger.exception.assert_not_called()
            finally:
                ripple.close()

    def test_colored_ripple_sync_preserves_color(self):
        with unittest.mock.patch.object(RippleEffectThread, 'start'), unittest.mock.patch.object(RippleEffectThread, 'join'):
            source, target = self.make_ripple_pair()
            ripple = RippleManager(target, 2)
            try:
                source.setRipple(255, 0, 255, 0.02)

                self.assertEqual(target.persisted_effect, ('backlight', 'effect', 'ripple'))
                self.assertEqual(target.zone['backlight']['colors'], [255, 0, 255])
                self.assertTrue(target.key_manager.temp_key_store_state)
                self.assertTrue(ripple._ripple_thread.active)
                self.assertEqual(ripple._ripple_thread._colour, (255, 0, 255))
                self.assertEqual(ripple._ripple_thread._refresh_rate, 0.02)
                self.assertFalse(target.disable_notify)
                self.effect_sync._logger.exception.assert_not_called()
            finally:
                ripple.close()
