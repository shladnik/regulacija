#!/usr/bin/python3

import os
import subprocess
import pickle
from datetime import datetime

import objdump
import xml_parse

defines = {
#  "F_CPU" : "8000000UL",
  "F_CPU" : "9216000",
  "PLAIN_CONSOLE" : 0,
  "BAUD" : 230400,
#  "BAUD" : 9600,
  "__ASSERT_USE_STDERR" : "", # I have my own assert right now anyway
#  "NDEBUG" : "",
}

cflags = (
#"-g3",
#"-ggdb3",
#"-gstabs+",
#"-gdwarf-2",
"-Os",
"-mmcu=atmega32",
"-std=gnu99",

"-Wall",
"-Wextra",
"-pedantic",

"-mcall-prologues",
"-fshort-enums",
"-combine",
"-fwhole-program",
"-fno-split-wide-types",
"-funsigned-char",
#"-flto",


"-Wl,--relax",
"-Wl,--section-start=.bootloader=0x7000",
"-Wl,--section-start=.config=0x6000",

# throw out unneeded code - this seems to have no effect when -combine -fwhole-program is used
#"-fdata-sections",
#"-ffunction-sections",
#"-Wl,-gc-sections,-print-gc-sections",

# select printf
#"-Wl,-u,vfprintf", "-lprintf_min",
#"-Wl,-u,vfprintf", "-lprintf_flt",
)

#
# Includes
# 
includes = [
# STDLIB
"stdlib.h",
"stdio.h",
"string.h",
"stdbool.h",
"setjmp.h",

# AVR
"avr/io.h",
"avr/interrupt.h",
"avr/wdt.h",
"avr/eeprom.h",
"avr/boot.h",
"avr/pgmspace.h",
"util/delay.h",
"util/atomic.h",
"util/crc16.h",

# custom
"src/fifo.h",
]

sources = (
"src/timer.c",
"src/main.c",
"src/sch.c",
"src/timer_q.c",
"src/uart.c",
"src/crc8.c",
"src/onewire.c",
"src/ds18b20.c",
"src/relay.c",
"src/port.c",
"src/lcd.c",
"src/valve.c",
"src/loops.c",
"src/stack_check.c",
"src/watchdog.c",
"src/radiator.c",
"src/furnace.c",
"src/pumping.c",
"src/collector.c",
"src/console.c",
"src/print.c",
"src/exexec.c",
"src/config.c",
"src/flash.c",
)

for i in sources:
  if os.path.isfile(i[0:len(i)-1] + "h"):
    includes[len(includes):] = [ i[0:len(i)-1] + "h" ]

def compile(dirn):
  # Generate additional sources from XML files
  xml_parse.gen_c()
  
  # Common part
  base_cmd = [ "avr-gcc", "-I", "src/auto" ]
  for i in includes:
    base_cmd[len(base_cmd):] = [ "-include", i ]

  for define in defines:
    base_cmd[len(base_cmd):] = [ "-D" + define + "=" + str(defines[define]) ]
  base_cmd[len(base_cmd):] = cflags
  
  # Compile
  objn = dirn + "/obj.obj"
  c = subprocess.Popen(base_cmd + list(sources) + ["-o", objn])
  c.communicate()
  if c.returncode: return c.returncode
  
  ### Pre-linking stage
  #noln = dirn + "/nol"
  #s = subprocess.Popen(base_cmd + list(sources) + ["-c", "-o", noln])
  #s.communicate()
  #if s.returncode: return s.returncode
  
  ## Pre-assembly stage
  #sn   = dirn + "/ss"
  #s = subprocess.Popen(base_cmd + list(sources) + ["-S", "-o", sn])
  #s.communicate()
  #if s.returncode: return s.returncode

  ## Defines
  #concat = dirn + "concat.c"
  #cc = open(concat, "w")
  #subprocess.Popen(["cat"] + list(sources), stdout = cc)
  #cc.close()
  #defn = dirn + "/defs"
  #d = subprocess.Popen(base_cmd + ["-dM", "-E", concat, "-o", defn])
  #d.communicate()
  #os.remove(concat)
  #if d.returncode: return d.returncode
  
  # obj -> bin
  binn = dirn + "/bin.bin"
  b = subprocess.Popen(["avr-objcopy", "-R", ".eeprom", "-O", "binary", objn, binn]) 
  b.communicate()
  return b.returncode


#stamp = datetime.strftime(datetime.today(), "%Y%m%d%H%M%S%f")
#folder = stamp
#os.mkdir(folder)
#print(folder)
folder = "."



#def rlistdir(name = ""):
#  lname = name
#  if lname == "": lname = "./"
#  l = [os.path.join(name, x) for x in os.listdir(os.path.join(lname))]
#  fl = list(l)
#  for d in l:
#    if os.path.isdir(d):
#      fl += rlistdir(os.path.join(name, d))
#
#  return fl

#def rlistdir(name):
#  ignore = [
#    ".git",
#    "meta",
#    ".pyc",
#    "./py/server.py",
#  ]
#  l = [(os.path.getmtime(name + "/" + x), name + "/" + x) for x in os.listdir(name)]
#  for i in ignore:
#    try:
#      cp = list(l)
#      for e in cp:
#        print(e[1][-len(i):], i)
#        if e[1][-len(i)-1:] == i:
#          l.remove((os.path.getmtime(i), i))
#    except: pass
#  fl = list(l)
#  for d in l:
#    if os.path.isdir(d[1]):
#      fl += rlistdir(d[1])
#
#  fl.sort()
#  return fl

#l = rlistdir()
#print(l)
#l = l[l.index((os.path.getmtime("./bin.bin"), "./bin.bin"))+1:-1]
#if len(l) == 1:
#  print("Nothing changed - not recompiling.")
#else:
#  print("Changed files:", l)
cs = compile(folder)
if cs:
  print("Compiler error", cs)
  exit()

meta = open("meta", 'wb')
symbols = objdump.correct_symbols(objdump.get_symbols())
pickle.dump((defines, symbols), meta)
meta.close()

subprocess.Popen(["avr-size", "-A", folder + "/obj.obj"]).communicate()
if 1:
  subprocess.Popen(["scp", 
                    folder + "/bin.bin",
                    folder + "/meta",
                    "stefan@stefuc.homeip.net:~/regulacija/"]).communicate()
  subprocess.Popen(["scp", 
                    folder + "/xml/xml.xml",
                    "stefan@stefuc.homeip.net:~/regulacija/xml"]).communicate()
  subprocess.Popen(["scp", 
  #                  folder + "/py/server.py",
                    folder + "/py/gum.py",
                    folder + "/py/rs232.py",
                    "stefan@stefuc.homeip.net:~/regulacija/"]).communicate()
if 0:
  subprocess.Popen(["avrdude", "-c", "stk500v2", "-P", "/dev/ttyUSB0", "-p", "m32", "-U", "flash:w:bin.bin"]).communicate()
