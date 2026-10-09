# SPDX-License-Identifier: GPL-2.0-or-later

"""Run with dbus-run-session to isolate the fake dock from the user's daemon."""

import multiprocessing
import os
import signal
import tempfile
import traceback
import unittest
from unittest.mock import Mock

import dbus

from openrazer._fake_driver import FakeDevice
from openrazer.client.devices import RazerDevice
from openrazer_daemon.daemon import RazerDaemon


class IsolatedDockDaemon(RazerDaemon):
    """Keep fake endpoints deterministic and never listen for real udev events."""

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


def run_fake_dock_daemon(driver_dir, ready):
    try:
        daemon = IsolatedDockDaemon(test_dir=driver_dir)
    except Exception:
        ready.send(traceback.format_exc())
        ready.close()
        raise
    ready.send(None)
    ready.close()
    daemon.run()


class MouseDockProClientTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if dbus.SessionBus().name_has_owner('org.razer'):
            raise unittest.SkipTest('An OpenRazer daemon owns this bus; use dbus-run-session')
        cls.tmp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.fake_dock = FakeDevice('razermousedockpro', tmp_dir=cls.tmp.name)
        cls.addClassCleanup(cls.fake_dock.close)
        context = multiprocessing.get_context('spawn')
        ready, child_ready = context.Pipe(duplex=False)
        cls.addClassCleanup(ready.close)
        cls.process = context.Process(target=run_fake_dock_daemon, args=(cls.tmp.name, child_ready))
        cls.process.start()
        child_ready.close()
        cls.addClassCleanup(cls.stop_daemon)
        if not ready.poll(10):
            raise RuntimeError('Fake Mouse Dock Pro daemon did not become ready')
        failure = ready.recv()
        if failure is not None:
            raise RuntimeError(failure)

    @classmethod
    def stop_daemon(cls):
        if cls.process.is_alive():
            os.kill(cls.process.pid, signal.SIGINT)
            cls.process.join(5)
        if cls.process.is_alive():
            cls.process.terminate()
            cls.process.join(5)
        cls.process.close()

    def setUp(self):
        self.fake_dock.set('paired_slots', '1:1:00ab 2:0:ffff')
        self.fake_dock.set('poll_rate', '1000')
        self.mouse = RazerDevice('MM00000000A4')

    def test_polling_getter_refreshes_hardware_instead_of_returning_prior_write(self):
        self.mouse.poll_rate = 1000
        self.fake_dock.set('poll_rate', '500')
        self.assertEqual(self.mouse.poll_rate, 500)


if __name__ == '__main__':
    unittest.main()
