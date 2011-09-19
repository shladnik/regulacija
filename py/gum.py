#!/usr/bin/python3

import time
import pickle
import threading

import rs232
import tools


def read_symbol(name, shape=int):
  if symbols[name]['mem'] == 'flash':
    arr = read_flash(symbols[name]['adr'], bytearray(symbols[name]['size']))
  else:
    arr = rs232.access(0, symbols[name]['adr'], bytearray(symbols[name]['size']))

  if shape == int:
    return tools.to_int(arr)
  else:
    return arr

def write_symbol(name, dat):
  if (type(dat) == int): dat = tools.to_bytes(dat, symbols[name]['size'])
  if symbols[name]['size'] < len(dat): raise Exception("Write over the symbol length")
  if symbols[name]['mem'] == 'flash':
    write_flash(symbols[name]['adr'], dat)
  else:
    rs232.access(1, symbols[name]['adr'], dat)

def read_flash(adr, dat):
  length = min(len(dat[0:0xff]), symbols['flash_buf']['size'])
  exexec('flash_read', [ symbols['flash_buf']['adr'], adr, length ])
  arr = rs232.access(0, symbols['flash_buf']['adr'], dat[0:length])
  if len(dat) > length:
    arr.extend(write_flash(adr + length, dat[length:]))
  return arr

def write_flash(adr, dat):
  length = min(len(dat[0:0xff]), symbols['flash_buf']['size'])
  arr = rs232.access(1, symbols['flash_buf']['adr'], dat[0:length])
  exexec('flash_write', [ symbols['flash_buf']['adr'], adr, length ])
  if len(dat) > length:
    arr.extend(write_flash(adr + length, dat[length:]))
  return arr


#
# Flashing stuff
#

def flash(f):
  try:
    print("Entering bootloader...")
    exexec('bootjmp', block = False)
  except:
    print("Error... try if it's already running...")
    time.sleep(4.0)
    rs232.flush()
  
  if rs232.get(timeout = 10.0) != 0xa5: raise Exception("Failed to start bootloader")
  print("Bootloader started! Flashing...")
  
  with rs232.rs232.lock:
    start = time.clock()
 
    binary = bytearray(f.read())
    binary.reverse()
    size = len(binary)
    print("size:", size)
    if not (0 < size <= 32 * 1024): raise Exception("File size error")
 
    size_pac = tools.to_bytes(size, 2)
    size_pac.reverse()
    rs232.flash_put(size_pac)
    
    length = size % defines['SPM_PAGESIZE']
    if length == 0: length = defines['SPM_PAGESIZE']
 
    cnt = 0;
    while cnt < size:
      pac = binary[cnt:cnt+length] + bytearray([0xa5])
      rs232.flash_put(pac)
      cnt += length
      length = defines['SPM_PAGESIZE']
 
    print("Time elapsed:", time.clock() - start)
  
  if rs232.get() != 0xa5: raise Exception("Failed!")
  else:                   print("Succeed!")
  
  time.sleep(1.0)
  print("Reinitializing debuging stuff...")
  exexec('dbg_init')


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





lock = threading.Lock()

def exexec(func, arg = [0, 0, 0, 0], block = True):
  global lock
  lock.acquire()

  try:
    max_args = 4
    
    if len(arg) > max_args:
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
    if type(func) == int:
      write_symbol("exexec_func", func >> 1)
    else:
      write_symbol("exexec_func", symbols[func]['adr'] >> 1)
    
    if block:
      while read_symbol("exexec_func") != 0:
        time.sleep(0.05)

      return_bytes = read_symbol("exexec_buf", None)
      ret_val = []
      for i in range(0,len(return_bytes),2):
        ret_val.insert(0, return_bytes[i] + return_bytes[i+1] * 256)
      return ret_val
  finally:
    lock.release()




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



def set_time(t = time.localtime(), format = None):
  write_symbol('date', tools.construct_time(t, format))



def connect():
  build = ""
  rs232.start(230400)
  for rs232.rs232.baudrate in [ 230400, 115200, 9600, 2400 ]:
    try:
      print("BAUD:", rs232.rs232.baudrate)
      build = str(tools.to_int(rs232.access(0, 0x60, bytearray(8))))
      break
    except:
      continue
    print('Failed to connect!')
    exit()

  meta = open("meta." + build, 'rb')
  pickled = pickle.load(meta)
  global defines, symbols
  defines = pickled[0]
  symbols = pickled[1]

connect()
