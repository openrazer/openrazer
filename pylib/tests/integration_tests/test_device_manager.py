#!/usr/bin/env python3

import multiprocessing
import os
import signal
import tempfile
import time
import unittest
import shutil
import openrazer.client
import openrazer_daemon.daemon
import openrazer._fake_driver as fake_driver

import coverage


def run_daemon(daemon_dir, driver_dir):
    # TODO console_log false
    openrazer_daemon.daemon.RazerDaemon(verbose=True, console_log=False, test_dir=driver_dir)


class DeviceManagerTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls._daemon_dir = tempfile.mkdtemp(prefix='tmp_', suffix='_daemondata')
        cls._tmp_dir = tempfile.mkdtemp(prefix='tmp_', suffix='_daemontest')

        cls._bw_serial = 'IO0000000000001'
        cls._bw_chroma = fake_driver.FakeDevice('razerblackwidowchroma', serial=cls._bw_serial, tmp_dir=cls._tmp_dir)
        print("Created BlackWidow Chroma endpoints")

        cls._daemon_proc = multiprocessing.Process(target=run_daemon, args=(cls._daemon_dir, cls._tmp_dir))
        cls._daemon_proc.start()
        print("Started daemon")
        time.sleep(5)

    @classmethod
    def tearDownClass(cls):
        print("Stopping daemon")
        os.kill(cls._daemon_proc.pid, signal.SIGINT)
        time.sleep(3)
        if cls._daemon_proc.is_alive():
            print("Daemon still alive...")
            time.sleep(8)
            if cls._daemon_proc.is_alive():
                cls._daemon_proc.terminate()

        if cls._daemon_proc.is_alive():
            print("Failed to kill daemon")

        cls._bw_chroma.close()

        shutil.rmtree(cls._tmp_dir)
        shutil.rmtree(cls._daemon_dir)

        time.sleep(5)

    def setUp(self):
        self._bw_chroma.create_endpoints()

        self.device_manager = openrazer.client.DeviceManager()

    def test_device_list(self):
        self.assertEqual(len(self.device_manager.devices), 1)

    def test_serial(self):
        device = self.device_manager.devices[0]

        self.assertEqual(device.serial, self._bw_chroma.get('get_serial'))

    def test_name(self):
        device = self.device_manager.devices[0]

        self.assertEqual(device.name, self._bw_chroma.get('device_type'))

    def test_type(self):
        device = self.device_manager.devices[0]

        self.assertEqual(device.type, 'keyboard')

    def test_fw_version(self):
        device = self.device_manager.devices[0]

        self.assertEqual(device.firmware_version, self._bw_chroma.get('get_firmware_version'))

    def test_brightness(self):
        device = self.device_manager.devices[0]

        # Test 100%
        device.brightness = 100.0

        self.assertEqual('255', self._bw_chroma.get('set_brightness'))

        self.assertEqual(100.0, device.brightness)

        device.brightness = 50.0

        self.assertEqual('127', self._bw_chroma.get('set_brightness'))

        self.assertAlmostEqual(50.0, device.brightness, delta=0.4)

        device.brightness = 0.0

        self.assertEqual('0', self._bw_chroma.get('set_brightness'))

        self.assertEqual(0, device.brightness)

    def test_capabilities(self):
        device = self.device_manager.devices[0]

        self.assertEqual(device.capabilities, device._capabilities)

    def test_device_keyboard_game_mode(self):
        device = self.device_manager.devices[0]

        self._bw_chroma.set('mode_game', '1')
        self.assertTrue(device.game_mode_led)
        device.game_mode_led = False
        self.assertEqual(self._bw_chroma.get('mode_game'), '0')
        device.game_mode_led = True
        self.assertEqual(self._bw_chroma.get('mode_game'), '1')

    def test_device_keyboard_macro_mode(self):
        device = self.device_manager.devices[0]

        self._bw_chroma.set('mode_macro', '1')
        self.assertTrue(device.macro_mode_led)
        device.macro_mode_led = False
        self.assertEqual(self._bw_chroma.get('mode_macro'), '0')
        device.macro_mode_led = True
        self.assertEqual(self._bw_chroma.get('mode_macro'), '1')

        self._bw_chroma.set('mode_macro_effect', '0')
        self.assertEqual(device.macro_mode_led_effect, openrazer.client.constants.MACRO_LED_STATIC)
        device.macro_mode_led_effect = openrazer.client.constants.MACRO_LED_BLINK
        self.assertEqual(self._bw_chroma.get('mode_macro'), str(openrazer.client.constants.MACRO_LED_BLINK))

    def test_device_keyboard_effect_none(self):
        device = self.device_manager.devices[0]

        device.fx.none()

        self.assertEqual(self._bw_chroma.get('mode_none'), '1')

    def test_device_keyboard_effect_spectrum(self):
        device = self.device_manager.devices[0]

        device.fx.spectrum()

        self.assertEqual(self._bw_chroma.get('mode_spectrum'), '1')

    def test_device_keyboard_effect_wave(self):
        device = self.device_manager.devices[0]

        device.fx.wave(openrazer.client.constants.WAVE_LEFT)
        self.assertEqual(self._bw_chroma.get('mode_wave'), str(openrazer.client.constants.WAVE_LEFT))
        device.fx.wave(openrazer.client.constants.WAVE_RIGHT)
        self.assertEqual(self._bw_chroma.get('mode_wave'), str(openrazer.client.constants.WAVE_RIGHT))

        with self.assertRaises(ValueError):
            device.fx.wave('lalala')

    def test_device_keyboard_effect_static(self):
        device = self.device_manager.devices[0]

        device.fx.static(255, 0, 255)
        self.assertEqual(b'\xFF\x00\xFF', self._bw_chroma.get('mode_static', binary=True))

        for red, green, blue in ((256.0, 0, 0), (0, 256.0, 0), (0, 0, 256.0)):
            with self.assertRaises(ValueError):
                device.fx.static(red, green, blue)

        device.fx.static(256, 0, 700)
        self.assertEqual(b'\xFF\x00\xFF', self._bw_chroma.get('mode_static', binary=True))

    def test_device_keyboard_effect_reactive(self):
        device = self.device_manager.devices[0]

        time = openrazer.client.constants.REACTIVE_500MS
        device.fx.reactive(255, 0, 255, time)
        self.assertEqual(b'\x01\xFF\x00\xFF', self._bw_chroma.get('mode_reactive', binary=True))

        for red, green, blue in ((256.0, 0, 0), (0, 256.0, 0), (0, 0, 256.0)):
            with self.assertRaises(ValueError):
                device.fx.reactive(red, green, blue, time)

        device.fx.reactive(256, 0, 700, time)
        self.assertEqual(b'\x01\xFF\x00\xFF', self._bw_chroma.get('mode_reactive', binary=True))

        with self.assertRaises(ValueError):
            device.fx.reactive(255, 0, 255, 'lalala')

    def test_device_keyboard_effect_breath_single(self):
        device = self.device_manager.devices[0]

        device.fx.breath_single(255, 0, 255)
        self.assertEqual(b'\xFF\x00\xFF', self._bw_chroma.get('mode_breath', binary=True))

        for red, green, blue in ((256.0, 0, 0), (0, 256.0, 0), (0, 0, 256.0)):
            with self.assertRaises(ValueError):
                device.fx.breath_single(red, green, blue)

        device.fx.breath_single(256, 0, 700)
        self.assertEqual(b'\xFF\x00\xFF', self._bw_chroma.get('mode_breath', binary=True))

    def test_device_keyboard_effect_breath_dual(self):
        device = self.device_manager.devices[0]

        device.fx.breath_dual(255, 0, 255, 255, 0, 0)
        self.assertEqual(b'\xFF\x00\xFF\xFF\x00\x00', self._bw_chroma.get('mode_breath', binary=True))

        for r1, g1, b1, r2, g2, b2 in ((256.0, 0, 0, 0, 0, 0), (0, 256.0, 0, 0, 0, 0), (0, 0, 256.0, 0, 0, 0),
                                       (0, 0, 0, 256.0, 0, 0), (0, 0, 0, 0, 256.0, 0), (0, 0, 0, 0, 0, 256.0)):
            with self.assertRaises(ValueError):
                device.fx.breath_dual(r1, g1, b1, r2, g2, b2)

        device.fx.breath_dual(256, 0, 700, 255, 0, 0)
        self.assertEqual(b'\xFF\x00\xFF\xFF\x00\x00', self._bw_chroma.get('mode_breath', binary=True))

    def test_device_keyboard_effect_breath_random(self):
        device = self.device_manager.devices[0]

        device.fx.breath_random()

        self.assertEqual(self._bw_chroma.get('mode_breath'), '1')

    def test_device_keyboard_effect_ripple(self):
        device = self.device_manager.devices[0]

        refresh_rate = 0.01
        device.fx.ripple(255, 0, 255, refresh_rate)
        time.sleep(0.1)

        custom_effect_payload = self._bw_chroma.get('set_key_row', binary=True)
        self.assertGreater(len(custom_effect_payload), 1)
        self.assertEqual(self._bw_chroma.get('mode_custom'), '1')

        for red, green, blue in ((256.0, 0, 0), (0, 256.0, 0), (0, 0, 256.0)):
            with self.assertRaises(ValueError):
                device.fx.reactive(red, green, blue, refresh_rate)

        with self.assertRaises(ValueError):
            device.fx.reactive(255, 0, 255, 'lalala')

        device.fx.none()

    def test_device_keyboard_effect_random_ripple(self):
        device = self.device_manager.devices[0]

        refresh_rate = 0.01
        device.fx.ripple_random(refresh_rate)
        time.sleep(0.1)

        custom_effect_payload = self._bw_chroma.get('set_key_row', binary=True)
        self.assertGreater(len(custom_effect_payload), 1)
        self.assertEqual(self._bw_chroma.get('mode_custom'), '1')

        with self.assertRaises(ValueError):
            device.fx.ripple_random('lalala')

        device.fx.none()

    def test_device_keyboard_effect_framebuffer(self):
        device = self.device_manager.devices[0]

        device.fx.advanced.matrix.set(0, 0, (255, 0, 255))

        self.assertEqual(device.fx.advanced.matrix.get(0, 0), (255, 0, 255))

        device.fx.advanced.draw()
        custom_effect_payload = self._bw_chroma.get('set_key_row', binary=True)
        self.assertEqual(custom_effect_payload[:4], b'\x00\xFF\x00\xFF')

        device.fx.advanced.matrix.to_framebuffer()  # Save 255, 0, 255
        device.fx.advanced.matrix.reset()  # Clear FB

        device.fx.advanced.matrix.set(0, 0, (0, 255, 0))

        device.fx.advanced.draw_fb_or()  # Draw FB or'd with Matrix
        custom_effect_payload = self._bw_chroma.get('set_key_row', binary=True)
        self.assertEqual(custom_effect_payload[:4], b'\x00\xFF\xFF\xFF')

        # Append that to FB
        device.fx.advanced.matrix.to_framebuffer_or()
        device.fx.advanced.draw()
        custom_effect_payload = self._bw_chroma.get('set_key_row', binary=True)

        binary = device.fx.advanced.matrix.to_binary()

        self.assertEqual(binary, custom_effect_payload)

    def test_device_keyboard_macro_enable(self):
        device = self.device_manager.devices[0]

        device.macro.enable_macros()

        self.assertEqual(self._bw_chroma.get('macro_keys'), '1')

    def test_device_keyboard_macro_add(self):
        device = self.device_manager.devices[0]

        url_macro = device.macro.create_url_macro_item('http://example.org')
        device.macro.add_macro('M1', [url_macro])

        macros = device.macro.get_macros()
        self.assertIn('M1', macros)

        with self.assertRaises(ValueError):
            device.macro.add_macro('M6', url_macro)  # Unknown key

        with self.assertRaises(ValueError):
            device.macro.add_macro('M1', 'lalala')  # Not a sequnce

        with self.assertRaises(ValueError):
            device.macro.add_macro('M1', ['lalala'])  # Bad element in sequence

    def test_device_keyboard_macro_del(self):
        device = self.device_manager.devices[0]

        url_macro = device.macro.create_url_macro_item('http://example.org')
        device.macro.add_macro('M2', [url_macro])

        macros = device.macro.get_macros()
        self.assertIn('M2', macros)

        device.macro.del_macro('M2')
        macros = device.macro.get_macros()
        self.assertNotIn('M2', macros)

        with self.assertRaises(ValueError):
            device.macro.del_macro('M6')  # Unknown key


if __name__ == "__main__":
    unittest.main()
