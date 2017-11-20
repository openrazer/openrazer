import os
import sys
import argparse
import struct
from openrazer_daemon.keyboard import EVENT_MAPPING, TARTARUS_EVENT_MAPPING

EVENT_FORMAT = '@llHHI'
EVENT_SIZE = struct.calcsize(EVENT_FORMAT)


def loop_on_event(event_file, mapping):
    with open(event_file, 'rb') as open_file:

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

                code = mapping.get(code, code)

                print("Type: EV_KEY, Code: {0}, Value: {1}".format(code, value))
            else:
                print("Type: {0}, Code: {1}, Value: {2}".format(ev_type, code, value))


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('event_file', metavar='EVENT_FILE', type=str, help="Device event file like \"/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-event-kbd\"")
    parser.add_argument('--tartarus', action='store_true', help='Use the tartarus event mapping instead')

    return parser.parse_args()


def run():
    args = parse_args()

    if not os.path.exists(args.event_file):
        print('Event file does not exist', file=sys.stderr)
        sys.exit(1)

    if args.tartarus:
        mapping = TARTARUS_EVENT_MAPPING
    else:
        #mapping = EVENT_MAPPING
        mapping = {}

    print('Starting. Press keys', file=sys.stderr)

    try:
        loop_on_event(args.event_file, mapping)
    except KeyboardInterrupt:
        print("Exiting", file=sys.stderr)


if __name__ == '__main__':
    run()
