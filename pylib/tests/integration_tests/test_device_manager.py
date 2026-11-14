#!/usr/bin/python3
# SPDX-License-Identifier: GPL-2.0-or-later

import multiprocessing
import os
import signal
import tempfile
import time
import traceback
import unittest
from unittest.mock import Mock

import dbus
import openrazer.client
import openrazer_daemon.daemon
import openrazer._fake_driver as fake_driver


class IsolatedKeyboardDaemon(openrazer_daemon.daemon.RazerDaemon):
    """Serve fake devices without monitoring the user's hardware or desktop."""

    def _check_plugdev_group(self):
        return True

    def _init_udev_monitor(self):
        self._udev_observer = Mock()

    def _init_screensaver_monitor(self):
        self._screensaver_monitor = Mock(monitoring=False)

    def _init_autosave_persistence(self):
        pass

    def _init_dock_mouse_monitor(self):
        self._dock_mouse_pending = {}


def run_daemon(driver_dir, log_dir, ready):
    try:
        daemon = IsolatedKeyboardDaemon(test_dir=driver_dir, log_dir=log_dir)
    except Exception:
        ready.send(traceback.format_exc())
        ready.close()
        raise
    ready.send(None)
    ready.close()
    daemon.run()


class DeviceManagerTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if dbus.SessionBus().name_has_owner('org.razer'):
            raise unittest.SkipTest('An OpenRazer daemon owns this bus; use dbus-run-session')
        cls._tmp_dir = tempfile.TemporaryDirectory(prefix='tmp_', suffix='_daemontest')
        cls.addClassCleanup(cls._tmp_dir.cleanup)
        driver_dir = os.path.join(cls._tmp_dir.name, 'driver')
        log_dir = os.path.join(cls._tmp_dir.name, 'logs')
        cls._log_file = os.path.join(log_dir, 'razer.log')

        cls._bw_serial = 'IO0000000000001'
        cls._bw_chroma = fake_driver.FakeDevice('razerblackwidowchroma', serial=cls._bw_serial, tmp_dir=driver_dir)
        cls.addClassCleanup(cls._bw_chroma.close)
        cls._wheel_serial = 'IO0000000000002'
        cls._bw_v4x = fake_driver.FakeDevice('razerblackwidowv4x', serial=cls._wheel_serial, tmp_dir=driver_dir)
        cls.addClassCleanup(cls._bw_v4x.close)

        context = multiprocessing.get_context('spawn')
        ready, child_ready = context.Pipe(duplex=False)
        cls.addClassCleanup(ready.close)
        cls._daemon_proc = context.Process(target=run_daemon, args=(driver_dir, log_dir, child_ready))
        cls._daemon_proc.start()
        child_ready.close()
        cls.addClassCleanup(cls.stop_daemon)
        if not ready.poll(10):
            raise RuntimeError('Fake keyboard daemon did not become ready')
        failure = ready.recv()
        if failure is not None:
            raise RuntimeError(failure)
        # Per-test endpoint resets clear device_mode, so capture startup here.
        cls._startup_device_mode = cls._bw_chroma.get('device_mode', binary=True)

    @classmethod
    def stop_daemon(cls):
        if cls._daemon_proc.is_alive():
            os.kill(cls._daemon_proc.pid, signal.SIGINT)
            cls._daemon_proc.join(5)
        if cls._daemon_proc.is_alive():
            cls._daemon_proc.terminate()
            cls._daemon_proc.join(5)
        if cls._daemon_proc.is_alive():
            cls._daemon_proc.kill()
            cls._daemon_proc.join(5)
        exit_code = cls._daemon_proc.exitcode
        cls._daemon_proc.close()
        if exit_code != 0:
            raise AssertionError(f'Fake keyboard daemon exited with status {exit_code}')
        cls.check_daemon_log()

    @classmethod
    def check_daemon_log(cls):
        with open(cls._log_file) as log:
            contents = log.read()
        if any(marker in contents for marker in ('Traceback (most recent call last):', '| ERROR ', '| CRITICAL ')):
            raise AssertionError(contents)

    def setUp(self):
        self._bw_chroma.create_endpoints()
        self._bw_v4x.create_endpoints()

        self.device_manager = openrazer.client.DeviceManager()
        self.device = next(device for device in self.device_manager.devices if device.serial == self._bw_serial)
        self.wheel_device = next(device for device in self.device_manager.devices if device.serial == self._wheel_serial)

    def tearDown(self):
        self.check_daemon_log()

    def test_device_list(self):
        self.assertCountEqual([device.serial for device in self.device_manager.devices], [self._bw_serial, self._wheel_serial])

    def test_serial(self):
        device = self.device

        self.assertEqual(device.serial, self._bw_chroma.get('device_serial'))

    def test_name(self):
        device = self.device

        self.assertEqual(device.name, self._bw_chroma.get('device_type'))

    def test_type(self):
        device = self.device

        self.assertEqual(device.type, 'keyboard')

    def test_fw_version(self):
        device = self.device

        self.assertEqual(device.firmware_version, self._bw_chroma.get('firmware_version'))

    def test_brightness(self):
        device = self.device

        # Test 100%
        device.brightness = 100.0

        self.assertEqual('255', self._bw_chroma.get('matrix_brightness'))

        self.assertEqual(100.0, device.brightness)

        device.brightness = 50.0

        self.assertEqual('127', self._bw_chroma.get('matrix_brightness'))

        self.assertAlmostEqual(50.0, device.brightness, delta=0.4)

        device.brightness = 0.0

        self.assertEqual('0', self._bw_chroma.get('matrix_brightness'))

        self.assertEqual(0, device.brightness)

    def test_capabilities(self):
        device = self.device

        self.assertEqual(device.capabilities, device._capabilities)

    def test_device_keyboard_game_mode(self):
        device = self.device

        self._bw_chroma.set('game_led_state', '1')
        self.assertTrue(device.game_mode_led)
        device.game_mode_led = False
        self.assertEqual(self._bw_chroma.get('game_led_state'), '0')
        device.game_mode_led = True
        self.assertEqual(self._bw_chroma.get('game_led_state'), '1')

    def test_device_keyboard_macro_mode(self):
        device = self.device

        self._bw_chroma.set('macro_led_state', '1')
        self.assertTrue(device.macro_mode_led)
        device.macro_mode_led = False
        self.assertEqual(self._bw_chroma.get('macro_led_state'), '0')
        device.macro_mode_led = True
        self.assertEqual(self._bw_chroma.get('macro_led_state'), '1')

        self._bw_chroma.set('macro_led_effect', '0')
        self.assertEqual(device.macro_mode_led_effect, openrazer.client.constants.MACRO_LED_STATIC)
        device.macro_mode_led_effect = openrazer.client.constants.MACRO_LED_BLINK
        self.assertEqual(self._bw_chroma.get('macro_led_effect'), str(openrazer.client.constants.MACRO_LED_BLINK))

    def test_device_keyboard_effect_none(self):
        device = self.device

        device.fx.none()

        self.assertEqual(self._bw_chroma.get('matrix_effect_none'), '1')

    def test_device_keyboard_effect_spectrum(self):
        device = self.device

        device.fx.spectrum()

        self.assertEqual(self._bw_chroma.get('matrix_effect_spectrum'), '1')

    def test_device_keyboard_effect_wave(self):
        device = self.device

        device.fx.wave(openrazer.client.constants.WAVE_LEFT)
        self.assertEqual(self._bw_chroma.get('matrix_effect_wave'), str(openrazer.client.constants.WAVE_LEFT))
        device.fx.wave(openrazer.client.constants.WAVE_RIGHT)
        self.assertEqual(self._bw_chroma.get('matrix_effect_wave'), str(openrazer.client.constants.WAVE_RIGHT))

        with self.assertRaises(ValueError):
            device.fx.wave('lalala')

    def test_device_keyboard_effect_wheel(self):
        device = self.wheel_device

        self.assertTrue(device.fx.has('wheel'))
        device.fx.wheel(openrazer.client.constants.WHEEL_LEFT)
        self.assertEqual(self._bw_v4x.get('matrix_effect_wheel'), str(openrazer.client.constants.WHEEL_LEFT))
        device.fx.wheel(openrazer.client.constants.WHEEL_RIGHT)
        self.assertEqual(self._bw_v4x.get('matrix_effect_wheel'), str(openrazer.client.constants.WHEEL_RIGHT))

        with self.assertRaises(ValueError):
            device.fx.wheel('lalala')

        self.assertFalse(self.device.fx.has('wheel'))
        for direction in (openrazer.client.constants.WHEEL_LEFT, openrazer.client.constants.WHEEL_RIGHT):
            with self.subTest(direction=direction), self.assertRaises(NotImplementedError):
                self.device.fx.wheel(direction)

    def test_device_keyboard_effect_static(self):
        device = self.device

        device.fx.static(255, 0, 255)
        self.assertEqual(b'\xFF\x00\xFF', self._bw_chroma.get('matrix_effect_static', binary=True))

        for red, green, blue in ((256.0, 0, 0), (0, 256.0, 0), (0, 0, 256.0)):
            with self.assertRaises(ValueError):
                device.fx.static(red, green, blue)

        device.fx.static(256, 0, 700)
        self.assertEqual(b'\xFF\x00\xFF', self._bw_chroma.get('matrix_effect_static', binary=True))

    def test_device_keyboard_effect_reactive(self):
        device = self.device

        time = openrazer.client.constants.REACTIVE_500MS
        device.fx.reactive(255, 0, 255, time)
        self.assertEqual(b'\x01\xFF\x00\xFF', self._bw_chroma.get('matrix_effect_reactive', binary=True))

        for red, green, blue in ((256.0, 0, 0), (0, 256.0, 0), (0, 0, 256.0)):
            with self.assertRaises(ValueError):
                device.fx.reactive(red, green, blue, time)

        device.fx.reactive(256, 0, 700, time)
        self.assertEqual(b'\x01\xFF\x00\xFF', self._bw_chroma.get('matrix_effect_reactive', binary=True))

        with self.assertRaises(ValueError):
            device.fx.reactive(255, 0, 255, 'lalala')

    def test_device_keyboard_effect_breath_single(self):
        device = self.device

        device.fx.breath_single(255, 0, 255)
        self.assertEqual(b'\xFF\x00\xFF', self._bw_chroma.get('matrix_effect_breath', binary=True))

        for red, green, blue in ((256.0, 0, 0), (0, 256.0, 0), (0, 0, 256.0)):
            with self.assertRaises(ValueError):
                device.fx.breath_single(red, green, blue)

        device.fx.breath_single(256, 0, 700)
        self.assertEqual(b'\xFF\x00\xFF', self._bw_chroma.get('matrix_effect_breath', binary=True))

    def test_device_keyboard_effect_breath_dual(self):
        device = self.device

        device.fx.breath_dual(255, 0, 255, 255, 0, 0)
        self.assertEqual(b'\xFF\x00\xFF\xFF\x00\x00', self._bw_chroma.get('matrix_effect_breath', binary=True))

        for r1, g1, b1, r2, g2, b2 in ((256.0, 0, 0, 0, 0, 0), (0, 256.0, 0, 0, 0, 0), (0, 0, 256.0, 0, 0, 0),
                                       (0, 0, 0, 256.0, 0, 0), (0, 0, 0, 0, 256.0, 0), (0, 0, 0, 0, 0, 256.0)):
            with self.assertRaises(ValueError):
                device.fx.breath_dual(r1, g1, b1, r2, g2, b2)

        device.fx.breath_dual(256, 0, 700, 255, 0, 0)
        self.assertEqual(b'\xFF\x00\xFF\xFF\x00\x00', self._bw_chroma.get('matrix_effect_breath', binary=True))

    def test_device_keyboard_effect_breath_random(self):
        device = self.device

        device.fx.breath_random()

        self.assertEqual(self._bw_chroma.get('matrix_effect_breath'), '1')

    def test_device_keyboard_effect_ripple(self):
        device = self.device

        refresh_rate = 0.01
        device.fx.ripple(255, 0, 255, refresh_rate)
        self.addCleanup(device.fx.none)
        time.sleep(0.1)

        custom_effect_payload = self._bw_chroma.get('matrix_custom_frame', binary=True)
        self.assertGreater(len(custom_effect_payload), 1)
        self.assertEqual(self._bw_chroma.get('matrix_effect_custom'), '1')

        for red, green, blue in ((256.0, 0, 0), (0, 256.0, 0), (0, 0, 256.0)):
            with self.assertRaises(ValueError):
                device.fx.ripple(red, green, blue, refresh_rate)

        with self.assertRaises(ValueError):
            device.fx.ripple(255, 0, 255, 'lalala')

        device.fx.none()

    def test_device_keyboard_effect_random_ripple(self):
        device = self.device

        refresh_rate = 0.01
        device.fx.ripple_random(refresh_rate)
        self.addCleanup(device.fx.none)
        time.sleep(0.1)

        custom_effect_payload = self._bw_chroma.get('matrix_custom_frame', binary=True)
        self.assertGreater(len(custom_effect_payload), 1)
        self.assertEqual(self._bw_chroma.get('matrix_effect_custom'), '1')

        with self.assertRaises(ValueError):
            device.fx.ripple_random('lalala')

        device.fx.none()

    def test_device_keyboard_effect_framebuffer(self):
        device = self.device

        def expected_frame(first_pixel):
            # BlackWidow Chroma: six rows, 22 RGB pixels per row, with
            # row/start/end headers before each row's pixel data.
            return b''.join(bytes((row, 0, 21)) + (first_pixel if row == 0 else bytes(3)) + bytes(63)
                            for row in range(6))

        device.fx.advanced.matrix.set(0, 0, (255, 0, 255))

        self.assertEqual(device.fx.advanced.matrix.get(0, 0), (255, 0, 255))

        device.fx.advanced.draw()
        custom_effect_payload = self._bw_chroma.get('matrix_custom_frame', binary=True)
        self.assertEqual(custom_effect_payload, expected_frame(b'\xFF\x00\xFF'))

        device.fx.advanced.matrix.to_framebuffer()  # Save 255, 0, 255
        device.fx.advanced.matrix.reset()  # Clear FB

        device.fx.advanced.matrix.set(0, 0, (0, 255, 0))

        device.fx.advanced.draw_fb_or()  # Draw FB or'd with Matrix
        custom_effect_payload = self._bw_chroma.get('matrix_custom_frame', binary=True)
        self.assertEqual(custom_effect_payload, expected_frame(b'\xFF\xFF\xFF'))

        # Append that to FB
        device.fx.advanced.matrix.to_framebuffer_or()
        device.fx.advanced.draw()
        custom_effect_payload = self._bw_chroma.get('matrix_custom_frame', binary=True)

        binary = device.fx.advanced.matrix.to_binary()

        self.assertEqual(binary, custom_effect_payload)

    def test_device_keyboard_driver_mode(self):
        # Macro keys are enabled by the daemon entering driver mode at startup.
        self.assertEqual(self._startup_device_mode, b'\x03\x00')

    def test_device_keyboard_macro_add(self):
        device = self.device

        url_macro = device.macro.create_url_macro_item('http://example.org')
        device.macro.add_macro('M1', [url_macro])
        self.addCleanup(device.macro.del_macro, 'M1')

        macros = device.macro.get_macros()
        self.assertIn('M1', macros)

        # M6 is a valid key, and the client also accepts a single macro object.
        device.macro.add_macro('M6', url_macro)
        self.addCleanup(device.macro.del_macro, 'M6')
        macros = device.macro.get_macros()
        self.assertEqual([macro.to_dict() for macro in macros['M6']], [url_macro.to_dict()])

        with self.assertRaises(ValueError):
            device.macro.add_macro('M1', 'lalala')  # Not a sequence

        with self.assertRaises(ValueError):
            device.macro.add_macro('M1', ['lalala'])  # Bad element in sequence
        self.assertEqual([macro.to_dict() for macro in device.macro.get_macros()['M1']], [url_macro.to_dict()])

    def test_device_keyboard_macro_del(self):
        device = self.device

        url_macro = device.macro.create_url_macro_item('http://example.org')
        device.macro.add_macro('M2', [url_macro])

        macros = device.macro.get_macros()
        self.assertIn('M2', macros)

        device.macro.del_macro('M2')
        macros = device.macro.get_macros()
        self.assertNotIn('M2', macros)

        device.macro.add_macro('M6', [url_macro])
        device.macro.del_macro('M6')
        self.assertNotIn('M6', device.macro.get_macros())

        with self.assertRaises(ValueError):
            device.macro.del_macro('INVALID_KEY')


if __name__ == "__main__":
    unittest.main()
