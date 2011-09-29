DBG2CP volatile void *  last_adr;
DBG2CP timer_t last_time;
DBG2CP func_t  last_isr;

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
  const uint8_t log_len = sizeof(assert_log) / sizeof(assert_log[0]);
  assert_log[MIN(assert_cnt, log_len - 1)] = __builtin_return_address(0);
  if (assert_cnt < (typeof(assert_cnt))-1) assert_cnt++;
  watchdog_mcu_reset();
}
#endif

DBG_ISR(BADISR_vect, ISR_BLOCK)
{
  assert(0);
}


USED void debug_init()
{
  stack_check_init();
  
  extern uint8_t __dbg_start;
  extern uint8_t __dbg_end;
  for (uint8_t * i = &__dbg_start; i < &__dbg_end; i++) *i = 0x00;
}

__attribute__((section(".init3"), naked, used))
void debug()
{
  const uint8_t RFMASK = (1 << JTRF) | (1 << WDRF) | (1 << BORF) | (1 << EXTRF) | (1 << PORF);
  uint8_t rst_src = MCUCSR & RFMASK;
  MCUCSR = MCUCSR & ~RFMASK;

  if (rst_src & ~(1 << WDRF)) {
    debug_init();
  } else {
    if (rst_src & (1 << WDRF)) {
      DBG static uint8_t wdrf_cnt;
      wdrf_cnt++;
    } else {
      DBG static uint8_t reboot_cnt;
      reboot_cnt++;
    }
 
    extern uint8_t __dbg2cp_start;
    extern uint8_t __dbg2cp_end;
    extern uint8_t __dbgcp_start;
    uintptr_t offset = &__dbgcp_start - &__dbg2cp_start;
    for (uint8_t * i = &__dbg2cp_start; i < &__dbg2cp_end; i++) *(i + offset) = *i;
  }
 
  DBG static uint8_t rst_src_last, rst_src_all;
  rst_src_last = rst_src;
  rst_src_all |= rst_src;
}

