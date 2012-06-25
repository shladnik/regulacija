INLINE void isr_enter()
{
  DBG2CP_LOG(isr_return, __builtin_return_address(0), 8);
  log_adr();
}

INLINE void isr_exit()
{
  log_adr();
}

void log_adr()
{
#ifndef NDEBUG
#if PLAIN_CONSOLE
  printf("log_adr:%x\n", __builtin_return_address(0));
#endif
  DBG2CP_LOG(adr_log, __builtin_return_address(0), 8);

  DBG2CP_VAR(adr_time, timer_now());
  extern uint8_t __heap_start;
  DBG2CP static int16_t adr_free; adr_free = (int16_t)SP - (int16_t)&__heap_start;
  DBG static typeof(adr_free) adr_free_min; adr_free_min = adr_free_min ? MIN(adr_free_min, adr_free) : 0x7fff;
#endif
}

PROGMEM static const uint8_t assert_sign [] = { 'a', 's', 's', 'e', 'r', 't', '(', ')' };

void __assert()
{
  void * ret_adr = __builtin_return_address(0);
#if PLAIN_CONSOLE
  printf("assert:%x\n", ret_adr);
#endif
  DBG_LOG_FINITE(assert_log, ret_adr, 4);
  DBG_VAR(assert_last, ret_adr);
  
  extern uint8_t __data_start;
  memcpy_P(&__data_start, assert_sign, sizeof(assert_sign));
  watchdog_mcu_reset();
}

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

USED void dbg2cp_copy()
{
  extern uint8_t __dbg2cp_start;
  extern uint8_t __dbg2cp_end;
  extern uint8_t __dbgcp_start;
  uintptr_t offset = &__dbgcp_start - &__dbg2cp_start;
  DBG_ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    DBG_VAR(dbg2cp_time, timer_now());
    for (uint8_t * i = &__dbg2cp_start; i < &__dbg2cp_end; i++) *(i + offset) = *i;
  }
}

__attribute__((section(".init3"), naked, used))
void debug()
{
  const uint8_t RFMASK = (1 << WDRF) | (1 << BORF) | (1 << EXTRF) | (1 << PORF);
  const uint8_t assertRF     = WDRF + 1;
  const uint8_t bootloaderRF = WDRF + 2;
  uint8_t rst_src = MCUSR & RFMASK;
  MCUSR = MCUSR & ~RFMASK;
  wdt_disable();

  /* add assert "reset" flag */
  extern uint8_t __data_start;
  if (memcmp_P(&__data_start, assert_sign, sizeof(assert_sign)) == 0) {
    rst_src |=  (1 << assertRF);
    rst_src &= ~(1 <<     WDRF);
  }
  
  /* add bootloader "reset" flag */
  PROGMEM static const char bootloader_sign [] = { 'b', 'o', 'o', 't', 'l', 'o', 'a', 'd' };
  extern uint8_t __data_start;
  if (memcmp_P(&__data_start, bootloader_sign, sizeof(bootloader_sign)) == 0) {
    rst_src |=  (1 << bootloaderRF);
    rst_src &= ~(1 <<         WDRF);
  }
  
  if (rst_src & ~((1 << assertRF) | (1 << WDRF))) {
    debug_init();
  } else {
    if (rst_src & (1 << WDRF)) {
      DBG_CNT(wdrf_cnt);
    } else if (!(rst_src & (1 << assertRF))) {
      DBG_CNT(reboot_cnt);
    }

    dbg2cp_copy();
  }

  DBG_VAR(rst_src_last, rst_src);
  DBG static uint8_t  rst_src_all;
  rst_src_all |= rst_src;
}

