#!/usr/bin/env python
# -*- coding: utf-8 -*-


def print_line(strs):
    print(strs)


def print_line_no_break(strs):
    print(strs, end=' ')


def print_hex(strs, data):
    print(strs)

    if data is None:
        print("None")
        return

    idx = 0
    for byte in data:
        print("%02x " % byte, end=' ')
        idx += 1
        if idx == 16:
            idx = 0
            print("")

    print("")










