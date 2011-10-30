import os
import tarfile
import pickle
import gum

def pack(timestamp = None):
  files = (
    "fw.bin",
    "bootloader.bin",
    "meta",
    "xml/xml.xml",
    "py/gum.py",
    "py/tools.py",
    "py/uart.py",
    "py/html_tools.py",
    "py/packer.py",
    "py/server.py",
  )

  a = tarfile.open('update.tar.bz2', mode='w:bz2')
  for f in files:
    if not timestamp or os.stat(f).st_mtime > timestamp:
      a.add(f)

def unpack(f):
  if type(f) == str: f = tarfile.open(name = f, mode='r:bz2')
  else:              f = tarfile.open(fileobj = f)
  f.extractall()

def getfile(f, fn):
  if type(f) == str: f = tarfile.open(name = f, mode='r:bz2')
  else:              f = tarfile.open(fileobj = f)
  if fn in f.getnames(): return f.extractfile(fn)
  else                 : return None

def update(a):
  if type(a) == str: a = tarfile.open(a, mode='r:bz2')
  else:              a = tarfile.open(fileobj = a, mode='r|bz2')

  a.extractall()
  names = a.getnames()
  if "bootloader.bin" in names or "fw.bin" in names:
    gumi = gum.Gum()
    if "bootloader.bin" in names: gumi.flash_bootloader(open("bootloader.bin", 'rb'))
    if "fw.bin"         in names: gumi.flash_fw        (open("fw.bin"        , 'rb'))
