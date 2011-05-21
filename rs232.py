#!/usr/bin/python3

import serial
import threading
import queue
import time

rs232 = serial.Serial(port="/dev/ttyUSB1", baudrate=115200)
fifo = queue.Queue()

def loop():
  while 1:
    fifo.put(rs232.read(1)[0])

def get(timeout = 1000 * 10.0/rs232.baudrate):
  return fifo.get(block=True, timeout=timeout)

prev = 0
dly_max = 0

def put(b):
  if type(b) == type(1):
    rs232.write(bytes([b]))
  else:
    rs232.write(b)
  # let it breath
  #time.sleep(1 * 10.0/rs232.baudrate)
  global prev, dly_max
  now = time.clock()
  dly = now - prev;
  if prev and dly > dly_max:
    dly_max = dly;
    print("\n\nMAX delay:", dly_max)
  prev = now

def start(baudrate):
  print("BAUD: ", baudrate)
  rs232.baudrate = baudrate

  readThread = threading.Thread(target=loop)
  readThread.daemon = True
  readThread.start()
