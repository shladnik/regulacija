#!/usr/bin/python3

import os, sys
import time
import pickle
import threading
import xml.etree.ElementTree as etree

import uart
import tools
import packer


def connect(port = None, rate = None, sw = None):
  ports = tuple(map(lambda x: '/dev/ttyUSB' + str(x), range(4)))
  if port:
    ports = filter(lambda x: x == port, ports)
    ports = ( port ) + tuple(ports)
  
  rates = ( 230400, 115200, 9600 )
  rates = rates + tuple(reversed(sorted(tuple(set(uart.uart.BAUDRATES) - set(rates)))))
  if rate:
    rates = filter(lambda x: x == rate, rates)
    rates = ( rate ) + tuple(rates)

  sws = filter(lambda x: x[0:6] == "update" and x[-8:] == ".tar.bz2", os.listdir("."))
  sws = sorted(sws, key=lambda x: os.stat(x).st_mtime)
  if sw:
    sws = filter(lambda x: x == sw, sws)
    sws = ( sw ) + tuple(sws)
  
  try:
    for uart.uart.port in ports:
      build = False
      try:
        for uart.uart.baudrate in rates:
          print("Trying", uart.uart.port, uart.uart.baudrate, "... ")
          uart.uart.open()
          try:
            build = uart.access(0, 0x60, bytearray(8))
            break;
          except:
            if uart.uart.baudrate == rates[-1]: raise
            else:                               continue
      except:
        if uart.uart.port == ports[-1]: raise
        else:                           continue
      if build: break
  except:
    print("Failed to autoconnect. Defaulting to the first listed ...")
    uart.uart.port     = ports[0]
    uart.uart.baudrate = rates[0]

  print("Device:", uart.uart.port, "- rate:", uart.uart.baudrate)
  
  if port and port != uart.uart.port:     print("Port changed (", port, "->", uart.uart.port    , ")")
  if rate and rate != uart.uart.baudrate: print("Rate changed (", rate, "->", uart.uart.baudrate, ")")
  
  #
  # Meta
  #
  def extract_build(meta):
    return bytearray([
      meta['macros']["BUILD_SEC"    ],
      meta['macros']["BUILD_MIN"    ],
      meta['macros']["BUILD_HOUR"   ],
      meta['macros']["BUILD_WEEKDAY"],
      meta['macros']["BUILD_DAY"    ],
      meta['macros']["BUILD_MONTH"  ]] +
      list(tools.to_bytes(meta['macros']["BUILD_YEAR"   ], 2)))

  latest_sw    = sws[0]
  latest_meta  = None
  latest_build = bytearray(8)

  global meta
  for s in sws:
    pickled_meta = packer.getfile(s, 'meta')
    if pickled_meta:
      meta = pickle.load(pickled_meta)
      this_build = extract_build(meta)
      if bytearray(reversed(this_build)) > bytearray(reversed(latest_build)):
        latest_sw    = s
        latest_meta  = meta
        latest_build = this_build
      if this_build == build: break
    if s == sws[-1]:
      if latest_meta:
        print("Failed to find appropriate meta (", build, "). Failing back to latest found.")
        meta = latest_meta
        s    = latest_sw
      else:
        raise Exception("No meta found")

  print("Meta:", tools.mcutime(extract_build(meta)))
  if sw and sw != s: print("SW changed (", sw, "->", s, ")")
  
  uart.max_write_len = meta['symbols']['rx_buf']['size']

def disconnect():
  uart.uart.close()

#
# R/W functions
#

def read_symbol(name, shape=int):
  if meta['symbols'][name]['mem'] == 'flash':
    arr = read_flash(meta['symbols'][name]['adr'], bytearray(meta['symbols'][name]['size']))
  else:
    arr = uart.access(0, meta['symbols'][name]['adr'], bytearray(meta['symbols'][name]['size']))

  if shape == int:
    return tools.to_int(arr)
  else:
    return arr

def read_symbol_dbgcp(name, shape=int):
  if meta['symbols'][name]['mem'] == 'ram' and \
     meta['symbols']['__dbg2cp_start']['adr'] <= meta['symbols'][name]['adr'] < meta['symbols']['__dbg2cp_end']['adr']:
    offset = meta['symbols']['__dbgcp_start']['adr'] - meta['symbols']['__dbg2cp_start']['adr']
    arr = uart.access(0, meta['symbols'][name]['adr'] + offset, bytearray(meta['symbols'][name]['size']))
  else:
    raise Exception("Not a dbgcp symbol.")

  if shape == int:
    return tools.to_int(arr)
  else:
    return arr

def write_symbol(name, dat):
  if (type(dat) == int): dat = tools.to_bytes(dat, meta['symbols'][name]['size'])
  if meta['symbols'][name]['size'] < len(dat): raise Exception("Write over the symbol length")
  if meta['symbols'][name]['mem'] == 'flash':
    write_flash(meta['symbols'][name]['adr'], dat)
  else:
    uart.access(1, meta['symbols'][name]['adr'], dat)

def read_flash(adr, dat):
  length = min(len(dat[0:0xff]), meta['symbols']['flash_buf']['size'])
  exexec('flash_read', [ meta['symbols']['flash_buf']['adr'], adr, length ])
  arr = uart.access(0, meta['symbols']['flash_buf']['adr'], dat[0:length])
  if len(dat) > length:
    arr.extend(write_flash(adr + length, dat[length:]))
  return arr

