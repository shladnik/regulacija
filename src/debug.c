void log_adr()
{
#ifndef NDEBUG
  DBG2CP static volatile void *  adr_log [4];

  const uint8_t log_len = sizeof(adr_log) / sizeof(adr_log[0]);
  for (uint8_t i = 0; i < log_len - 1; i++)
    adr_log[i] = adr_log[i+1];
  adr_log[log_len-1] = __builtin_return_address(0);
#endif
}

#if 0
__attribute__((always_inline)) void __assert()
{
  label:
  assert_log[0] = &&label;
  watchdog_mcu_reset();
}
#else
void __assert()
{
  DBG static uint8_t assert_cnt;
  DBG static void *  assert_log [4];

  const uint8_t log_len = sizeof(assert_log) / sizeof(assert_log[0]);
  assert_log[MIN(assert_cnt, log_len - 1)] = __builtin_return_address(0);
  if (assert_cnt < (typeof(assert_cnt))-1) assert_cnt++;
  watchdog_mcu_reset();
}
#endif

DBG_ISR(BADISR_vect, ISR_BLOCK)
{
  DBG_CNT(vector_bad);
}

//DBG_ISR(WDT_vect, ISR_BLOCK)

#if 0
#define BADISR_CNT(i) DBG_ISR(_VECTOR(i)) { DBG_CNT(vector_##i##_cnt); }
//BADISR_CNT( 1)
//BADISR_CNT( 2)
BADISR_CNT( 3)
BADISR_CNT( 4)
//BADISR_CNT( 5)
BADISR_CNT( 6)
//BADISR_CNT( 7)
BADISR_CNT( 8)
//BADISR_CNT( 9)
BADISR_CNT(10)
//BADISR_CNT(11)
BADISR_CNT(12)
//BADISR_CNT(13)
//BADISR_CNT(14)
BADISR_CNT(15)
//BADISR_CNT(16)
BADISR_CNT(17)
//BADISR_CNT(18)
//BADISR_CNT(19)
BADISR_CNT(20)
#endif

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
  const uint8_t RFMASK = (1 << WDRF) | (1 << BORF) | (1 << EXTRF) | (1 << PORF);
  uint8_t rst_src = MCUSR & RFMASK;
  MCUSR = MCUSR & ~RFMASK;
  wdt_disable();

  /* add bootloader "reset" flag */
  PROGMEM static const char sign [] = { 'b', 'o', 'o', 't', 'l', 'o', 'a', 'd' };
  extern uint8_t __data_start;
  if (memcmp_P(&__data_start, sign, sizeof(sign)) == 0) {
    rst_src |= RFMASK + 1;
    rst_src &= ~(1 << WDRF);
  }
  
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

