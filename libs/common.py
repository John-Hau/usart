#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import hashlib
import base64
import random


def list_to_string(list_data):
    if type(list_data) is str:
        return list_data
    else:
        return "".join([chr(x) for x in list_data])


def string_to_list(string_data):
    if type(string_data) is list:
        return string_data
    elif type(string_data) is bytes:
        return list(string_data)
    else:
        return [ord(x) for x in string_data]


def int_to_list(in_data):
    if type(in_data) is list:
        return in_data
    else:
        return [in_data & 0xff, (in_data >> 8) & 0xff,
                (in_data >> 16) & 0xff, (in_data >> 24) & 0xff]


def int_to_list_be(in_data):
    if type(in_data) is list:
        return in_data
    else:
        return [(in_data >> 24) & 0xff, (in_data >> 16) & 0xff,
                (in_data >> 8) & 0xff, in_data & 0xff]


def list_to_int(list_data):
    if type(list_data) is int:
        return list_data
    else:
        return list_data[0] | (list_data[1] << 8) | \
               (list_data[2] << 16) | (list_data[3] << 24)


def list_to_num(in_data, in_len=0, little_endian=1):
    if in_len == 0:
        in_len = len(in_data)
    num_data = 0
    for idx in range(in_len):
        if little_endian:
            num_data |= in_data[idx]
        else:
            num_data |= in_data[in_len-1-idx]
        num_data <<= 8
    num_data &= 0xffffffff
    return num_data


def b64_encode(sys_argv):
    '''
    --b64 <filename>
    '''
    if len(sys_argv) == 0:
        print("param error")
        return None

    file_name = sys_argv[0]
    file_data = file_readf(file_name)

    file_data_list = string_to_list(file_data)

    encode_str = str(base64.b64encode(bytes(file_data_list)), "utf-8")
    print(encode_str)


def calc_crc(list_data):
    data = list_data
    crc = 0xFFFF
    for pos in data:
        crc ^= pos
        for i in range(8):
            if((crc & 1) != 0):
                crc >>= 1
                crc ^= 0xA001
            else:
                crc >>= 1
    return crc


def get_random_vector(vec_size):
    return [random.randrange(0, 0xff) for x in range(vec_size)]


def file_readf(path, r_len=0):
    if not os.path.exists(path):
        print(path)
        raise Exception("File not exists!")

    #print("Open file: " + path)
    f = open(path, 'rb')
    if r_len > 0:
        r_data = f.read(r_len)
    else:
        r_data = f.read()
    f.close()
    return r_data


def file_writef(path, wdata):
    if wdata is None or len(wdata) == 0:
        raise Exception("Data error!")

    if type(wdata) is str:
        in_data = bytes(string_to_list(wdata))
    elif type(wdata) is list:
        in_data = bytes(wdata)
    else:
        in_data = wdata

    #print("Write to file: " + path)
    f = open(path, 'wb')
    f.write(in_data)
    f.close()


def calc_digest(file_data, hash_type="sha256"):
    if type(file_data) is list:
        in_data = bytes(file_data)
    elif type(file_data) is str :
        in_data = bytes(string_to_list(file_data))
    else:
        in_data = file_data

    if hash_type == "sha256":
        x = hashlib.sha256()
    elif hash_type == "sha512":
        x = hashlib.sha512()
    else:
        x = hashlib.sha256()

    x.update(in_data)
    dig = x.digest()

    return list(dig)



