# SPDX-License-Identifier: GPL-2.0-or-later
"""Register and remove Bluetooth objects in the daemon's device collection."""
from gi.repository import GLib
from .backend import BlueZBackend
from .device import BasiliskV3ProBluetooth


class BluetoothManager:
    def __init__(self, daemon):
        self.daemon = daemon
        self.backend = BlueZBackend()
        self.registered = {}
        self.poll()
        self.source = GLib.timeout_add_seconds(3, self.poll)

    def reconcile(self, discovered):
        for path in list(self.registered):
            if path not in discovered:
                device = self.registered.pop(path)
                device.close()
                device.remove_from_connection()
                self.daemon._razer_devices.remove(path)
                self.daemon.device_removed()
        for path, address in discovered.items():
            if path in self.registered:
                continue
            device = BasiliskV3ProBluetooth(self.backend, path, address)
            device.effect_sync = self.daemon._config.getboolean('Startup', 'sync_effects_enabled')
            self.daemon._razer_devices.add(path, device.serial, device)
            self.registered[path] = device
            self.daemon.device_added()
            self.daemon.logger.info('Found Bluetooth Basilisk V3 Pro: %s', device.serial)

    def poll(self):
        try:
            self.reconcile(self.backend.call(self.backend.discover()))
        except Exception:
            self.daemon.logger.warning('Bluetooth discovery failed', exc_info=True)
            # Do not expose stale objects when BlueZ is unavailable.
            self.reconcile({})
        return True

    def close(self):
        GLib.source_remove(self.source)
        try:
            self.reconcile({})
        finally:
            self.backend.close()
