/* must be on the start of data section, so it can always be read, independant of version */
__attribute__((section(".build"))) USED date_t build = {
  BUILD_SEC,
  BUILD_MIN,
  BUILD_HOUR,
  BUILD_WEEKDAY,
  BUILD_DAY,
  BUILD_MONTH,
  BUILD_YEAR,
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

  relay_off_all();
  valve_init();

  lcd_init();
  lprintf(1, 5, "Zaganjam...");

  loops_start();

  watchdog_start();

  while (1) {
    sch();
    exexec();
  }

  return 0;
}
