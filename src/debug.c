extern uint8_t __data_start;
PROGMEM static const uint8_t     assert_sign [] = { 'a', 's', 's', 'e', 'r', 't', '(', ')' };
PROGMEM static const uint8_t bootloader_sign [] = { 'b', 'o', 'o', 't', 'l', 'o', 'a', 'd' };
PROGMEM static const uint8_t   soft_rst_sign [] = { 's', 'o', 'f', 't', '_', 'r', 's', 't' };

uintptr_t dump_stack()
{
  extern uint8_t __stack;
  uintptr_t sp = SP + 3;
  uintptr_t size = (uintptr_t)(&__stack) - sp;
  DBG_VAR(stack_size, size);
  DBG static uint8_t stack_dump [0xc8];
  memcpy(stack_dump, (uint8_t *)sp, MIN(sizeof(stack_dump) / sizeof(stack_dump[0]), size));
  return size;
}

void log_adr()
{
#ifndef NDEBUG
  void * ret_adr = __builtin_return_address(0);
#if PLAIN_CONSOLE
  printf("log_adr:%x\n", ret_adr);
#endif
  DBG2CP_LOG(adr_log, ret_adr, 8);

  DBG2CP_VAR(adr_time, timer_now());
  extern uint8_t __heap_start;
  DBG2CP static int16_t adr_free; adr_free = (int16_t)SP - (int16_t)&__heap_start;
  DBG static typeof(adr_free) adr_free_min; adr_free_min = adr_free_min ? MIN(adr_free_min, adr_free) : 0x7fff;
#endif
}

INLINE void isr_enter()
{
  uintptr_t ret_adr = (uintptr_t)__builtin_return_address(0);
  DBG2CP_LOG(isr_return, ret_adr, 8);
  extern uint8_t __interruptable0_start;
  extern uint8_t __interruptable0_end;
  extern uint8_t __interruptable1_start;
  extern uint8_t __interruptable1_end;
  assert((((uintptr_t)&__interruptable0_start)>>1 <= ret_adr && ret_adr < ((uintptr_t)&__interruptable0_end)>>1) ||
         (((uintptr_t)&__interruptable1_start)>>1 <= ret_adr && ret_adr < ((uintptr_t)&__interruptable1_end)>>1));
  log_adr();
}

INLINE void isr_exit()
{
  log_adr();
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

USED void soft_rst()
{
  cli();
  memcpy_P(&__data_start, soft_rst_sign, sizeof(soft_rst_sign));
  watchdog_mcu_reset();
}

void __assert()
{
  cli();
  dump_stack();
  void * ret_adr = __builtin_return_address(0);
#if PLAIN_CONSOLE
  printf("assert:%x\n", ret_adr);
#endif
  DBG_LOG_FINITE(assert_log, ret_adr, 4);
  DBG_VAR(assert_last, ret_adr);
  
  memcpy_P(&__data_start, assert_sign, sizeof(assert_sign));
  watchdog_mcu_reset();
}

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

const uint8_t RFMASK = (1 << WDRF) | (1 << BORF) | (1 << EXTRF) | (1 << PORF);

__attribute__((section(".init1"), naked, used))
void debug0()
{
  asm volatile
  (
    "cli\n\t"
    "clr __zero_reg__\n\t"
  );
  if ((MCUSR & RFMASK) == 0) dump_stack();
}

__attribute__((section(".init3"), naked, used))
void debug1()
{
  const uint8_t assertRF     = WDRF + 1;
  const uint8_t bootloaderRF = WDRF + 2;
  const uint8_t soft_rstRF   = WDRF + 3;
  const uint8_t dbgRFMASK = (1 << assertRF) | (1 << WDRF);

  uint8_t rst_src = MCUSR;
  MCUSR = rst_src & ~RFMASK;
  wdt_disable();
  rst_src &= RFMASK;

  if (rst_src & (1 << WDRF)) {
    if        (memcmp_P(&__data_start,     assert_sign, sizeof(    assert_sign)) == 0) {
      rst_src |=  (1 <<     assertRF);
      rst_src &= ~(1 <<         WDRF);
    } else if (memcmp_P(&__data_start, bootloader_sign, sizeof(bootloader_sign)) == 0) {
      rst_src |=  (1 << bootloaderRF);
      rst_src &= ~(1 <<         WDRF);
    } else if (memcmp_P(&__data_start,   soft_rst_sign, sizeof(  soft_rst_sign)) == 0) {
      rst_src |=  (1 <<   soft_rstRF);
      rst_src &= ~(1 <<         WDRF);
    }
  }
  
  if (rst_src & ~dbgRFMASK) {
    debug_init();
  } else {
    if (rst_src == 0) {
      DBG_CNT(reboot_cnt);
    } else if (rst_src & (1 << WDRF)) {
      DBG_CNT(wdrf_cnt);
    } else {
      /* must have been assert */
    }

    dbg2cp_copy();
  }

  DBG_VAR(rst_src_last, rst_src);
  DBG static uint8_t  rst_src_all;
  rst_src_all |= rst_src;
}

