/* this is a very cheap check for such a fundamental thing like timer is */
#define OVF_CHECK 1

static uint16_t high;
static bool en;
static uint16_t cmp_high;

void timer_start()
{
  TIMSK  |= (1 << TOIE1);
  TCCR1B = 0x1;
#if OVF_CHECK
  TCCR2 = 0x7;
  TIMSK |= (1 << TOIE2);
#endif
}

#if OVF_CHECK
ISR(TIMER2_OVF_vect)
{
  assert(((uint8_t)high & 0x03) == 0x03);
}
#endif

void TIMER1_COMPA_vect_trigger()
{
  OCR1A = TCNT1 + 0x10;
  while (!(TIFR & (1 << OCIE1A))) OCR1A--;
}

ISR(TIMER1_OVF_vect)
{
  high++;
  if (en && cmp_high == high) {
    en = 0;
    TIMSK |= 1 << OCIE1A;
    TIFR   = 1 << OCIE1A;
    if (OCR1A < TCNT1) {
      TIMER1_COMPA_vect_trigger();
    }
  }
}

ISR(TIMER1_COMPA_vect)
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
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    high_val  = high;
    TCNT1_val = TCNT1;
    ovf = TIFR & (1 << TOV1);
    if (ovf) TCNT1_val = TCNT1;
  }
  if (ovf) high_val++;
  
  timer_t ret_val = ((timer_t)high_val << 16) | TCNT1_val;
  return ret_val;
}

bool in_range(timer_t s, timer_t val, timer_t e)
{
  bool if0 = val >= s;
  bool if1 = val <  e;
  return s < e ? if0 && if1 :
                 if0 || if1;
}

void timer_set(timer_t start, timer_t cmp_new)
{
  en = 0;
  TIMSK &= ~(1 << OCIE1A);

  OCR1A = (uint16_t)cmp_new;
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
