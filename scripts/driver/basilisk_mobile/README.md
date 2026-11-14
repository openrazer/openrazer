# Basilisk Mobile startup without the OpenRazer daemon

OpenRazer's daemon normally selects driver mode for the Basilisk Mobile. This
optional helper is for systems that use the kernel driver without that daemon.
It requires Linux, systemd and Python 3, with no additional Python packages.

A successful userspace request for driver mode enables the kernel driver's wake
recovery. The mouse can initially work using a retained firmware setting even
when its startup request failed. If the firmware later returns to normal mode,
recovery will not be armed for the current driver instance.

A single udev `RUN` command can fail if the receiver is not ready during boot.
This helper waits for the driver's input node, requests mode `0300`, and verifies
readback. It always writes the mode even if the firmware already reports `0300`,
so the kernel also records the request. It makes up to three attempts, one second
apart. If unsuccessful, it waits for actual mouse input before another set of
attempts. It does not poll USB while waiting idle, grab input, or retain input
data. It exits after successful initialization; the kernel handles later wakes.

The rule handles wired (`1532:00D3`) and receiver (`1532:00D4`) connections, only
on primary interface 00. Driver unbind marks the device unready so a later bind
can launch initialization again. Hardware testing used the wireless receiver.

## Install

Remove any earlier Basilisk Mobile driver-mode startup rule, including the old
single-command example in the main README. If a custom rule file also handles a
keyboard, keep its keyboard entries. Use this helper only when you want driver
mode selected automatically; do not combine it with another mode manager.

From the repository root:

```sh
sudo install -Dm755 scripts/driver/basilisk_mobile/openrazer_mouse_init.py /usr/local/libexec/openrazer-mouse-init
sudo install -Dm644 scripts/driver/basilisk_mobile/openrazer-mouse-init@.service /etc/systemd/system/openrazer-mouse-init@.service
sudo install -Dm644 scripts/driver/basilisk_mobile/99-razer-basilisk-mobile.rules /etc/udev/rules.d/99-razer-basilisk-mobile.rules
sudo systemctl daemon-reload
sudo udevadm control --reload-rules
```

Reconnect the receiver or wired mouse. Udev starts the service automatically;
it does not need to be enabled with `systemctl enable`.

Inspect initialization with:

```sh
journalctl -b -u 'openrazer-mouse-init@*'
```

Successful initialization logs "Basilisk Mobile driver mode initialized and
verified." The service then becomes inactive. If the mouse is not ready, it
logs that it is waiting for activity; move the mouse to let it retry.

## Remove

Disconnect the receiver or wired mouse, remove the three installed files listed
above, then reload systemd and udev with the two reload commands. Reconnect the
mouse afterward. This does not uninstall the kernel driver.

## Validation

Run the isolated tests without hardware or administrator access:

```sh
python3 -m unittest discover -s scripts/driver/basilisk_mobile -p 'test_*.py'
```

The helper was tested through udev bind and fresh module initialization, followed
by a simulated firmware mode reset and automatic kernel recovery. The user also
reported successful everyday use after a full reboot, without the previously
observed failures after device sleep. Those later observations did not include
a USB capture proving each firmware sleep transition.
