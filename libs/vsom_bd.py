#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Generate NXP bd file

import time

from libs.common import *
import collections


class VsomBd:
    CMD_LOAD = 0x01
    CMD_ENABLE = 0x02
    CMD_ERASE = 0x03

    def __init__(self, file_name):
        self.file_name = file_name
        self.data_comment = "#This file is generate by FAT, do NOT modify!\r\n\r\n"
        self.data_source_cnt = 0
        self.data_source = collections.OrderedDict()
        self.data_section = {}
        self.file_data = None
        self.cmd_handler = {
            VsomBd.CMD_LOAD: self._add_cmd_load,
            VsomBd.CMD_ENABLE: self._add_cmd_enable,
            VsomBd.CMD_ERASE: self._add_cmd_erase,
        }

    def add_source(self, var_name, file_path):
        if file_path is None:
            # external
            self.data_source[var_name] = r"extern (" + str(self.data_source_cnt) + r")"
            self.data_source_cnt += 1
        else:
            self.data_source[var_name] = file_path

    def _add_cmd_load(self, item_data, section=0):
        if len(item_data) != 2:
            print("load data error!")
            return -1

        source = item_data[0]
        dest = item_data[1]
        if type(source) is int:
            self.data_section[section] += "\tload "
            self.data_section[section] += hex(source) + r" > " + hex(dest) + ";\r\n"
        elif type(source) is str:
            self.data_section[section] += "\tload "
            self.data_section[section] += source + r" > " + hex(dest) + ";\r\n"
        else:
            print("load source not support")
            return -1
        return 0

    def _add_cmd_enable(self, item_data, section=0):
        if len(item_data) != 2:
            print("enable data error!")
            return -1

        flash_type = item_data[0]
        dest = item_data[1]
        self.data_section[section] += "\tenable " + flash_type + r" " + hex(dest) + ";\r\n"

    def _add_cmd_erase(self, item_data, section=0):
        if len(item_data) != 2:
            print("enable data error!")
            return -1

        start = item_data[0]
        end = item_data[1]
        self.data_section[section] += "\terase " + hex(start) + r".." + hex(end) + ";\r\n"

    def add_section_item(self, comment, command, item_data, section=0):
        if comment is not None:
            self.data_section[section] += "\r\n\t# " + comment + "\r\n"

        self.cmd_handler[command](item_data, section)

    def init_section(self, section):
        self.data_section[section] = ""

    def construct_file_data(self):
        # add comment
        self.file_data = self.data_comment

        # add source
        self.file_data += "sources {\r\n"
        for k, v in self.data_source.items():
            self.file_data += "\t" + k + r" = " + v + ";\r\n"
        self.file_data += "}\r\n\r\n"

        # add section
        for k, v in self.data_section.items():
            self.file_data += "section (" + str(k) + ") {\r\n"
            self.file_data += v
            self.file_data += "\r\n}\r\n\r\n"

    def write_to_file(self):
        self.construct_file_data()

        file_writef(self.file_name, self.file_data)
        time.sleep(0.1)

