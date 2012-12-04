#/usr/bin/python

import os
import sys
import subprocess
import tempfile

cmd = [ 'avrdude', '-c', 'stk500v2', '-P', '/dev/ttyUSB0', '-p', 'm328p', '-U' ]
fuse_list = [ 'efuse', 'hfuse', 'lfuse', 'lock' ]
fdir = './fuse/'

def uC_op(f, fn, op):
  tmp = tempfile.TemporaryFile() 
  if subprocess.call(cmd + [ f + ':' + op + ':' + fn + ':r' ], stdout=tmp, stderr=subprocess.STDOUT):
    tmp.seek(0)
    print(tmp.read().decode())
    raise Exception()

def fl_read(f):
  try:
    with open(fdir + f, 'rb') as fuse_file:
      val = hex(int.from_bytes(fuse_file.read(), 'big'))
  except IOError:
    val = str(None)
  return val

def backup():
  try: os.mkdir(fdir)
  except: pass
  for f in fuse_list:
    uC_op(f, fdir + f, 'r')
  
def restore():
  for f in fuse_list:
    uC_op(f, fdir + f, 'w')
 
if len(sys.argv) > 1:
  eval(sys.argv[1] + '()')
else:
  print("FUSE\tBACKUP\tuC")
  
  for f in fuse_list:
    
    fn = f + '.uC' 
    uC_op(f, fn, 'r')
    with open(fn, 'rb') as fd:
      val_uC = hex(int.from_bytes(fd.read(), 'big'))
    os.remove(fn)

    val_fl = fl_read(f)

    print("{}\t{}\t{}".format(f, val_fl, val_uC))
