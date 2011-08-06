volatile void *  last_adr;
timer_t last_time;
func_t  last_isr;
func_t  last_sch_func;
func_t  last_timer_func;
DBG uint8_t assert_cnt;
DBG void *  assert_log [4];
DBG func_t  isr_max;
DBG timer_t isr_max_time;

void log_adr()
{
  last_adr = __builtin_return_address(0);
  last_time = timer_now();
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

__attribute__((section(".init3"), naked, used))
void debug_recovery()
{
  DBG static uint8_t boot_cnt;
  boot_cnt++;

  if (MCUCSR & (1 << WDRF)) {
    DBG static uint8_t wdrf_cnt;
    wdrf_cnt++;

    #define WDR_COPY(name) \
      extern typeof(name) name; \
      DBG static typeof(name) name##_wdr_copy; \
      memcpy(&name##_wdr_copy, &name, sizeof(name));

    WDR_COPY(last_adr);
    WDR_COPY(last_time);
    WDR_COPY(last_isr);
    WDR_COPY(last_sch_func);
    WDR_COPY(last_timer_func);
    WDR_COPY(exexec_func);
    WDR_COPY(sch_queue);
    WDR_COPY(sch_wp);
    WDR_COPY(sch_rp);
extern uint8_t first;
extern slot_t slot [MAX_TIMERS];
    WDR_COPY(first);
    WDR_COPY(slot);
  } else if (MCUCSR & 0x1f) {
    stack_check_init();
    dbg_init();
  }
  
  DBG static uint8_t rst_src_last, rst_src;
  rst_src_last = MCUCSR & 0x1f;
  MCUCSR = MCUCSR & 0xe0;
  rst_src |= rst_src_last;
}

int main()
{
  timer_init();
  sei();

  print_buf_init();
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
