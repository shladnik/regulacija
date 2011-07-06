#!/usr/bin/python3

import serial
import threading
import queue
import time


rs232 = serial.Serial(port="/dev/ttyUSB0", baudrate=115200)
fifo = queue.Queue()

def loop():
  while 1:
    fifo.put(rs232.read(1)[0])

def get(timeout = 0.1): #1000 * 10.0/rs232.baudrate):
  return fifo.get(block=True, timeout=timeout)

def put(b):
  if type(b) != int:
    raise Exception("Cannot send more then a char at once.")
  rs232.write(bytes([b]))
  time.sleep(15 * 10.0/rs232.baudrate)

def start(baudrate):
  print("BAUD: ", baudrate)
  rs232.baudrate = baudrate

  readThread = threading.Thread(target=loop)
  readThread.daemon = True
  readThread.start()



#
# Protocol stuff
#

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
      b = get(timeout=0.5)
      #print("RX:%x" % b)
    else:
      b = get()
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
    put(b)

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
      #print(pac)
      #print("rx:", "%x" % pac[0], "%x" % pac[1], pac[2])
      if not (pac[0] == ~write & 0x1 and pac[1] == adr and len(pac[2]) == len(dat[0:0xff])):
        raise Exception("Wrong packet received", pac, adr, len(dat))
      else:
        break
    except Exception as inst:
      print("Protocol error:", type(inst), inst)
      time.sleep(5)
      if (type(inst) == queue.Empty):
        send(write, adr, dat[0:0xff])

  if len(dat) > 0xff:
    pac[2].extend(access(write, adr + 0xff, dat[0xff:]))
  return pac[2]


