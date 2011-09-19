import time

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


def construct_time(t = time.localtime(), format = None):
  if type(t) == str:
    if format == None: t = time.strptime(t)
    else:              t = time.strptime(t, format)
  date = bytearray()
  date[0:] += bytearray([ t.tm_sec ])
  date[1:] += bytearray([ t.tm_min ])
  date[2:] += bytearray([ t.tm_hour ])
  date[3:] += bytearray([ t.tm_wday ])
  date[4:] += bytearray([ t.tm_mday - 1 ])
  date[5:] += bytearray([ t.tm_mon  - 1 ])
  date[6:] += to_bytes(t.tm_year, 2)
  return date
