class macrodump():
  def __init__(self, fn):
    self.strs = list(map(lambda x: x[len("#define "):], open(fn, "r").readlines()))

  def getdict(self): # This is a partial parse only - could be extended if needed however
    out = map(lambda x: x.split(), self.strs)
    out = filter(lambda x: len(x) <= 2, out)
    r = dict()
    for d in out:
      if len(d):
        if len(d) == 1:
          r[d[0]] = True
        else:
          try:
            r[d[0]] = int(d[1])
          except:
            try:
              r[d[0]] = int(d[1], 16)
            except:
              r[d[0]] = d[1]
    return r
    
  def getregs(self):
    out = map(lambda x: x.split(), self.strs)
    out = list(filter(lambda x: len(x) == 2, out))
    offset = int(list(filter(lambda x: x[0] == "__SFR_OFFSET", out))[0][1], 16)
    regs1 = filter(lambda x: x[1][0: 9] == "_SFR_IO8(" , out)
    regs2 = filter(lambda x: x[1][0:10] == "_SFR_IO16(", out)
    regs1 = map(lambda x: [ x[0], int(x[1][ 9:-1], 16) + offset ], regs1)
    regs2 = map(lambda x: [ x[0], int(x[1][10:-1], 16) + offset ], regs2)
    regs = dict()
    for r in regs1:
      regs[r[0]] = { 'adr' : r[1], 'size' : 1, 'mem' : 'reg', 'section' : None }
    for r in regs2:
      regs[r[0]] = { 'adr' : r[1], 'size' : 2, 'mem' : 'reg', 'section' : None }

    return regs
