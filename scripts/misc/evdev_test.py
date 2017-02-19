from evdev import InputDevice, categorize, ecodes, UInput
import glob
import selectors

selector = selectors.DefaultSelector()

devices = [InputDevice(dev) for dev in glob.glob('/dev/input/by-id/usb-Razer_*-kbd')]

for device in devices:
    selector.register(device, selectors.EVENT_READ)

while True:
    for key, mask in selector.select():
        device = key.fileobj
        for event in device.read():
            if event.type == ecodes.EV_KEY:
                print(categorize(event))
