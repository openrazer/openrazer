
import struct

EVENT_FORMAT = '@llHHI'
EVENT_SIZE = struct.calcsize(EVENT_FORMAT)

with open('/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-event-kbd', 'rb') as open_file:

    while True:
        payload = open_file.read(EVENT_SIZE)

        ev_type, code, value = struct.unpack(EVENT_FORMAT, payload)[2:]

        if ev_type == 1:
            print("Type: {0}, Code: {1}, Value: {2}".format(ev_type, code, value))