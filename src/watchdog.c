static const timer_t reseter_dly  = TIMER_S(1);
static const timer_t syscheck_dly = TIMER_MIN(2);
static const timer_t schcheck_dly = TIMER_S(30);
static uint8_t syscheck;
       uint8_t schcheck;

void watchdog_reseter()
{
  wdt_reset();
  DBG2CP_VAR(last_wdt_reset, timer_now());
  syscheck++;
  assert(syscheck < (syscheck_dly / reseter_dly));
  schcheck++;
  assert(schcheck < (schcheck_dly / reseter_dly));
}

void syscheck_loop()
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
    syscheck = 0;
}

void watchdog_start()
{
  wdt_enable(WDTO_2S);
}

__attribute__((noreturn)) void watchdog_mcu_reset()
{
  cli();
  wdt_enable(0);
  while (1);
}
