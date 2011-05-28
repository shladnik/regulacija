#!/usr/bin/python3

from datetime import datetime 
import gum


#exexec_test()
print(gum.exexec_ds18b20_get_temp(0, 0))

#ovf_prev = 0
#while 1:
#  readcon()
#  #TODO sleep
#  ovf = read_int("rx_ovf_cnt")
#  if ovf != ovf_prev:
#    ovf_prev = ovf
#    print("Overflows detected:", ovf)
#    write_symbol("rx_ovf_cnt", 0)


