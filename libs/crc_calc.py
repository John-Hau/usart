#!/usr/bin/env python
# -*- coding: utf-8 -*-


def crc16(x, invert):
    a = 0xFFFF
    b = 0xA001
    for byte in x:
        a ^= byte
        for i in range(8):
            last = a % 2
            a >>= 1
            if last == 1:
                a ^= b

    if invert:
        return [(a >> 8) & 0xff, a & 0xff]
    else:
        return [a & 0xff, (a >> 8) & 0xff]


