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

def read_symbol(name, shape=int):
  arr = rs232.access(0, symbols[name]['adr'], bytearray(symbols[name]['size']))
  if shape == int:
    return to_int(arr)
  else:
    return arr

def write_symbol(name, dat):
  if (type(dat) == int): dat = to_bytes(dat, symbols[name]['size'])
  rs232.access(1, symbols[name]['adr'], dat)

def to_int(arr):
  int_val = 0
  for i in range(len(arr)):
    int_val |= arr[i] << i * 8
  return int_val

def to_bytes(int_val, size):
  arr = bytearray(size)
  for i in range(size):
    arr[i] = (int_val >> (8 * i)) & 0xff
  return arr


def readcon(max_read=1.0):
  buf  = symbols["print_buf"]['adr']
  size = symbols["print_buf"]['size']
  nr = int(max_read * size)
  wp = read_symbol("print_buf_wp")
  rp = read_symbol("print_buf_rp")
  ovf = 0

  data = bytearray()
  if   wp > rp:
    nr = min(nr, wp - rp)
    data += rs232.access(0, buf + rp, bytearray(nr))
  elif wp < rp:
    top    = min(nr, size - rp)
    bottom = min(nr - top, wp)
    nr = top + bottom
    data += rs232.access(0, buf + rp, bytearray(top))
    if bottom > 0:
      data[len(data):] += rs232.access(0, buf, bytearray(bottom))
  else:
    nr = 0

  if nr:
    rp += nr
    if rp >= size: rp -= size
    write_symbol("print_buf_rp", rp)
 
  ovf = read_symbol("print_buf_ovf")
  wp  = read_symbol("print_buf_wp")
  
  ret_val = data.decode('utf8', 'ignore')

  if wp == rp and ovf != 0:
    write_symbol("print_buf_ovf", 0)
    ret_val += "\n\n### LOST >=" + str(ovf) + "CHARACTERS ###\n\n"

  return ret_val



def exexec(func, arg):
  max_args = 4
  
  if arg == None:
    arg = [ 0, 0, 0, 0 ]
  elif len(arg) > max_args:
    raise Exception("To many arguments to exexec!")
  else:
    for i in range(max_args):
      if i < len(arg):
        if type(arg[i]) != int:
          arg[i] = 0
      else:
        arg.append(0)

  arg_buf = bytearray()
  for i in arg:
    arg_buf.insert(0, (i >> 8) & 0xff)
    arg_buf.insert(0, (i >> 0) & 0xff)

  while read_symbol("exexec_func") != 0:
    print("exexec busy!")
    time.sleep(0.01)

  write_symbol("exexec_buf", arg_buf)
  write_symbol("exexec_func", symbols[func]['adr'])
  while read_symbol("exexec_func") != 0:
    time.sleep(0.01)

  return_bytes = read_symbol("exexec_buf", None)
  ret_val = []
  for i in range(0,len(return_bytes),2):
    ret_val.insert(0, return_bytes[i] + return_bytes[i+1] * 256)
  return ret_val

def ds18b20_get_temp(i, resolution = 0, rty = 1):
  val = exexec("ds18b20_get_temp", [ i, resolution, rty ])
  val = val[0]/256.0
  return val

def valve_state(i):
  if   exexec("valve_opened", [ i ])[0] & 0xff: return True
  elif exexec("valve_closed", [ i ])[0] & 0xff: return False
  else:
    state = exexec("valve_get", [ i ])
    return (state[0] << 16) + state[1]

def relay_get(i):
  return exexec("relay_get", [ i ])[0]



