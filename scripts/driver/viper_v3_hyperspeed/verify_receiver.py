#!/usr/bin/env python3
"""Read-only protocol check for the Viper V3 HyperSpeed receiver (1532:00B8).

Install ``hidapi`` in an isolated environment, then run this script. It sends
GET commands only; it does not change configuration or reset the device.
"""

import argparse
import json
import time
from datetime import datetime, timezone

VID, PID = 0x1532, 0x00B8
REPORT_SIZE = 91


def make_report(command_class, command, data):
    if len(data) > 80:
        raise ValueError("data exceeds 80 bytes")
    report = bytearray(REPORT_SIZE)
    report[2] = 0x1F
    report[6:9] = bytes((len(data), command_class, command))
    report[9:9 + len(data)] = data
    report[89] = checksum(report)
    return bytes(report)


def checksum(report):
    value = 0
    for byte in report[3:89]:
        value ^= byte
    return value


def open_receiver(hid):
    devices = sorted(hid.enumerate(VID, PID), key=lambda item: item.get("interface_number") != 0)
    for item in devices:
        device = hid.device()
        try:
            device.open_path(item["path"])
            return device, item
        except OSError:
            pass
    raise RuntimeError("no openable 1532:00B8 HID interface")


def read_command(device, command_class, command, data):
    request = make_report(command_class, command, data)
    written = device.send_feature_report(request)
    if written not in (90, 91):
        raise RuntimeError(f"short Feature Report write: {written}")
    statuses = []
    deadline = time.monotonic() + 3.0
    while time.monotonic() < deadline:
        time.sleep(0.05)
        raw = bytes(device.get_feature_report(0, REPORT_SIZE))
        if len(raw) == 90:
            raw = b"\0" + raw
        if len(raw) != REPORT_SIZE:
            continue
        if raw[2] != request[2] or raw[7:9] != request[7:9]:
            continue
        if checksum(raw) != raw[89]:
            raise RuntimeError(f"bad checksum for {command_class:02x}/{command:02x}")
        statuses.append(raw[1])
        if raw[1] == 1:  # Busy: poll GET_REPORT without sending again.
            continue
        if raw[1] != 2:
            raise RuntimeError(f"device status {raw[1]:02x} for {command_class:02x}/{command:02x}")
        return raw, statuses
    raise TimeoutError(f"no final response for {command_class:02x}/{command:02x}; statuses={statuses}")


def verify(device):
    checks = {}

    def get(name, command_class, command, data):
        raw, statuses = read_command(device, command_class, command, data)
        checks[name] = {"statuses": statuses, "data_size": raw[6]}
        return raw[9:89]

    poll = get("polling", 0, 0x85, b"\0")
    checks["polling"]["hz"] = {1: 1000, 2: 500, 8: 125}.get(poll[0])

    dpi = get("dpi", 4, 0x85, b"\0")
    # This receiver reports data_size=1, but X/Y occupy the next four bytes.
    checks["dpi"]["xy"] = [int.from_bytes(dpi[1:3], "big"), int.from_bytes(dpi[3:5], "big")]

    stages = get("dpi_stages", 4, 0x86, b"\1" + bytes(37))
    count = stages[2]
    if not 1 <= count <= 5:
        raise RuntimeError(f"invalid stage count {count}")
    checks["dpi_stages"]["active"] = stages[1]
    checks["dpi_stages"]["values"] = [
        int.from_bytes(stages[4 + index * 7:6 + index * 7], "big")
        for index in range(count)
    ]
    checks["dpi_stages"]["extra_pairs"] = [
        int.from_bytes(stages[8 + index * 7:10 + index * 7], "big")
        for index in range(count)
    ]

    idle = get("idle", 7, 0x83, b"\0\0")
    checks["idle"]["seconds"] = int.from_bytes(idle[:2], "big")

    battery = get("battery", 7, 0x80, b"\0\0")
    checks["battery"]["raw"] = battery[1]

    charging = get("charging", 7, 0x84, b"\0\0")
    checks["charging"]["value"] = charging[1]

    threshold = get("low_battery_threshold", 7, 0x81, b"\0")
    checks["low_battery_threshold"]["raw"] = threshold[0]
    return checks


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", help="write the JSON report to this path")
    parser.add_argument("--self-test", action="store_true", help="check packet codec without hardware")
    args = parser.parse_args()
    assert make_report(0, 0x85, b"\0")[89] == 0x84
    if args.self_test:
        print("packet codec: pass")
        return

    import hid  # pylint: disable=import-error  # Optional hardware dependency.
    device, item = open_receiver(hid)
    try:
        result = {
            "timestamp_utc": datetime.now(timezone.utc).isoformat(),
            "vid_pid": "1532:00B8",
            "interface": item.get("interface_number"),
            "checks": verify(device),
        }
    finally:
        device.close()
    output = json.dumps(result, indent=2, ensure_ascii=False) + "\n"
    if args.output:
        with open(args.output, "w", encoding="utf-8") as handle:
            handle.write(output)
    print(output, end="")


if __name__ == "__main__":
    main()
