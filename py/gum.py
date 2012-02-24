#!/usr/bin/python3

import os, sys
import time
import pickle
import threading
import xml.etree.ElementTree as etree

import uart
import tools
import packer

class Gum():
  icnt = 0
  icnt_lock = threading.Lock()

  def __init__(self, port = None, rate = None, meta = None, sw = None):
    with Gum.icnt_lock:
      if not Gum.icnt:
        self.connect(port = port, rate = rate, meta = meta, sw = sw)
      self.meta = Gum.meta
      Gum.icnt += 1

  def connect(self, port = None, rate = None, meta = None, sw = None):
    try:
      unpickled = pickle.load(open("gum.cache", 'rb'))
    except: pass
    else:
      if not port: port = unpickled['port']
      if not rate: rate = unpickled['rate']
      if not meta: meta = unpickled['meta']

    ports = tuple(map(lambda x: '/dev/ttyUSB' + str(x), range(4)))
    if port:
      ports = filter(lambda x: x != port, ports)
      ports = tuple([port]) + tuple(ports)
    ports = tuple(filter(os.path.exists, ports))
    if not ports: raise Exception("No port found!")
    
    rates = ( 230400, 115200, 9600 )
    #rates = rates + tuple(reversed(sorted(tuple(set(uart.uart.BAUDRATES) - set(rates)))))
    if rate:
      rates = filter(lambda x: x != rate, rates)
      rates = tuple([rate]) + tuple(rates)
    
    try:
      for uart.uart.port in ports:
        build = False
        try:
          for uart.uart.baudrate in rates:
            print("Trying", uart.uart.port, uart.uart.baudrate, "... ")
            uart.uart.open()
            uart.reset(timeout = 0.02)
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
    except Exception as inst:
      print("Failed to autoconnect:", inst)
      print("Defaulting to the first listed ...")
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
 
    sws = filter(lambda x: x[0:6] == "update" and x[-8:] == ".tar.bz2", os.listdir("."))
    sws = sorted(sws, key=lambda x: os.stat(x).st_mtime)
    if sw:
      sws = filter(lambda x: x == sw, sws)
      sws = tuple([sw]) + tuple(sws)
    
    def sw2meta(sw):
      pickled_meta = packer.getfile(sw, 'meta')
      if pickled_meta: return pickle.load(pickled_meta)
      else           : return None
    
    meta_latest  = None
    build_latest = bytearray(8)
    metafs = tuple([lambda: meta]) + tuple(map(lambda x: lambda: sw2meta(x), sws))
    for metaf in metafs:
      meta = metaf()
      if meta:
        build_curr = extract_build(meta)
        if meta and build_curr == build:
          break
        elif bytearray(reversed(build_curr)) > bytearray(reversed(build_latest)):
          meta_latest  = meta
          build_latest = build_curr
      if metaf == metafs[-1]:
        if not meta_latest: raise Exception("No meta found")
        print("Failed to find appropriate meta (", tools.mcutime(build), "). Failing back to latest found (", tools.mcutime(build_latest), ").")
        meta  = meta_latest
        build = build_latest

    print("Meta build:", tools.mcutime(build))
      
    uart.max_write_len = meta['symbols']['rx_buf']['size']
    Gum.meta = meta

  def __del__(self):
    with Gum.icnt_lock:
      if hasattr(self, 'meta'):
        Gum.icnt -= 1
        if not Gum.icnt:
          to_pickle = { 'port' : uart.uart.port,
                        'rate' : uart.uart.baudrate,
                        'meta' : self.meta,
                      }
          pickle.dump(to_pickle, open("gum.cache", 'wb'))
          uart.uart.close()

  #
  # R/W functions
  #

  def read_symbol(self, name, shape=int):
    if self.meta['symbols'][name]['mem'] == 'flash':
      arr = self.read_flash(self.meta['symbols'][name]['adr'], bytearray(self.meta['symbols'][name]['size']))
    else:
      arr = uart.access(0, self.meta['symbols'][name]['adr'], bytearray(self.meta['symbols'][name]['size']))
  
    if shape == int:
      return tools.to_int(arr)
    else:
      return arr
  
  def read_symbol_dbgcp(self, name, shape=int):
    if self.meta['symbols'][name]['mem'] == 'ram' and \
       self.meta['symbols']['__dbg2cp_start']['adr'] <= self.meta['symbols'][name]['adr'] < self.meta['symbols']['__dbg2cp_end']['adr']:
      offset = self.meta['symbols']['__dbgcp_start']['adr'] - self.meta['symbols']['__dbg2cp_start']['adr']
      arr = uart.access(0, self.meta['symbols'][name]['adr'] + offset, bytearray(self.meta['symbols'][name]['size']))
    else:
      raise Exception("Not a dbgcp symbol.")
  
    if shape == int:
      return tools.to_int(arr)
    else:
      return arr
  
  def write_symbol(self, name, dat):
    if (type(dat) == int): dat = tools.to_bytes(dat, self.meta['symbols'][name]['size'])
    if self.meta['symbols'][name]['size'] < len(dat): raise Exception("Write over the symbol length")
    if self.meta['symbols'][name]['mem'] == 'flash':
      self.write_flash(self.meta['symbols'][name]['adr'], dat)
    else:
      uart.access(1, self.meta['symbols'][name]['adr'], dat)
  
  def read_flash(self, adr, dat):
    length = min(len(dat[0:0xff]), self.meta['symbols']['flash_buf']['size'])
    self.exexec('flash_read', [ self.meta['symbols']['flash_buf']['adr'], adr, length ])
    arr = uart.access(0, self.meta['symbols']['flash_buf']['adr'], dat[0:length])
    if len(dat) > length:
      arr.extend(self.rite_flash(adr + length, dat[length:]))
    return arr
  
  def write_flash(self, adr, dat):
    length = min(len(dat[0:0xff]), self.meta['symbols']['flash_buf']['size'])
    arr = uart.access(1, self.meta['symbols']['flash_buf']['adr'], dat[0:length])
    self.exexec('flash_write', [ self.meta['symbols']['flash_buf']['adr'], adr, length ])
    if len(dat) > length:
      arr.extend(self.write_flash(adr + length, dat[length:]))
    return arr
  
  
  #
  # Flashing stuff
  #
  
  def flash_fw(self, f):
    print("Saving config ... ")
    conf = self.get_config()
    print("done.")

    try:
      print("Executing bootloader ... ")
      self.exexec(self.meta['symbols']['__bootloader_adr']['adr'], block = False)
      print("done.")
    except:
      print("Error. Maybe already running.")
      uart.reset()
    
    print("Waiting for initial ACK ...")
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
    
    length = size % self.meta['macros']['SPM_PAGESIZE']
    if length == 0: length = self.meta['macros']['SPM_PAGESIZE']
   
    cnt = 0;
    while cnt < size:
      pac = binary[cnt:cnt+length] + bytearray([0xa5])
      uart.flash_put(pac)
      cnt += length
      length = self.meta['macros']['SPM_PAGESIZE']
   
    print("Time elapsed:", time.clock() - start)
    
    if uart.get() != 0xa5: raise Exception("Failed!")
    else:                  print("Succeed!")
    
    time.sleep(5.0)
    print("Reconnecting...")
    self.connect()

    print("Restoring config ... ")
    self.set_config(conf)
    print("done.")

    print("Setting time ... ")
    self.set_time()
    print("done.")
  
  def flash_bootloader(self, f):
    print("Flashing bootloader...")
    binary = bytearray(f.read())
    print("Size:", len(binary))
    for i in range(0, len(binary), self.meta['macros']['SPM_PAGESIZE']):
      page = binary[i:i+self.meta['macros']['SPM_PAGESIZE']]
      self.write_symbol('flash_buf', page)
      self.exexec('flash_write_block', [ self.meta['symbols']['flash_buf']['adr'], self.meta['symbols']['__bootloader_adr']['adr'] + i, len(page) ])
    print("Done!")
  
  
  #
  # Text console
  #
  
  def readcon(self, max_read=1.0):
    buf  = self.meta['symbols']["print_buf"]['adr']
    size = self.meta['symbols']["print_buf"]['size']
    nr = int(max_read * size)
    wp = self.read_symbol("print_buf_wp")
    rp = self.read_symbol("print_buf_rp")
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
      self.write_symbol("print_buf_rp", rp)
   
    ovf = self.read_symbol("print_buf_ovf")
    wp  = self.read_symbol("print_buf_wp")
    
    ret_val = data.decode('utf8', 'ignore')
  
    if wp == rp and ovf != 0:
      self.write_symbol("print_buf_ovf", 0)
      ret_val += "\n\n### LOST >=" + str(ovf) + "CHARACTERS ###\n\n"
  
    return ret_val
  
  
  #
  # External executor
  #
  
  def exexec(self, func, arg = [0, 0, 0, 0], block = True):
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
  
    with self.exexec.lock:
      while self.read_symbol("exexec_func") != 0:
        print("exexec busy with", hex(self.read_symbol("exexec_func")))
        time.sleep(1.0)
  
      self.write_symbol("exexec_buf", arg_buf)
      if type(func) != int: func = self.meta['symbols'][func]['adr']
      func >>= 1
      
      with uart.access.lock:
        self.write_symbol("exexec_func", func)
        
        if block:
          expected = ( 1, self.meta['symbols']['exexec_func']['adr'], bytearray([0, 0]) )
          response = None
          try:
            response = uart.receive(timeout = 15)
            if response != expected: raise Exception("Exexec response failture (expected %s, got %s)." % (str(expected), str(response)))
          except:
            if self.read_symbol("exexec_func") != 0: raise Exception("Exexec response failback failture (expected %s, got %s)." % (str(expected), str(response)))
  
      if block:
        return_bytes = self.read_symbol("exexec_buf", None)
        ret_val = []
        for i in range(0,len(return_bytes),2):
          ret_val.insert(0, return_bytes[i] + return_bytes[i+1] * 256)
        return ret_val
  exexec.lock = threading.Lock()
  
  
  
  def ds18b20_get_temp(self, sensor_list = None, resolution = 0, rty = 1):
    if sensor_list == None:
      sensor_list = etree.parse("xml/xml.xml").getroot().find("ds18b20_list").findall("ds18b20")
  
    arr = bytearray(len(sensor_list) * 2)
    for i in range(len(sensor_list)):
      arr[i * 2] = i
  
    uart.access(1, self.meta['symbols']['_end']['adr'], arr)
    self.exexec("ds18b20_get_temp_tab", [ len(sensor_list), resolution, rty, self.meta['symbols']['_end']['adr']])
    arr = uart.access(0, self.meta['symbols']['_end']['adr'], arr)
   
    for i in range(len(sensor_list)):
      v = tools.to_int(arr[2*i:2*i+2])
      if v >= 0x8000: v -= 0x10000
      v /= 256.0
      sensor_list[i] = v
    return sensor_list
  
  
  
  def valve_state(self, i):
    return self.exexec("valve_get", [ i ])[0]
  
  def relay_get(self, i):
    return self.exexec("relay_get", [ i ])[0]
  
  
  
  def set_time(self, t = None, format = None):
    if t == None: t = time.localtime()
    self.write_symbol('date', tools.mcutime(t, format).get())

  def is_config(self, name):
    syms = self.meta['symbols']
    return syms['__config_start']['adr'] <= syms[name]['adr'] < syms['__config_end']['adr'] and syms[name]['size']

  def is_mem(self, name):
    syms = self.meta['symbols']
    return syms[name]['mem'] == 'ram' and syms[name]['size']

  def get_config(self):
    conf = [ (x, self.read_symbol(x)) for x in self.meta['symbols'] if self.is_config(x) ]
    for i in conf: print(i[0], hex(i[1]))
    return conf

  def set_config(self, conf):
    for i in conf:
      if i[0] in self.meta['symbols']:
        print(i[0], hex(i[1]))
        self.write_symbol(i[0], i[1])
      else:
        print("Skipping:", i[0], hex(i[1]))

