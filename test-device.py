from openrazer.client import DeviceManager

manager = DeviceManager()
devices = manager.devices

for device in devices:
    print(f"Device: {device.name}")
    print(f"Type: {device.type}")
    print(f"Serial: {device.serial}")