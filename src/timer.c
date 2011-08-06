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
  TCCR0 = 0x5;
  SFIOR |= (1 << PSR10);
  TIFR  |= (1 << TOV1 ) | (1 << TOV0 );
  TIMSK |= (1 << TOIE1) | (1 << TOIE0);
#else
  TIFR  |= (1 << TOV1 );
  TIMSK |= (1 << TOIE1);
#endif
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
  while (!(TIFR & (1 << OCIE1A)));
}

DBG_ISR(TIMER1_OVF_vect,)
{
  high++;
  if (en && cmp_high == high) {
    en = 0;
    TIMSK |= 1 << OCIE1A;
    TIFR   = 1 << OCIE1A;
    if (OCR1A <= TCNT1) {
      TIMER1_COMPA_vect_trigger();
    }
  }
}

DBG_ISR(TIMER1_COMPA_vect,)
{
  TIMSK &= ~(1 << OCIE1A);
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
    ovf = TIFR & (1 << TOV1);
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

void timer_set(timer_t start, timer_t cmp_new)
{
  timer_unset();

  OCR1A = (uint16_t)cmp_new;
  TIFR = 1 << OCIE1A;

  //timer_t now = timer_tracked_get();
  timer_t now = timer_now();
  if (in_range(start, cmp_new, now)) {
    TIMER1_COMPA_vect_trigger();
    TIMSK |= 1 << OCIE1A;
  } else {
    cmp_high = cmp_new >> 16;
    if (cmp_high == high) {
      TIMSK |= 1 << OCIE1A;
    } else {
      en = 1;
    }
  }
}

void timer_unset()
{
  en = 0;
  TIMSK &= ~(1 << OCIE1A);
}
