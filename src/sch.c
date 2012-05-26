#define SCH_QUEUE_LEN 16

static volatile sch_t queue[SCH_QUEUE_LEN];
static uint8_t wp;
static uint8_t rp;

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

void sch_add(sch_t e)
{
  DBG_ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
#if PLAIN_CONSOLE
      printf("sch_add:%x(%x)\n", e.func, e.arg);
#endif
    DBG2CP_VAR(last_sch_add, e);
    if (e.level == (typeof(e.level))-1) {
      e.func(e.arg);
    } else {
      assert(queue[wp].func == 0);
      queue[wp] = e;
      wp = pinc(wp);
    }
  }
}

void sch()
{
  while (1) {
    schcheck = 0;

    if (level() <= SCH_QUEUE_LEN / 2) exexec();

    sch_t e = queue[rp];
    if (e.func) {
      DBG_ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
        queue[rp].func = 0;
      }
      rp = pinc(rp);
#ifdef NDEBUG
      e.func(e.arg);
#else
#if PLAIN_CONSOLE
      printf("sch:%x(%x)\n", e.func, e.arg);
#endif
      DBG2CP static sch_t last_sch; last_sch = e;
      e.func(e.arg);
      last_sch = (sch_t){ 0, 0, 0 };
#endif
    }
    
    set_sleep_mode(SLEEP_MODE_IDLE);
    cli();
    if (queue[rp].func == 0 && exexec_func == 0) {
      sleep_enable();
      sei();
      sleep_cpu();
      sleep_disable();
    }
    sei();
  }
}
