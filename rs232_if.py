#!/usr/bin/python3

from datetime import datetime 
import sys
import time
import signal
import queue
import rs232
import pickle



def crc_update(crc, byte):
  if not 0 <= byte < 0x100: raise
  poly = 0x18

  for i in range(8):
    feedback_bit = (crc ^ byte) & 0x01
    if feedback_bit: crc = crc ^ poly
    crc = crc >> 1;
    if feedback_bit: crc = crc | 0x80;
    byte = byte >> 1;

  return crc

def calc_crc(bs):
  crc = 0
  for b in bs:
    crc = crc_update(crc, b)
  return crc

rx_bytes = bytearray()

def rx_nr(cnt):
  while len(rx_bytes) < cnt:
    b = int()
    if cnt == 1:
      b = rs232.get(timeout=0.5)
      #print("RX:%x" % b)
    else:
      b = rs232.get()
    #print("RX: %2x" % b)
    rx_bytes.append(b)

def receive():
  try:
    pac_len = 1
    rx_nr(pac_len)
    write   = (rx_bytes[0] >> 7) & 0x1
    adr_len = rx_bytes[0] & 0x7f
    if adr_len > 2: raise Exception("Address too long")
  
    pac_len += adr_len
    rx_nr(pac_len)
    cnt = 0
    adr = 0
    while cnt < adr_len:
      cnt += 1
      adr |= rx_bytes[cnt] << (8 * (adr_len - cnt))
  
    pac_len += 1
    rx_nr(pac_len)
    dat_len = rx_bytes[adr_len+1]
  
    if dat_len == 0: raise Exception("Data length zero")
   
    if write:
      dat_start = pac_len
      pac_len += dat_len
      rx_nr(pac_len)
      dat = rx_bytes[dat_start:pac_len]
    else:
      dat = bytearray(dat_len)
  
    pac_len += 1
    rx_nr(pac_len)
    if calc_crc(rx_bytes[0:pac_len]): raise Exception("CRC error")
 
    for i in range(pac_len): rx_bytes.pop(0)

    return (write, adr, dat)
  
  except Exception as inst:
    if len(rx_bytes):
      rx_bytes.pop(0)
    raise


def send(write, adr, dat):
  pac = bytearray()
  adr_len = 0
  while adr >= (1 << (8 * adr_len)):
    adr_len += 1
  pac.append((write << 7) | adr_len)
  while adr_len:
    adr_len -= 1
    pac.append((adr >> (8 * adr_len)) & 0xff)
  pac.append(len(dat))
  if write:
    for b in dat:
      pac.append(b)
  pac.append(calc_crc(pac))
  for b in pac:
    rs232.put(b)

def access(write, adr, dat):
  if len(dat) == 0: raise Exception()
  adr &= 0xffff
  #print("tx:", "%x" % write, "%x" % adr, dat)
  send(write, adr, dat[0:0xff])
  #pac = receive()
  #if not (pac[0] == ~write & 0x1 and pac[1] == adr and len(pac[2]) == len(dat[0:0xff])):
  #  raise Exception("Wrong packet received", pac, adr, len(dat))
  while 1: 
    try:
      pac = receive()
      #print("rx:", "%x" % pac[0], "%x" % pac[1], pac[2])
      if not (pac[0] == ~write & 0x1 and pac[1] == adr and len(pac[2]) == len(dat[0:0xff])):
        raise Exception("Wrong packet received", pac, adr, len(dat))
      else:
        break
    except Exception as inst:
      print("except:", type(inst), ":", inst, "!")
      if (type(inst) == queue.Empty):
        send(write, adr, dat[0:0xff])

  if len(dat) > 0xff:
    pac[2].extend(access(write, adr + 0xff, dat[0xff:]))
  return pac[2]










#
# TODO split to 2 files
#












meta = open("meta", 'rb')
pickled = pickle.load(meta)
defines = pickled[0]
symbols = pickled[1]


def name_correct(name):
  if name in symbols:
    return name
  else:
    found = []
    for key in symbols:
      if len(name) > len(key): continue
      ok = 1
      for i in range(len(name)):
        if name[i] != key[i]:
          ok = 0
          break
      if ok and key[len(name)] == '.':
        found.append(key)
  
    if len(found) == 1:
      return found[0]
    else:
      print("Symbols found: ", found)

def read_symbol(name):
  name = name_correct(name)
  return access(0, symbols[name]['adr'], 
              bytearray(symbols[name]['size']))

def write_symbol(name, dat):
  name = name_correct(name)
  if (type(dat) == int): dat = to_bytes(dat, symbols[name]['size'])
  access(1, symbols[name]['adr'], dat)

def to_int(arr):
  int_val = 0
  for i in range(len(arr)):
    #int_val |= arr[i] << (len(arr) - 1 - i) * 8
    int_val |= arr[i] << i * 8
  return int_val

def to_bytes(int_val, size):
  arr = bytearray(size)
  for i in range(size):
    arr[i] = (int_val >> (8 * i)) & 0xff
  return arr

buf    = symbols[name_correct("print_buf")]['adr']
size   = symbols[name_correct("print_buf")]['size']
wpn    = name_correct("print_buf_wp")
rpn    = name_correct("print_buf_rp")
ovfn   = name_correct("print_buf_ovf")
blockn = name_correct("print_buf_block")

def readcon_old():
  wp = to_int(read_symbol(wpn))
  rp = to_int(read_symbol(rpn))
  ovf = 0

  while wp != rp:
    if   wp > rp:
      data = access(0, buf + rp, bytearray(wp - rp))
    elif wp < rp:
      data = access(0, buf + rp, bytearray(size - rp))
      if wp > 0:
        data[len(data):] = access(0, buf, bytearray(wp))

    write_symbol(rpn, wp)
    rp = wp
    print(data.decode('utf8', 'ignore'), end="")
    ovf = to_int(read_symbol(ovfn))
    wp = to_int(read_symbol(wpn))
  
  if ovf:
    write_symbol(ovfn, 0)
    print()
    print()
    print("### LOST >=", ovf, "CHARACTERS ###")
    print()

def readcon(max_read=1.0):
  nr = int(max_read * size)
  wp = to_int(read_symbol(wpn))
  rp = to_int(read_symbol(rpn))
  ovf = 0

  if   wp > rp:
    nr = min(nr, wp - rp)
    data = access(0, buf + rp, bytearray(nr))
  elif wp < rp:
    top    = min(nr, size - rp)
    bottom = min(nr - top, wp)
    nr = top + bottom
    data = access(0, buf + rp, bytearray(top))
    if bottom > 0:
      data[len(data):] = access(0, buf, bytearray(bottom))
  else:
    nr = 0

  if nr:
    rp += nr
    if rp >= size: rp -= size
    write_symbol(rpn, rp)
    print(data.decode('utf8', 'ignore'), end="")
 
  ovf = to_int(read_symbol(ovfn))
  wp  = to_int(read_symbol(wpn))
  
  if wp == rp and ovf != 0:
    write_symbol(ovfn, 0)
    print()
    print()
    print("### LOST >=", ovf, "CHARACTERS ###")
    print()


ovf_cntn = name_correct("ovf_cnt")

rs232.start(defines["BAUD"])

if defines["PLAIN_CONSOLE"]:
  while 1:
    print(bytes([rs232.get(timeout = 60)]).decode('utf8', 'ignore'), end="")
else:
  while 1:
    try:
      readcon()
    except:
      raise
