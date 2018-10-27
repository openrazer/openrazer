#!/usr/bin/python3
"""
Simple program to walk through each key, illuminating it red so I can see
what element in the 22 RGB list each key refers to.
"""

import glob

colour_file = glob.glob('/sys/bus/hid/drivers/razerkbd/0*/matrix_custom_frame')[0]
custom_mode = glob.glob('/sys/bus/hid/drivers/razerkbd/0*/matrix_effect_custom')[0]


def clear_row(row_num):

    result = bytes([row_num, 0x00, 0x15])  # Results in b'\x00', b'\x01' ...

    for i in range(0, 22):
        result += b'\x00\x00\x00'

    return result


def gen_row(row_num):

    result = bytes([row_num, 0, 0x15])  # Results in b'\x00', b'\x01' ...

    for i in range(0, 22):
        for j in range(0, 22):
            if j == i:
                result += b'\x00\xFF\x00'
            else:
                result += b'\x00\x00\x00'
        yield result

        # Reset result
        result = bytes([row_num, 0x00, 0x15])


def write_binarystr(filename, binary_str):
    with open(filename, 'wb') as bin_file:
        bin_file.write(binary_str)


for i in range(0, 6):
    write_binarystr(colour_file, clear_row(i))
    write_binarystr(custom_mode, bytes('1', 'ascii'))

for i in range(0, 6):

    for key_id, byte_str in enumerate(gen_row(i)):
        write_binarystr(colour_file, byte_str)
        write_binarystr(custom_mode, bytes('1', 'ascii'))
        print("ROW {0}:{1}".format(i, key_id))
        input()
    write_binarystr(colour_file, clear_row(i))
    write_binarystr(custom_mode, bytes('1', 'ascii'))
