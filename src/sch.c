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

void sch_add(sch_t e)
{
  DBG_ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    DBG2CP_VAR(last_sch_add, e);
    if (e.level == (typeof(e.level))-1) {
      e.func(e.arg);
    } else {
      uint8_t wpc = wp;
      assert(wpc != rp || queue[wpc].func == 0);
      queue[wpc] = e;
      wp = pinc(wpc);
    }
  }
}

void sch()
{
  while (1) {
    schcheck = 0;

    uint8_t level = ((volatile uint8_t)wp) - rp;
    if (level >= SCH_QUEUE_LEN) level += SCH_QUEUE_LEN;

    if (level) {
      sch_t e = queue[rp];
      queue[rp].func = 0;
      rp = pinc(rp);
#ifdef NDEBUG
      e.func(e.arg);
#else
      DBG2CP static sch_t last_sch; last_sch = e;
      if (PLAIN_CONSOLE) printf("sch:%x(%p,%x)\n", (uint16_t)e.func, e.arg, e.level);
      e.func(e.arg);
      last_sch = (sch_t){ 0, 0, 0 };
#endif
    }
    
    if (level <= SCH_QUEUE_LEN / 2) exexec(); // TODO use scheduler for exexec (add exexec() with function as parameter to queue OR also argument/result buffer)

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
