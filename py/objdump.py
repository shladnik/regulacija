#!/usr/bin/python3

import subprocess

base_cmd = [ "avr-objdump", "-t", "obj.obj" ]
ram_sections = [ ".noinit", ".bss", ".data" ]
rom_sections = [ ".text" ]

def get_symbols(sections):
  symbol = {}
  cmd = list(base_cmd)
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
    if rom_sections.count(syms[i]['section']):
      syms[i]['adr'] >>= 1
    if ram_sections.count(syms[i]['section']):
      syms[i]['adr'] -= 0x800000

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
  symbol = correct_symbols(get_symbols(ram_sections + rom_sections))
  print("Found ", len(symbol), " symbols.")
  if len(sys.argv) > 1:
    print_symbols(correct_symbols(get_symbols(sys.argv[1:])), 1)
