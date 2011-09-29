#!/usr/bin/python3

import os

to_remove = []

filelist = os.listdir(".")
to_remove += filter(lambda x: x[-4:] == ".bin" , filelist)
to_remove += filter(lambda x: x[-4:] == ".obj" , filelist)
to_remove += filter(lambda x: x[0:5] == "meta.", filelist)

filelist = os.listdir("py/")
to_remove += map(lambda x: "py/" + x, filter(lambda x: x[-4:] == ".pyc" , filelist))

filelist = os.listdir("src/auto/")
to_remove += map(lambda x: "src/auto/" + x, filelist)

for f in to_remove:
  os.remove(f)
