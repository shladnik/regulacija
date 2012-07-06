/* must be on the start of data section, so it can always be read, independant of version */
__attribute__((section(".build"))) USED date_t build = {
  (uint8_t)BUILD_SEC,
  (uint8_t)BUILD_MIN,
  (uint8_t)BUILD_HOUR,
  (uint8_t)BUILD_WEEKDAY,
  (uint8_t)BUILD_DAY,
  (uint8_t)BUILD_MONTH,
  (uint16_t)BUILD_YEAR,
};

int main()
{
  timer_init();
  sei();
  clock_init();
#ifndef NDEBUG
  print_buf_init();
#endif
  uart_init();
#if PLAIN_CONSOLE
  printf("Console up.\n");
#endif
  relay_off_all();
  valve_init();
  lcd_init();
  ac_init();
  watchdog_start();

  ds18b20_init();
  loops_start();

  sch();

  return 0;
}
