# OpenRazer protocol on macOS (userspace)

The OpenRazer kernel module is Linux-only, but the Razer vendor protocol it
implements does not need a kernel driver: it is carried in HID **feature
reports** on the device's control interface, and macOS's IOHIDManager lets a
plain userspace process exchange feature reports with a vendor device — no
kext, no DriverKit extension, no special entitlements.

`razer_cli.py` is a small proof of that: a dependency-light CLI that reuses
the report format, checksum, command classes and transaction ids from
`driver/razercommon.c` / `driver/razerchromacommon.c`, talking through
[hidapi](https://github.com/libusb/hidapi).

```
brew install hidapi
pip install hid
./razer_cli.py                 # status
./razer_cli.py dpi 1600
./razer_cli.py poll 1000
./razer_cli.py stages 400,800,1600 --active 2
./razer_cli.py idle 600
```

## macOS-specific findings

- Each USB interface shows up as one or more IOHIDDevice collections.
  Commands must go to the device's control interface (interface 0 on mice);
  other collections accept SET_REPORT but never answer.
- Keyboard-usage interfaces are seized by macOS and can't be opened without
  Input Monitoring permission — fortunately they are not needed.
- After SET_REPORT, poll GET_REPORT until `status != 0x00`; wireless
  receivers take ~50–100 ms to round-trip the command to the mouse.

## Supported devices

| Device | PID | Verified |
|---|---|---|
| Razer Pro Click Mini (Receiver) | 1532:009A | ✅ real hardware |

Adding a device = one `DeviceSpec` entry (PID, transaction id, feature set),
all copied from the Linux driver sources. Please only mark entries verified
after testing on hardware.