def write_flash(adr, dat):
  length = min(len(dat[0:0xff]), meta['symbols']['flash_buf']['size'])
  arr = uart.access(1, meta['symbols']['flash_buf']['adr'], dat[0:length])
  exexec('flash_write', [ meta['symbols']['flash_buf']['adr'], adr, length ])
  if len(dat) > length:
    arr.extend(write_flash(adr + length, dat[length:]))
  return arr


#
# Flashing stuff
#

def flash_fw(f):
  try:
    print("Entering bootloader...")
    exexec(meta['symbols']['__bootloader_adr']['adr'], block = False)
  except:
    print("Error... trying if it's already running...")
    uart.reset()
  
  print("Waiting for initial ACK ...", end=" ")
  if uart.get(timeout = 30.0) != 0xa5: raise Exception("Failed to start bootloader")
  print("got!")
  
  print("Bootloader started! Flashing...")
  
  start = time.clock()
 
  binary = bytearray(f.read())
  binary.reverse()
  size = len(binary)
  print("Size:", size)
  if not (0 < size <= 32 * 1024): raise Exception("File size error")
 
  size_pac = tools.to_bytes(size, 2)
  size_pac.reverse()
  uart.flash_put(size_pac)
  
  length = size % meta['macros']['SPM_PAGESIZE']
  if length == 0: length = meta['macros']['SPM_PAGESIZE']
 
  cnt = 0;
  while cnt < size:
    pac = binary[cnt:cnt+length] + bytearray([0xa5])
    uart.flash_put(pac)
    cnt += length
    length = meta['macros']['SPM_PAGESIZE']
 
  print("Time elapsed:", time.clock() - start)
  
  if uart.get() != 0xa5: raise Exception("Failed!")
  else:                   print("Succeed!")
  
  time.sleep(1.0)
  print("Reconnecting...")
  connect()
  print("Reinitializing debuging stuff...")
  exexec('debug_init')

def flash_bootloader(f):
  print("Flashing bootloader...")
  binary = bytearray(f.read())
  print("Size:", len(binary))
  for i in range(0, len(binary), meta['macros']['SPM_PAGESIZE']):
    page = binary[i:i+meta['macros']['SPM_PAGESIZE']]
    write_symbol('flash_buf', page)
    exexec('flash_write_block', [ meta['symbols']['flash_buf']['adr'], meta['symbols']['__bootloader_adr']['adr'] + i, len(page) ])
  print("Done!")


#
# Text console
#

def readcon(max_read=1.0):
  buf  = meta['symbols']["print_buf"]['adr']
  size = meta['symbols']["print_buf"]['size']
  nr = int(max_read * size)
  wp = read_symbol("print_buf_wp")
  rp = read_symbol("print_buf_rp")
  ovf = 0

  data = bytearray()
  if   wp > rp:
    nr = min(nr, wp - rp)
    data += uart.access(0, buf + rp, bytearray(nr))
  elif wp < rp:
    top    = min(nr, size - rp)
    bottom = min(nr - top, wp)
    nr = top + bottom
    data += uart.access(0, buf + rp, bytearray(top))
    if bottom > 0:
      data[len(data):] += uart.access(0, buf, bytearray(bottom))
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


#
# External executor
#

def exexec(func, arg = [0, 0, 0, 0], block = True):
  max_args = 4
  
  if len(arg) > max_args:
    raise Exception("To many arguments to exexec!")
  else:
    for i in range(max_args):
      if i < len(arg):
        if type(arg[i]) != int:
          if arg[i]: arg[i] = int(arg[i], 0)
          else:      arg[i] = 0
      else:
        arg.append(0)

  arg_buf = bytearray()
  for i in arg:
    arg_buf.insert(0, (i >> 8) & 0xff)
    arg_buf.insert(0, (i >> 0) & 0xff)

  with exexec.lock:
    while read_symbol("exexec_func") != 0:
      print("exexec busy!")
      time.sleep(1.0)

    write_symbol("exexec_buf", arg_buf)
    if type(func) != int: func = meta['symbols'][func]['adr']
    func >>= 1
    
    with uart.access.lock:
      write_symbol("exexec_func", func)
      
      if block:
        expected = ( 1, meta['symbols']['exexec_func']['adr'], bytearray([0, 0]) )
        try:
          response = uart.receive(timeout = 30)
          if response != expected: raise Exception("Exexec response failture.")
        except:
          response = read_symbol("exexec_func")
          if response != expected: raise Exception("Exexec response failback failture.")

    if block:
      return_bytes = read_symbol("exexec_buf", None)
      ret_val = []
      for i in range(0,len(return_bytes),2):
        ret_val.insert(0, return_bytes[i] + return_bytes[i+1] * 256)
      return ret_val
exexec.lock = threading.Lock()



def ds18b20_get_temp(sensor_list = None, resolution = 0, rty = 1):
  if sensor_list == None:
    sensor_list = etree.parse("xml/xml.xml").getroot().find("ds18b20_list").findall("ds18b20")

  arr = bytearray(len(sensor_list) * 2)
  for i in range(len(sensor_list)):
    arr[i * 2] = i

  uart.access(1, meta['symbols']['_end']['adr'], arr)
  exexec("ds18b20_get_temp_tab", [ len(sensor_list), resolution, rty, meta['symbols']['_end']['adr']])
  arr = uart.access(0, meta['symbols']['_end']['adr'], arr)
 
  for i in range(len(sensor_list)):
    v = tools.to_int(arr[2*i:2*i+2])
    if v >= 0x8000: v -= 0x10000
    v /= 256.0
    sensor_list[i] = v
  return sensor_list



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
