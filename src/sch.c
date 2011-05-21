#define QUEUE_LEN 12

static volatile func_t queue[QUEUE_LEN] = { 0 };
static uint8_t wp = 0;
static uint8_t rp = 0;

static uint8_t pinc(uint8_t p)
{
  p++;
  if (p >= QUEUE_LEN) p = 0;
  return p;
}

void sch_add(func_t func /*, uint8_t level*/)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
#if 1
    if (queue[wp] != 0) {
      printf("Adding: %x\n", (uintptr_t)func);
      uint8_t p = rp;
      do {
        printf("%x\n", (uintptr_t)queue[p]);
        p = pinc(p);
      } while (p != wp);
    }
#endif
    assert(queue[wp] == 0);
    queue[wp] = func;
    wp = pinc(wp);
  }
}

void sch()
{
    while (queue[rp]) {
      func_t func = queue[rp];
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
        queue[rp] = 0;
      }
      rp = pinc(rp);
      dbg32[1] = timer_now();
      dbg16[1] = (uintptr_t)func;
      func();
    }
}
