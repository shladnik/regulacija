volatile void *  last_adr;
timer_t last_time;
func_t  last_isr;

DBG func_t  isr_max;
DBG timer_t isr_max_time;

DBG uint8_t assert_cnt;
DBG void *  assert_log [4];

void log_adr()
{
#ifndef NDEBUG
  last_adr = __builtin_return_address(0);
  last_time = timer_now();
#endif
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
  assert_log[assert_cnt] = __builtin_return_address(0);
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
DBG uint8_t arr [0x4a];
__attribute__((section(".init3"), naked, used))
void debug()
{
  DBG static uint8_t boot_cnt;
  boot_cnt++;

  if (MCUCSR & (1 << WDRF)) {
    DBG static uint8_t wdrf_cnt;
    wdrf_cnt++;

    DBG_COPY(last_adr);
    DBG_COPY(last_time);
    DBG_COPY(last_isr);
    //extern void timer_debug(); timer_debug();
    //extern void exexec_debug(); exexec_debug();
  } else if (MCUCSR & 0x1f) {
    stack_check_init();
    dbg_init();
  }
  
  DBG static uint8_t rst_src_last, rst_src;
  rst_src_last = MCUCSR & 0x1f;
  MCUCSR = MCUCSR & 0xe0;
  rst_src |= rst_src_last;
}

