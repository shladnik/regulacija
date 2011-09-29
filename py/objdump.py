#!/usr/bin/python3

import subprocess
import os

base_cmd = [ "avr-objdump", "-t" ]
ram_sections = [ ".noinit", ".bss", ".data", ".dbg" ]
rom_sections = [ ".text", ".config" ]

def get_symbols(sections = [], objs = None):
  if not objs:
    objs = list(filter(lambda x: x[-4:] == ".obj" , os.listdir(".")))
  symbol = {}
  cmd = base_cmd + objs
  for s in sections:
    cmd[len(cmd):] = [ "-j", s ]
  p = subprocess.Popen(cmd, bufsize=-1, stdout=subprocess.PIPE)
  for line in p.stdout:
    try:
      tab_pos = -1
      for c in line:
        tab_pos += 1
        if chr(c) == '\t':
          break
      symbol[line[tab_pos+10:len(line)-1].decode()] = { 'adr': int(line[0:8], 16),
                                                      'flags': line[9:16].decode(),
                                                    'section': line[17:tab_pos].decode(),
                                                       'size': int(line[tab_pos+1:tab_pos+9], 16) }
    except Exception as inst:
      #print("Ignoring: ", line, inst)
      pass
  return symbol
    
def correct_symbols(syms):
  for i in syms:
    if syms[i]['adr'] < 0x800000:
      syms[i]['mem'] = 'flash'
    else:
      syms[i]['adr'] -= 0x800000
      syms[i]['mem'] = 'ram'

  original = syms.copy()
  for key in original:
    inx = key.find('.')
    if inx != -1:
      real_name = key[0:inx]
      cnt = 0
      for j in original:
        inx_j = j.find('.')
        real_name_j = j[0:inx_j]
        if inx == inx_j and real_name == real_name_j:
          cnt += 1
      if cnt == 1:
        del syms[key]
        syms[real_name] = original[key]

  return syms


def print_symbols(syms, min_size=0):
  for i in syms:
    if syms[i]['size'] >= min_size:
      print("%04x" % syms[i]['adr'], "%04x" % syms[i]['size'], i)
      #print("%04x" % syms[i]['adr'], "%04x" % syms[i]['size'], "%10s" % syms[i]['section'], i)

if __name__ == "__main__":
  import sys
  symbol = correct_symbols(get_symbols([]))
  print("Found ", len(symbol), " symbols.")
  if len(sys.argv) > 1:
    print_symbols(correct_symbols(get_symbols(sys.argv[1:])), 1)
  else:
    print_symbols(correct_symbols(get_symbols()), 1)
