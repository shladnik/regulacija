int main()
{
  timer_init();
  sei();

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

  while (1) sch();

  return 0;
}
