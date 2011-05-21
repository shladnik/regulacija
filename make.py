#!/usr/bin/python3

import os
import subprocess
import objdump
import pickle
from datetime import datetime

defines = {
#  "F_CPU" : "8000000UL",
  "F_CPU" : "9216000UL",
  "PLAIN_CONSOLE" : 0,
#  "BAUD" : 230400,
  "BAUD" : 9600,
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
"avr/pgmspace.h",
"util/delay.h",
"util/atomic.h",
"util/crc16.h",

# custom
"src/fifo.h",
]

sources = (
"src/main.c",
"src/timer_q.c",
"src/timer.c",
"src/uart.c",
"src/crc8.c",
"src/onewire.c",
"src/ds18b20.c",
"src/relay.c",
"src/port.c",
"src/lcd.c",
"src/valve.c",
"src/sch.c",
"src/loops.c",
"src/stack_check.c",
"src/watchdog.c",
"src/radiator.c",
"src/furnace.c",
"src/pumping.c",
"src/collector.c",
"src/console.c",
"src/print.c",
#"src/test.c",
)

for i in sources:
  if os.path.isfile(i[0:len(i)-1] + "h"):
    includes[len(includes):] = [ i[0:len(i)-1] + "h" ]

def compile(dirn):
  objn = dirn + "/obj.obj"
  binn = dirn + "/bin.bin"
  noln = dirn + "/nol"
  sn   = dirn + "/ss"
  defn = dirn + "/defs"

  # Common part
  base_cmd = [ "avr-gcc" ]
  for i in includes:
    base_cmd[len(base_cmd):] = [ "-include", i ]
  for define in defines:
    base_cmd[len(base_cmd):] = [ "-D" + define + "=" + str(defines[define]) ]
  base_cmd[len(base_cmd):] = cflags
  
  # Compile
  c = subprocess.Popen(base_cmd + list(sources) + ["-o", objn])
  c.communicate()
  if c.returncode: return c.returncode
  
  ### Pre-linking stage
  #s = subprocess.Popen(base_cmd + list(sources) + ["-c", "-o", noln])
  #s.communicate()
  #if s.returncode: return s.returncode
  
  ## Pre-assembly stage
  #s = subprocess.Popen(base_cmd + list(sources) + ["-S", "-o", sn])
  #s.communicate()
  #if s.returncode: return s.returncode

  ## Defines
  #concat = dirn + "concat.c"
  #cc = open(concat, "w")
  #subprocess.Popen(["cat"] + list(sources), stdout = cc)
  #cc.close()
  #d = subprocess.Popen(base_cmd + ["-dM", "-E", concat, "-o", defn])
  #d.communicate()
  #os.remove(concat)
  #if d.returncode: return d.returncode
  
  # obj -> bin
  b = subprocess.Popen(["avr-objcopy", "-R", ".eeprom", "-O", "binary", objn, binn]) 
  b.communicate()
  return b.returncode


#stamp = datetime.strftime(datetime.today(), "%Y%m%d%H%M%S%f")
#folder = stamp
#os.mkdir(folder)
#print(folder)
folder = "."

cs = compile(folder)
if cs:
  print("Compiler error", cs)
  exit()

meta = open("meta", 'wb')
symbols = objdump.correct_symbols(objdump.get_symbols(objdump.ram_sections + objdump.rom_sections))
pickle.dump((defines, symbols), meta)
meta.close()

subprocess.Popen(["ls", "-l", folder + "/obj.obj", folder + "/bin.bin"]).communicate()
#subprocess.Popen(["scp", folder + "/bin.bin", "stefan@stefuc.homeip.net:~"]).communicate()
subprocess.Popen(["avrdude", "-c", "stk500v2", "-P", "/dev/ttyUSB0", "-p", "m32", "-U", "flash:w:bin.bin"]).communicate()
