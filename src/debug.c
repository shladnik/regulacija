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


USED void dbg_init()
{
  stack_check_init();
  extern uint8_t __data_end;
  extern uint8_t __bss_start;
  for (uint8_t * i = &__data_end; i < &__bss_start; i++) *i = 0x00;
}

__attribute__((section(".init3"), naked, used))
void debug()
{
  const uint8_t RFMASK = (1 << JTRF) | (1 << WDRF) | (1 << BORF) | (1 << EXTRF) | (1 << PORF);
  uint8_t rst_src = MCUCSR & RFMASK;
  MCUCSR = MCUCSR & ~RFMASK;

  if (rst_src & ~(1 << WDRF)) {
    dbg_init();
  } else {
    if (rst_src & (1 << WDRF)) {
      DBG static uint8_t wdrf_cnt;
      wdrf_cnt++;
    } else {
      DBG static uint8_t reboot_cnt;
      reboot_cnt++;
    }
 
    DBG_COPY(last_adr);
    DBG_COPY(last_time);
    DBG_COPY(last_isr);
    //extern void timer_debug(); timer_debug();
    //extern void exexec_debug(); exexec_debug();
  }
 
  DBG static uint8_t rst_src_last, rst_src_all;
  rst_src_last = rst_src;
  rst_src_all |= rst_src;
}

