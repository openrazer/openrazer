"""
Hardcore mocking to remove the need for starting the thread and dbus
"""
import unittest
import unittest.mock
import time

import razer_daemon.misc.screensaver_thread

class DummyParent(object):
    def __init__(self):
        self.suspend_devices = unittest.mock.MagicMock()
        self.resume_devices = unittest.mock.MagicMock()
    #     self.resume_called = False
    #     self.suspend_called = False
    #
    # def resume_devices(self):
    #     self.resume_called = True
    #
    # def suspend_devices(self):
    #     self.suspend_called = True


def get_dbus_session(*args):
    # Setup dbus mock
    dbus_session_mock = unittest.mock.MagicMock()
    dbus_session_mock.get_object.return_value = 'screensaver_object'
    return dbus_session_mock

def get_dbus_session_exception(*args):
    # Setup dbus mock
    raise razer_daemon.misc.screensaver_thread.dbus.exceptions.DBusException

def get_dbus_interface(*args):
    # Setup dbus interface mock
    dbus_interface = unittest.mock.MagicMock()
    dbus_interface.GetActive.side_effect = [False,False,True,True,True,True,True,False,False]
    return dbus_interface

def get_dbus_interface_exception(*args):
    # Setup dbus interface mock
    raise razer_daemon.misc.screensaver_thread.dbus.exceptions.DBusException

def logger_mock(*args):
    return unittest.mock.MagicMock()

class ScreensaverThreadTest(unittest.TestCase):

    def __init__(self, *args):
        super(ScreensaverThreadTest, self).__init__(*args)

        self.screensaver_active = False

    @unittest.mock.patch('razer_daemon.misc.screensaver_thread.logging.getLogger', logger_mock)
    def setUp(self):
        self.parent = DummyParent()
        self.screensaver = razer_daemon.misc.screensaver_thread.ScreensaverThread(self.parent, active=True)

    @unittest.mock.patch('razer_daemon.misc.screensaver_thread.time.sleep', get_dbus_session)
    @unittest.mock.patch('razer_daemon.misc.screensaver_thread.dbus.SessionBus', get_dbus_session)
    @unittest.mock.patch('razer_daemon.misc.screensaver_thread.dbus.Interface', get_dbus_interface)
    def test_thread_start_stop(self):
        self.screensaver.start()

        time.sleep(0.5)

        self.screensaver.shutdown = True
        self.screensaver.join(timeout=2)
        self.assertFalse(self.screensaver.is_alive())

    @unittest.mock.patch('razer_daemon.misc.screensaver_thread.dbus.SessionBus', get_dbus_session)
    @unittest.mock.patch('razer_daemon.misc.screensaver_thread.dbus.Interface', get_dbus_interface)
    def test_load_dbus(self):
        self.screensaver.load_dbus()

        self.assertTrue(isinstance(self.screensaver._dbus_interface, unittest.mock.MagicMock))

    @unittest.mock.patch('razer_daemon.misc.screensaver_thread.dbus.SessionBus', get_dbus_session_exception)
    @unittest.mock.patch('razer_daemon.misc.screensaver_thread.dbus.Interface', get_dbus_interface)
    def test_load_dbus_exception(self):
        # Exception should be caught
        self.screensaver.load_dbus()

        self.assertIsNone(self.screensaver._dbus_interface)
        self.assertTrue(self.screensaver.logger.exception.called)

    @unittest.mock.patch('razer_daemon.misc.screensaver_thread.dbus.SessionBus', get_dbus_session)
    @unittest.mock.patch('razer_daemon.misc.screensaver_thread.dbus.Interface', get_dbus_interface_exception)
    def test_load_dbus_no_screensaver(self):
        # Exception should be caught
        self.screensaver.load_dbus()

        self.assertIsNone(self.screensaver._dbus_interface)
        self.assertTrue(self.screensaver.logger.warning.called)

    @unittest.mock.patch('razer_daemon.misc.screensaver_thread.time.sleep', get_dbus_session)
    @unittest.mock.patch('razer_daemon.misc.screensaver_thread.dbus.SessionBus', get_dbus_session)
    @unittest.mock.patch('razer_daemon.misc.screensaver_thread.dbus.Interface', get_dbus_interface)
    def test_run_active(self):
        type(self.screensaver)._shutdown = unittest.mock.PropertyMock(side_effect=[False,False,False,False,False,False,False,False,False,False,False,False,False,True])
        self.screensaver.run()

        self.assertTrue(self.screensaver._dbus_interface.GetActive.called)
        self.assertTrue(self.parent.resume_devices.called)
        self.assertTrue(self.parent.suspend_devices.called)

        type(self.screensaver)._shutdown = False

    def test_properties(self):
        self.screensaver.shutdown = True
        self.assertTrue(self.screensaver.shutdown)
        self.screensaver.shutdown = False
        self.assertFalse(self.screensaver.shutdown)

        self.screensaver.active = True
        self.assertTrue(self.screensaver.active)
        self.screensaver.active = False
        self.assertFalse(self.screensaver.active)

