NOINIT func_t last_isr;
NOINIT func_t last_sch_func;
NOINIT func_t last_timer_func;
NOINIT static void * last_adr;
DBG uint8_t assert_cnt;
DBG void *  assert_log [4];
DBG timer_t assert_time_log [sizeof(assert_log)/sizeof(assert_log[0])];

void log_adr()
{
  last_adr = __builtin_return_address(0);
}

#if 0
__attribute__((always_inline)) void __assert()
{
  label:
  assert_last = &&label;
  watchdog_mcu_reset();
}
#else
void __assert()
{
  assert_log     [assert_cnt] = __builtin_return_address(0);
  assert_time_log[assert_cnt] = timer_now();
  if (assert_cnt < sizeof(assert_log) / sizeof(assert_log[0]) - 1) assert_cnt++;
  watchdog_mcu_reset();
}
#endif

DBG_ISR(BADISR_vect, ISR_BLOCK)
{
  assert(0);
}

void dbg_init()
{
  extern uint8_t __data_end;
  extern uint8_t __bss_start;
  for (uint8_t * i = &__data_end; i < &__bss_start; i++) *i = 0x00;
}

int main()
{
  if (MCUCSR & (1 << WDRF)) {
    #define WDR_COPY(type, name) \
      extern type name; \
      DBG static type name##_wdr_copy; \
      name##_wdr_copy = name
    WDR_COPY(void *, last_adr);
    WDR_COPY(func_t, last_isr);
    WDR_COPY(func_t, last_sch_func);
    WDR_COPY(func_t, last_timer_func);
  } else {
    stack_check_init();
    dbg_init();
  }
  
  DBG static uint8_t rst_src_last, rst_src;
  rst_src_last = MCUCSR & 0x1f;
  MCUCSR = MCUCSR & 0xe0;
  rst_src |= rst_src_last;

  log_adr();
  log_adr();

  uart_init();
  print_buf_init();

  relay_off_all();
  valve_init();
  timer_init();
  //extern void flash_test();
  //flash_test();
  
  watchdog_start();
  sei();

  lcd_init();
  lprintf(1, 5, "Zaganjam...");

  loops_start();

  while (1) sch();

  return 0;
}
