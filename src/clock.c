#define CLOCK_FREQ      128 //32768
#define CLOCK_PRESCALER 128
#define SUBSECOND_MAX (CLOCK_FREQ / CLOCK_PRESCALER)

void clock_init()
{
  ASSR = 0x09;
#if   CLOCK_PRESCALER == 1
  TCCR2 = 0x1;
#elif CLOCK_PRESCALER == 8
  TCCR2 = 0x2;
#elif CLOCK_PRESCALER == 32
  TCCR2 = 0x3;
#elif CLOCK_PRESCALER == 64
  TCCR2 = 0x4;
#elif CLOCK_PRESCALER == 128
  TCCR2 = 0x5;
#elif CLOCK_PRESCALER == 256
  TCCR2 = 0x6;
#elif CLOCK_PRESCALER == 1024
  TCCR2 = 0x7;
#else
#error unsupported prescaler
#endif
  TIFR  |= (1 << TOV2 );
  TIMSK |= (1 << TOIE2);
}

date_t date;

uint8_t month_len()
{
  uint8_t len = 31;
  switch (date.month) {
    case  3:
    case  5:
    case  8:
    case 10:
      len = 30;
      break;
    case  1:
      len = date.year & 0x3 ? 28 : 29;
      break;
  }
  return len;
}

void daylight_saving(uint16_t hours)
{
  date.hour += (int8_t)hours;
}

DBG_ISR(TIMER2_OVF_vect,)
{
#if   SUBSECOND_MAX == 1
  uint8_t subsecond = 0;
#elif SUBSECOND_MAX < (1 << 8)
  static uint8_t subsecond;
#elif SUBSECOND_MAX < (1 << 16)
  static uint16_t subsecond;
#elif SUBSECOND_MAX < (1 << 32)
  static uint32_t subsecond;
#else
  #error
#endif

  subsecond++;
  if (subsecond >= SUBSECOND_MAX) {
    subsecond = 0;

    static uint32_t uptime;
    uptime++;
    
    if (date.year) { // year must be set to enable clock & cron
      date.sec++;
      if (date.sec >= 60) {
        date.sec = 0;
        date.min++;
        if (date.min >= 60) {
          date.min = 0;
          date.hour++;
          if (date.hour >= 24) {
            date.hour = 0;
            date.day++;
            if (date.day >= month_len()) {
              date.day = 0;
              date.month++;
              if (date.month >= 12) {
                date.month = 0;
                date.year++;
              }
            }
            
            date.weekday++;
            if (date.weekday >= 7) date.weekday = 0;
          }
        }

        sch_add(cron);
      }
    }
  }
}
