
void watchdog_loop()
{
  wdt_reset();
  static timer_t prev = 0;
  timer_t next = prev + TIMER_S(1.);
  timer_add_cmp(next, watchdog_loop, 0, -1);
#ifndef NDEBUG
  timer_t now = timer_now();
  assert(in_range(prev, now, next));
#endif
  prev = next;
}

void watchdog_start()
{
  wdt_enable(WDTO_2S);
  watchdog_loop();
}

void watchdog_mcu_reset()
{
  cli();
  wdt_enable(0);
  while (1);
}
