#!/usr/bin/python3

import time
import rs232
import pickle

meta = open("meta", 'rb')
pickled = pickle.load(meta)
defines = pickled[0]
symbols = pickled[1]

rs232.start(defines["BAUD"])

if defines["PLAIN_CONSOLE"]:
  # there is no point to go further
  while 1: print(bytes([gum.rs232.get(timeout = 60)]).decode('utf8', 'ignore'), end="")

def read_symbol(name):
  return rs232.access(0, symbols[name]['adr'], 
         bytearray(symbols[name]['size']))

def write_symbol(name, dat):
  if (type(dat) == int): dat = to_bytes(dat, symbols[name]['size'])
  rs232.access(1, symbols[name]['adr'], dat)

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

def read_int(name):
  return to_int(read_symbol(name))

def readcon(max_read=1.0):
  buf  = symbols["print_buf"]['adr']
  size = symbols["print_buf"]['size']
  nr = int(max_read * size)
  wp = read_int("print_buf_wp")
  rp = read_int("print_buf_rp")
  ovf = 0

  if   wp > rp:
    nr = min(nr, wp - rp)
    data = rs232.access(0, buf + rp, bytearray(nr))
  elif wp < rp:
    top    = min(nr, size - rp)
    bottom = min(nr - top, wp)
    nr = top + bottom
    data = rs232.access(0, buf + rp, bytearray(top))
    if bottom > 0:
      data[len(data):] = rs232.access(0, buf, bytearray(bottom))
  else:
    nr = 0

  if nr:
    rp += nr
    if rp >= size: rp -= size
    write_symbol("print_buf_rp", rp)
    print(data.decode('utf8', 'ignore'), end="")
 
  ovf = read_int("print_buf_ovf")
  wp  = read_int("print_buf_wp")
  
  if wp == rp and ovf != 0:
    write_symbol("print_buf_ovf", 0)
    print()
    print()
    print("### LOST >=", ovf, "CHARACTERS ###")
    print()


def exexec(func, arg):
  while read_int("exexec_func") != 0:
    print("exexec busy!")
    time.sleep(0.01)
  write_symbol("exexec_buf", arg)
  write_symbol("exexec_func", symbols[func]['adr'])
  while read_int("exexec_func") != 0:
    time.sleep(0.01)
  return read_symbol("exexec_buf")

def exexec_test():
  func_list = [
    "exexec_test_8",
    "exexec_test_16",
    "exexec_test_32",
    "exexec_test_64",
  ]
  for f in func_list:
    for b in exexec(f, [0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0]):
      print(hex(b), end=" ")
    print()

def exexec_ds18b20_get_temp(i, resolution = 0, rty = 1):
  val = exexec("ds18b20_get_temp", [ 0, 0, rty, 0, resolution, 0, i, 0 ])
  val = val[7] + val[6]/256.0
  return val




