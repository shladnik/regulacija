#define SCH_QUEUE_LEN 16

static volatile func_t queue[SCH_QUEUE_LEN] = { 0 };
static uint8_t wp = 0;
static uint8_t rp = 0;

static uint8_t pinc(uint8_t p)
{
  p++;
  if (p >= SCH_QUEUE_LEN) p = 0;
  return p;
}

static uint8_t level()
{
  return (wp < rp ? SCH_QUEUE_LEN : 0) + wp - rp;
}

void sch_add(func_t func /*, uint8_t level*/)
{
  DBG_ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    DBG2CP static func_t last_sch_add_func;
    last_sch_add_func = func;
    assert(queue[wp] == 0);
    queue[wp] = func;
    wp = pinc(wp);
  }
}

void sch()
{
  while (1) {
    if (level() <= SCH_QUEUE_LEN / 2) exexec();
    
    if (queue[rp]) {
      func_t func = queue[rp];
      DBG_ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
        queue[rp] = 0;
      }
      rp = pinc(rp);
#ifdef NDEBUG
      func();
#else
      DBG2CP static func_t last_sch_func;
      last_sch_func = func;
      timer_t start = timer_now();
      func();
      timer_t dly = timer_now() - start;
      DBG static timer_t max_dly;
      DBG static func_t max_func;
      if (max_dly < dly) {
        max_dly = dly;
        max_func = func;
      }
      last_sch_func = 0;
#endif
    }
  }
}
