#!/usr/bin/python3
import openrazer.client

devmgr = openrazer.client.DeviceManager()
passed = True


def _test_failed(name, msg):
    global passed
    passed = False
    print(f"\nFAILED: {name}\n       ", msg)


def test_sanity_check_matrix_capabilities(d):
    if d.has("lighting_led_matrix"):
        d.fx.advanced.matrix[0, 0] = [0, 255, 0]
        try:
            d.fx.advanced.draw()
        except Exception as e:
            _test_failed(d.name, e)


def test_wired_wireless_naming():
    # Enforce the (Wired) or (Wireless) suffix naming convention when devices have multiple PIDs
    device_names = list(map(lambda device: device.name, devmgr.devices))

    for name in device_names:
        if name.find("(Wired)") != -1:
            striped_name = name.replace("(Wired)", "").strip()
            if striped_name in device_names:
                _test_failed(striped_name, "Naming pattern is inconsistent. Append \"(Wireless)\" for the other USB PID.")

        elif name.find("(Wireless)") != -1:
            striped_name = name.replace("(Wireless)", "").strip()
            if striped_name in device_names:
                _test_failed(striped_name, "Naming pattern is inconsistent. Append \"(Wired)\" for the other USB PID.")


for d in devmgr.devices:
    test_sanity_check_matrix_capabilities(d)
test_wired_wireless_naming()

if not passed:
    exit(1)
