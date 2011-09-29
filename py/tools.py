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

class mcutime():
  def __init__(self, t = time.localtime(), format = None):
    if   type(t) == str:
      if format == None: t = time.strptime(t)
      else:              t = time.strptime(t, format)
    elif type(t) == bytearray:
      t = time.strptime("%d_%02d_%02d__%02d_%02d_%02d" % (to_int(t[6:8]), t[5] + 1, t[4] + 1, t[2], t[1], t[0]), "%Y_%m_%d__%H_%M_%S")
    self.t = t

  def get(self):
    date = bytearray()
    date[0:] += bytearray([ self.t.tm_sec ])
    date[1:] += bytearray([ self.t.tm_min ])
    date[2:] += bytearray([ self.t.tm_hour ])
    date[3:] += bytearray([ self.t.tm_wday ])
    date[4:] += bytearray([ self.t.tm_mday - 1 ])
    date[5:] += bytearray([ self.t.tm_mon  - 1 ])
    date[6:] += to_bytes(self.t.tm_year, 2)
    return date

  def __str__(self):
    return "%d_%02d_%02d__%02d_%02d_%02d" % (self.t.tm_year, self.t.tm_mon, self.t.tm_mday, self.t.tm_hour, self.t.tm_min, self.t.tm_sec)
    
