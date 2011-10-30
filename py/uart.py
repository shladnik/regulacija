#!/usr/bin/python3

import serial
import threading
import queue
import time

uart = serial.Serial()

def get(timeout = 0.1): #1000 * 10.0/uart.baudrate):
  uart.timeout = timeout
  r = uart.read(1)
  if not r: raise Exception 
  r = r[0]
  return r

def put(b, delay = 0.0001):
  if type(b) != int:
    raise Exception("Cannot send more then a char at once.")
  #print("T:", hex(b))
  uart.write(bytes([b]))
  time.sleep(delay)

def reset(timeout = 1):
  timeout_orig = uart.timeout
  uart.timeout = timeout
  foo = uart.read(256)
  while foo:
    print(foo)
    foo = uart.read(256)
  print("Uart flushed.")
  uart.timeout = timeout_orig

#
# Flashing stuff
#

def flash_put(pac):
  if type(pac) == int: pac = bytes([pac])
  uart.write(pac)
  echo = uart.read(len(pac))
  if pac != echo:
    raise Exception("Flashing echo error (sent:" + str(pac) + " echo: " + str(echo))


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

def rx_nr(cnt, timeout = 0):
  while len(rx_bytes) < cnt:
    b = int()
    if cnt == 1:
      b = get(timeout=timeout)
    else:
      b = get()
    rx_bytes.append(b)

def receive(timeout):
  with access.lock:
    pac_len = 1
    rx_nr(pac_len, timeout)
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

def send(write, adr, dat):
  with access.lock:
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

def access(write, adr, dat, rty = 2):
  if write: max_len = max_write_len
  else:     max_len = 0xff

  if len(dat) == 0: raise Exception()
  adr &= 0xffff
  
  with access.lock:
    send(write, adr, dat[0:max_len])
    while rty >= 0:
      try:
        pac = receive(timeout = 0.5)
        if not (pac[0] == ~write & 0x1 and pac[1] == adr and len(pac[2]) == len(dat[0:max_len])):
          raise Exception("Wrong packet received", pac, adr, len(dat))
        else:
          break
      except Exception as inst:
        # clear things up
        reset()
        global rx_bytes
        rx_bytes = bytearray()
        print("Protocol error:", type(inst), inst)
        if rty:
          rty -= 1
          send(write, adr, dat[0:max_len])
        else:
          raise inst

    if len(dat) > max_len:
      pac[2].extend(access(write, adr + max_len, dat[max_len:]))
    return pac[2]
access.lock = threading.RLock()
