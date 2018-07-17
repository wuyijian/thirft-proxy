#!/usr/bin/python
#encoding=latin1

import socket
import struct
import hashlib
import sys
import time

def send_data(send_buf, s, log = False):
    #s = socket.socket(socket.AF_INET)
    #s.connect((ip, port))
    if log: print("-------------NEW CONNECTION--------------")
    send_len = s.send(send_buf)
    if log: print("send_len:", send_len)
    recv_buf = s.recv(4)
    while len(recv_buf) < 4:
        recv_buf += s.recv(4 - len(recv_buf))
    recv_len = struct.unpack("=L", recv_buf[:4])[0]
    while len(recv_buf) < recv_len:
        recv_buf += s.recv(recv_len - len(recv_buf))
    if log: print("recv_len:", recv_len)
    #s.close()
    return recv_buf

def pack(user_id,youxi_bi, trans_id):
    body_buf = struct.pack("=L16sBLLLL100s", game_id, "fuck\0", type, 0, 16, 1, 100, content)
    head_buf = struct.pack("=LLHLL", 18 + len(body_buf), 0, 18178, 0, user_id)
    #head_buf = struct.pack("=LLHLL", 18 + len(body_buf), 0, 1817, 0, user_id)
    return head_buf + body_buf


def unpack(recv_buf):
    if len(recv_buf) < 18: raise Exception()
    return struct.unpack("=LLHLL", recv_buf[:18])

if __name__ == '__main__':
    now = time.time()
    s = socket.socket(socket.AF_INET)
    s.connect(('127.0.0.1', 21146))
    for i in range(1, 2):
        user_id = 721018944
        game_id = 5
        type = 1
        #IP = '10.1.1.35'
        #game_id = 2
        content = '可惜花仙名字可以随便改，不然就是神ID'
        print unpack(send_data(pack(user_id, game_id, type), s, log=True))
        #time.sleep(1);
    s.close()
    print time.time() - now
