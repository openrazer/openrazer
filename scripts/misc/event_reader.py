import struct
from razer_daemon.keyboard import EVENT_MAPPING

EVENT_FORMAT = '@llHHI'
EVENT_SIZE = struct.calcsize(EVENT_FORMAT)

with open('/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-event-kbd', 'rb') as open_file:

    while True:
        payload = open_file.read(EVENT_SIZE)

        ev_type, code, value = struct.unpack(EVENT_FORMAT, payload)[2:]

        if (ev_type == code == 0) or ev_type == 4:
            continue

        if ev_type == 1:
            if value == 0:
                value = 'UP'
            elif value == 1:
                value = 'DOWN'
            else:
                value = 'REPEAT'

            code = EVENT_MAPPING.get(code, code)

            print("Type: EV_KEY, Code: {1}, Value: {2}".format(ev_type, code, value))
        else:
            print("Type: {0}, Code: {1}, Value: {2}".format(ev_type, code, value))