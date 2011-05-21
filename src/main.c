#if DBG16_SIZE
uint16_t dbg16 [DBG16_SIZE] __attribute__ ((section (".noinit")));
#endif
#if DBG32_SIZE
uint32_t dbg32 [DBG32_SIZE] __attribute__ ((section (".noinit")));
#endif

void __assert()
{
  printf("ERR: assert %x\n", (uintptr_t)__builtin_return_address(0));
}

ISR(BADISR_vect)
{
  assert(0);
}

int main()
{
  uart_init();
  print_buf_init();

  if (MCUCSR & (1 << EXTRF)) {
    stack_check_init();
  } else {
    stack_check();
  }

  printf("ERR: Reset ");
  uint8_t mcucsr = MCUCSR;
  if (MCUCSR & (1 << PORF )) printf("PORF ");
  if (MCUCSR & (1 << EXTRF)) printf("EXTRF ");
  if (MCUCSR & (1 << BORF )) printf("BORF ");
  if (MCUCSR & (1 << WDRF )) printf("WDRF ");
  if (MCUCSR & (1 << JTRF )) printf("JTRF ");
  MCUCSR = mcucsr & 0xe0;
  printf("\n");

#if DBG16_SIZE || DBG32_SIZE
  printf("dbg16\n");
  for (uint8_t i  = 0; i < DBG16_SIZE; i++) {
    if (dbg16[i]) {
      printf("%x: %x\n", i, dbg16[i]);
    }
    dbg16[i] = 0;
  }
  printf("dbg32\n");
  for (uint8_t i  = 0; i < DBG32_SIZE; i++) {
    if (dbg32[i]) {
      printf("%x: %lx\n", i, dbg32[i]);
    }
    dbg32[i] = 0;
  }
#endif
  
  relay_off_all();
  valve_init();
  timer_init();
  watchdog_start();
  sei();

#if 1
  int i = 0;
  while (1) {
    printf("%04x\n", i++);
    timer_sleep_s(1);
  }
#endif

  lcd_init();
  lprintf(1, 5, "Zaganjam...");
   
  loops_start();
  
  while (1) sch();

  return 0;
}
