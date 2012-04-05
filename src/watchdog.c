static const timer_t reseter_dly = TIMER_S(1.);
static const timer_t timeout = TIMER_MIN(2.);
static uint8_t timeout_cnt;

void watchdog_reseter()
{
  wdt_reset();

  timeout_cnt++;
  assert(timeout_cnt < (timeout / reseter_dly));

  static timer_t prev = 0;
  timer_t next = prev + reseter_dly;
  timer_add_cmp(next, watchdog_reseter, 0, -1);
#ifndef NDEBUG
  timer_t now = timer_now();
  assert(in_range(prev, now, next));
#endif
  prev = next;
}

void watchdog_loop()
{
  enum {
    FURNACE,
    COLLECTOR,
    NR,
  };

  temp_t tab [NR] = {
    DS18B20_FURNACE_T,
    DS18B20_COLLECTOR,
  };
  
  ds18b20_get_temp_tab(NR, RESOLUTION_9, 7, tab);

  if (tab[FURNACE  ] < TEMP(95) &&
      tab[COLLECTOR] < TEMP(95))
    timeout_cnt = 0;
}

void watchdog_start()
{
  wdt_enable(WDTO_2S);
  watchdog_reseter();
}

__attribute__((noreturn)) void watchdog_mcu_reset()
{
  cli();
  wdt_enable(0);
  while (1);
}
