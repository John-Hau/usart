#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Function to encode and decode slip packet

from libs.crc_calc import *
from libs.print_log import *
import random


class frame_slip:
    def __init__(self):
        self.END = 0xC0
        self.ESC = 0xDB
        self.ESC_END = 0xDC
        self.ESC_ESC = 0xDD

        self.rx_data = []

    def encode_frame(self, in_data_frame):
        if len(in_data_frame) == 0:
            return None

        if type(in_data_frame) is str:
            in_data = [ord(x) for x in in_data_frame]
        elif type(in_data_frame) is list:
            in_data = in_data_frame
        else:
            return None

        frame_data = []

        # CRC
        crc = crc16(in_data, 0)

        frame_data.append(self.END)
        for byte in in_data+crc:
            if byte == self.END:
                frame_data = frame_data + [self.ESC, self.ESC_END]
            elif byte == self.ESC:
                frame_data = frame_data + [self.ESC, self.ESC_ESC]
            else:
                frame_data.append(byte)

        frame_data.append(self.END)

        return frame_data

    def decode_frame(self, in_frame):
        if type(in_frame) is str:
            in_frame_list = [ord(x) for x in in_frame]
        elif type(in_frame) is list:
            in_frame_list = in_frame
        elif type(in_frame) is bytes:
            in_frame_list = [x for x in in_frame]
        else:
            return None

        payload = []
        self.rx_data = self.rx_data + in_frame_list
        if self.rx_data.count(self.END) == 0:
            # frame error
            self.rx_data = []
            return None
        elif self.rx_data.count(self.END) == 1:
            # frame not full
            return None
        else:
            # frame get
            first_end = self.rx_data.index(self.END)
            second_end = self.rx_data[first_end+1:].index(self.END) + first_end + 1

            encode_pack = self.rx_data[first_end+1:second_end]
            self.rx_data = self.rx_data[second_end+1:]
            # decode frame
            idx = 0
            while idx < len(encode_pack):
                if encode_pack[idx] == self.ESC:
                    if idx + 1 < len(encode_pack):
                        if encode_pack[idx+1] == self.ESC_ESC:
                            payload.append(self.ESC)
                            idx += 1
                        elif encode_pack[idx+1] == self.ESC_END:
                            payload.append(self.END)
                            idx += 1
                        else:
                            payload.append(encode_pack[idx])
                    else:
                        payload.append(encode_pack[idx])
                else:
                    payload.append(encode_pack[idx])
                idx += 1

            if len(payload) <= 2:
                return None

            # check crc
            crc_get = payload[-2:]
            crc_chk = crc16(payload[0:-2], 0)
            if crc_chk != crc_get:
                return None

        return payload[0:-2]


def slip_test():
    frames = frame_slip()
    for data_len in range(11, 1000):
        org_data = [random.randint(0, 0xff) for x in range(data_len)]
        print_hex("origin data:", org_data)
        encode_data = frames.encode_frame(org_data)
        print_hex("frame data:", encode_data)
        decode_data = frames.decode_frame(encode_data)
        print_hex("frame data:", decode_data)
        if decode_data == org_data:
            print_line("ok")
        else:
            print_line("fail")
            break


# Run main.
if __name__ == "__main__":
    slip_test()




