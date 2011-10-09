#!/usr/bin/python3

import os

to_remove = []

filelist = os.listdir(".")
to_remove += filter(lambda x: x[-4:] == ".bin"      , filelist)
to_remove += filter(lambda x: x[-4:] == ".obj"      , filelist)
to_remove += filter(lambda x: x[-8:] == ".tar.bz2"  , filelist)
to_remove += filter(lambda x: x      == "meta"      , filelist)
to_remove += filter(lambda x: x      == ".timestamp", filelist)

filelist = os.listdir("py/")
to_remove += map(lambda x: "py/" + x, filter(lambda x: x[-4:] == ".pyc" , filelist))

filelist = os.listdir("src/auto/")
to_remove += map(lambda x: "src/auto/" + x, filelist)

for f in to_remove:
  os.remove(f)
