#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Message based on serial port

import queue
import threading
import time
import serial
import serial.tools.list_ports
import easygui
from libs.print_log import *
from libs.frame_slip import *


class serial_msg():
    def __init__(self, name, dbg=False):
        self.name = name
        self.frames = frame_slip()
        self.snd_queue = queue.Queue()
        self.snd_queueLock = threading.Lock()
        self.recv_queue = queue.Queue()
        #self.status_queue = queue.Queue()
        self.recv_queueLock = threading.Lock()
        self.serial_port = None
        self.serial_port_lock = threading.Lock()
        self.running = False
        self.debug = dbg
        self.tx_thread = self.Txhandler(self)
        self.rx_thread = self.Rxhandler(self)

    def send_msg(self, msg):
        if self.debug:
            print_line(msg)
        self.snd_queueLock.acquire()
        self.snd_queue.put(msg)
        self.snd_queueLock.release()
        self.tx_thread.resume()

    def get_msg(self):
        if self.recv_queue.qsize() > 0:
            self.recv_queueLock.acquire()
            msg = self.recv_queue.get()
            self.recv_queueLock.release()
            return msg
        else:
            return None

    def has_msg(self):
        if self.recv_queue.qsize() > 0:
            return True
        else:
            return False

    @property
    def dbg(self):
        return self.debug

    @dbg.setter
    def dbg(self, status):
        self.debug = status

    def open(self, port):
        if self.running:
            # already running
            return

        self.running = True
        self.serial_port = serial.Serial()
        self.serial_port.port = port
        self.serial_port.baudrate = 115200
        self.serial_port.open()
        if self.debug:
            print_line("connect to:" + port)

        self.tx_thread.start()
        self.rx_thread.start()

    def close(self):
        time.sleep(2)
        self.running = False

        if self.tx_thread is not None:
            self.tx_thread.resume()
            self.tx_thread.join()
            self.tx_thread = None
        if self.rx_thread is not None:
            self.rx_thread.join()
            self.rx_thread = None

        if self.serial_port is not None:
            self.serial_port.close()
            self.serial_port = None

    class Txhandler(threading.Thread):
        def __init__(self, parent):
            threading.Thread.__init__(self)
            self.upper = parent
            self.event = threading.Event()
            self.event.clear()
            self.name = "Txhandler"

        def resume(self):
            self.event.set()

        def run(self):
            print_line("thread start：" + self.name)
            while self.upper.running or self.upper.snd_queue.qsize():
                if self.upper.snd_queue.qsize() > 0:
                    self.upper.snd_queueLock.acquire()
                    msg = self.upper.snd_queue.get()
                    self.upper.snd_queueLock.release()
                    if self.upper.debug:
                        print_line("send:")
                        print(msg)

                    #encode_data = self.upper.frames.encode_frame(msg)
                    encode_data = msg
                    if self.upper.debug:
                        print_hex("send encode:",  encode_data)

                    try:
                        self.upper.serial_port_lock.acquire()
                        #self.upper.serial_port.write(encode_data)

                        # send Head then PDU for RT1062
                        self.upper.serial_port.write(encode_data[0:4])
                        if len(encode_data[4:]) > 0:
                            time.sleep(0.001)
                            self.upper.serial_port.write(encode_data[4:])

                        self.upper.serial_port_lock.release()
                    except Exception as ex:
                        print_line("write error!")

                    self.event.clear()
                    time.sleep(0.001)
                else:
                    self.event.wait()
            print_line("tx exit")

    class Rxhandler(threading.Thread):
        def __init__(self, parent):
            threading.Thread.__init__(self)
            self.upper = parent
            self.name = "Rxhandler"

        def run(self):
            print_line("thread start：" + self.name)
            recv_data = []
            while self.upper.running or self.upper.serial_port.in_waiting:
                data_pending = self.upper.serial_port.in_waiting
                if data_pending > 0:
                    self.upper.serial_port_lock.acquire()
                    recv_data += self.upper.serial_port.read(data_pending)
                    self.upper.serial_port_lock.release()
                    if self.upper.debug:
                        print_hex("recv:", recv_data)

                #decode_data = self.upper.frames.decode_frame(recv_data)
                decode_data = recv_data
                if decode_data is not None and len(decode_data) > 3:
                    if self.upper.debug:
                        print_hex("recv decode:", decode_data)

                    if len(decode_data) and len(decode_data) >= (4 + decode_data[2] + decode_data[3]*256):
                        self.upper.recv_queueLock.acquire()
                        self.upper.recv_queue.put(decode_data)
                        self.upper.recv_queueLock.release()
                        recv_data = []

                time.sleep(0.01)
            print_line("rx exit")


def choose_serial_port():
    port_list = list(serial.tools.list_ports.comports())
    if len(port_list) == 0:
        print('no serial port')
        return None
    else:
        show_port = []
        for obj in port_list:
            show_port.append(obj.description)

        choice = easygui.choicebox(msg="choose serial port", title="Serial", choices=show_port)
        if choice is None:
            return None
        else:
            choice_port = show_port.index(choice)

    return port_list[choice_port].device


def msg_test():
    choose_port = choose_serial_port()
    if choose_port is None:
        return

    msg_task = serial_msg("serial msg", dbg=True)
    msg_task.open(choose_port)
    msg_task.send_msg("test")
    msg_task.close()


# Run main.
if __name__ == "__main__":
    msg_test()


