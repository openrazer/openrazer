import random
import sys
import glob
import os
import pprint

def get_driver_string(base_path, filename):
    with open(os.path.join(base_path, filename), 'r') as open_file:
        return open_file.read().strip()

def ask_is_right():
    answer = 'NAN'

    while answer not in ('YES', 'Y', 'NO', 'N'):
        answer = input('Is this correct [y/N]: ')
        answer = answer.strip().upper()
        if len(answer) == 0:
            answer = 'N'

    return answer in ('YES', 'Y')


def get_string_test(driver_file, name=None):
    if name is None:
        name = driver_file

    def test(driver_path):
        print("Getting {0} string... '{1}'".format(name, get_driver_string(driver_path, driver_file)))
        is_right = ask_is_right()

        return name, is_right

    return test

def set_brightness(driver_path):
    file = os.path.join(driver_path, 'set_brightness')

    print("Setting keyboard to max brightness...")
    open(file, 'w').write('255')
    is_right = ask_is_right()

    if not is_right:
        return 'set_brightness', False

    rand = random.Random()
    random_brightness = rand.randint(100, 150)
    print("Setting keyboard to a random medium brightness...")
    open(file, 'w').write(str(random_brightness))
    is_right = ask_is_right()

    if not is_right:
        return 'set_brightness', False

    print("Setting keyboard to max brightness... (255)")
    open(file, 'w').write('255')

    return 'set_brightness', True

def mode_none(driver_path):
    file = os.path.join(driver_path, 'mode_none')

    print("Setting keyboard to no effect...")
    open(file, 'w').write('1')
    is_right = ask_is_right()

    return 'mode_none', is_right

def mode_static(driver_path):
    file = os.path.join(driver_path, 'mode_static')

    print("Setting keyboard to dark green...")
    open(file, 'wb').write(b'\x00\x44\x00')
    is_right = ask_is_right()

    if not is_right:
        return 'mode_static', False

    print("Setting keyboard to bright green...")
    open(file, 'wb').write(b'\x00\xFF\x00')
    is_right = ask_is_right()

    return 'set_brightness', is_right







if __name__ == '__main__':

    # Find keyboard
    if not os.path.exists('/sys/bus/hid/drivers/razerkbd'):
        print('/sys/bus/hid/drivers/razerkbd does not exist. Is the driver loaded?', file=sys.stderr)
        sys.exit(1)

    try:
        driver_path = glob.glob(os.path.join('/sys/bus/hid/drivers/razerkbd', '*:*:*.*'))[0]
    except IndexError:
        print('Could not find keyboard under /sys/bus/hid/drivers/razerkbd, is it binded', file=sys.stderr)
        sys.exit(1)

    files = set([drv_file for drv_file in os.listdir(driver_path) if os.path.isfile(os.path.join(driver_path, drv_file))]) - {'test', 'country', 'reset', 'modalias', 'uevent', 'report_descriptor', 'temp_clear_row'}
    files = tuple(sorted(list(files)))

    mapping = {
        'device_type': get_string_test('device_type'),
        'get_serial': get_string_test('get_serial'),
        'get_firmware_version': get_string_test('get_firmware_version'),
        'set_brightness': [set_brightness, get_string_test('set_brightness', name='get_brightness')],
        'mode_none': mode_none,
        'mode_static': mode_static,
    }

    answer_map = {}

    for drv_file in files:
        if drv_file in mapping:
            if isinstance(mapping[drv_file], (list, tuple)):
                result = True
                key = None

                for func in mapping[drv_file]:
                    key, ok = func(driver_path)

                    result &= ok

                answer_map[key] = result
            else:
                key, ok = mapping[drv_file](driver_path)

                answer_map[key] = ok
        else:
            print("No test for {0}".format(drv_file))

    print("Results")
    pprint.pprint(answer_map)

