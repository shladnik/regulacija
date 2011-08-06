volatile func_t sch_queue[QUEUE_LEN] = { 0 };
uint8_t sch_wp = 0;
uint8_t sch_rp = 0;

static uint8_t pinc(uint8_t p)
{
  p++;
  if (p >= QUEUE_LEN) p = 0;
  return p;
}

void sch_add(func_t func /*, uint8_t level*/)
{
  DBG_ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    assert(sch_queue[sch_wp] == 0);
    sch_queue[sch_wp] = func;
    sch_wp = pinc(sch_wp);
  }
}

void sch()
{
  while (sch_queue[sch_rp]) {
    func_t func = sch_queue[sch_rp];
    DBG_ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      sch_queue[sch_rp] = 0;
    }
    sch_rp = pinc(sch_rp);
    log_adr();
    last_sch_func = func;
    func();
    last_sch_func = 0;
  }
}
