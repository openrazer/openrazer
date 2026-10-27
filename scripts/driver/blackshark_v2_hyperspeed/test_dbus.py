# SPDX-License-Identifier: GPL-2.0-or-later
# Run inside dbus-run-session; only generated fake devices are exposed.
import os
import subprocess
import sys
import tempfile
import time
from pathlib import Path
import dbus
from openrazer._fake_driver import FakeDevice
from openrazer.client import DeviceManager
root = Path(__file__).resolve().parents[3]
with tempfile.TemporaryDirectory(prefix='openrazer-blackshark-dbus-') as folder:
    test = Path(folder)
    devices = test / 'devices'
    devices.mkdir()
    config = test / 'razer.conf'
    config.write_text('[General]\nverbose_logging=True\n[Startup]\nsync_effects_enabled=False\ndevices_off_on_screensaver=False\nbattery_notifier=False\nrestore_persistence=False\n')
    fake = [FakeDevice('razerblacksharkv2hyperspeed' + mode, tmp_dir=str(devices)) for mode in ('wireless', 'wired')]
    entry = test / 'daemon_entry.py'
    entry.write_text("import runpy\nfrom openrazer_daemon.daemon import RazerDaemon\n# Fake files belong to the test user, so no distro-specific group is needed.\nRazerDaemon._check_plugdev_group = lambda self: True\nrunpy.run_path(" + repr(str(root / 'daemon/run_openrazer_daemon.py')) + ", run_name='__main__')\n")
    log = open(test / 'daemon.log', 'w+')
    process = subprocess.Popen([sys.executable, str(entry), '-F', '--test-dir', str(devices), '--config', str(config), '--persistence', str(test / 'persistence.conf'), '--run-dir', str(test / 'run'), '--log-dir', str(test / 'logs')], cwd=root, stdout=log, stderr=subprocess.STDOUT)
    try:
        bus = dbus.SessionBus()
        for _ in range(100):
            if bus.name_has_owner('org.razer'):
                break
            if process.poll() is not None:
                raise RuntimeError('Daemon exited')
            time.sleep(.05)
        manager = DeviceManager()
        assert len(manager.devices) == 2, [d.name for d in manager.devices]
        for d in manager.devices:
            assert d.type == 'headset' and d.battery_level == 100 and d.is_charging
            assert d.headset_firmware_version == 'v1.7.0.0'
            assert d.has('equalizer') and d.has('sidetone')
            d.sidetone_level = 12
            assert d.sidetone_level == 12
            d.sidetone_enabled = True
            assert d.sidetone_enabled
            d.equalizer_bands = (-9, -3, 0, 1, 2, 3, 4, 5, 6, 0)
            assert d.equalizer_bands == (-9, -3, 0, 1, 2, 3, 4, 5, 6, 0)
            d.equalizer_preset = 7
            assert d.equalizer_preset == 7
            d.equalizer_enabled = False
            assert not d.equalizer_enabled
            d.do_not_disturb = True
            assert d.do_not_disturb
            assert d.microphone_muted is False
            d.set_idle_time(1800)
            assert d.get_idle_time() == 1800
            if d._pid == 0x0565:
                d.dongle_led_mode = 3
                assert d.dongle_led_mode == 3 and d.wireless_connected
            else:
                assert not d.has('dongle_led_mode') and not d.has('wireless_connected')
        print('Both fake devices discovered; battery, firmware, audio, EQ, power and receiver D-Bus/pylib round trips passed')
    except BaseException:
        log.flush()
        log.seek(0)
        print(log.read()[-10000:])
        raise
    finally:
        process.terminate()
        try:
            process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()
        log.close()
        for d in fake:
            d.close()
