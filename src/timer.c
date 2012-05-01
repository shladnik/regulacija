/* this is a very cheap check for such a fundamental thing like timer is */
#define OVF_CHECK 1

static uint16_t high;
static bool en;
static uint16_t cmp_high;

void timer_start()
{
#if   PRESCALER == 1
  TCCR1B = 0x1;
#elif PRESCALER == 8
  TCCR1B = 0x2;
#elif PRESCALER == 64
  TCCR1B = 0x3;
#elif PRESCALER == 256
  TCCR1B = 0x4;
#elif PRESCALER == 1024
  TCCR1B = 0x5;
#else
#error unsupported prescaler
#endif
#if OVF_CHECK
  TCCR0B  = 0x5;
#if __AVR_ATmega32__
  SFIOR |= (1 << PSR10);
#else
  GTCCR  |= (1 << PSRSYNC);
#endif
  TIFR0   = (1 << TOV0 );
  TIMSK0 |= (1 << TOIE0);
#endif
  TIFR1   = (1 << TOV1 );
  TIMSK1 |= (1 << TOIE1);
}

#if OVF_CHECK
DBG_ISR(TIMER0_OVF_vect, ISR_BLOCK)
{
#if PRESCALER > 256
  static uint16_t high0;
#else
  static uint8_t  high0;
#endif
  high0++;
  if ((high0 % PRESCALER) == 0)
    assert(((uint8_t)high & 0x03) != 0x2);
}
#endif

void TIMER1_COMPA_vect_trigger()
{
  OCR1A = TCNT1 + (PRESCALER > 1 ? 0x2 : 0x8);
  while (!(TIFR1 & (1 << OCIE1A)));
}

DBG_ISR(TIMER1_OVF_vect,)
{
  high++;
  if (en && cmp_high == high) {
    en = 0;
    TIMSK1 |= 1 << OCIE1A;
    TIFR1   = 1 << OCIE1A;
    if (OCR1A <= TCNT1) {
      TIMER1_COMPA_vect_trigger();
    }
  }
}

DBG_ISR(TIMER1_COMPA_vect,)
{
  TIMSK1 &= ~(1 << OCIE1A);
  extern void timer_int();
  timer_int();
}

timer_t timer_now()
{
  uint16_t TCNT1_val;
  timer_t  high_val;
  
  bool ovf;
  DBG_ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    high_val  = high;
    TCNT1_val = TCNT1;
    ovf = TIFR1 & (1 << TOV1);
    if (ovf) TCNT1_val = TCNT1;
  }
  if (ovf) {
    high_val++;
  }
  
  timer_t ret_val = ((timer_t)high_val << 16) | (timer_t)TCNT1_val;
  return ret_val;
}

bool in_range(timer_t s, timer_t val, timer_t e)
{
  bool if0 = val >= s;
  bool if1 = val <  e;
  return s <= e ? if0 && if1 :
                  if0 || if1;
}

void timer_set(timer_t cmp)
{
  timer_unset();

  timer_t tracked = timer_tracked_get();
  OCR1A = (uint16_t)cmp;
  TIFR1 = 1 << OCIE1A;
  timer_t now = timer_now();

  if (in_range(tracked, cmp, now)) {
#ifndef NDEBUG
    DBG static uint8_t timer_late_cnt;
    DBG static timer_t timer_late_max;
    timer_late_cnt++;
    timer_late_max = MAX(timer_late_max, now - tracked);
#endif
    TIMER1_COMPA_vect_trigger();
    TIMSK1 |= 1 << OCIE1A;
  } else {
    cmp_high = cmp >> 16;
    if (cmp_high == high) {
      TIMSK1 |= 1 << OCIE1A;
    } else {
      en = 1;
    }
  }
}

void timer_unset()
{
  en = 0;
  TIMSK1 &= ~(1 << OCIE1A);
}
