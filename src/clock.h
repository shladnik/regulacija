#ifndef __CLOCK_H__
#define __CLOCK_H__

#define LOOP_CLOCK // clock provided via loops.c facility
#define CLOCK_LOOP_PERIOD 1L // in seconds

#ifndef LOOP_CLOCK

#define SAME_OSCILLATOR 0

#if SAME_OSCILLATOR
  #define CLOCK_FREQ      F_CPU
  #define CLOCK_PRESCALER 8L
#else
  #define CLOCK_FREQ      32768L
  #define CLOCK_PRESCALER 128L
#endif

#endif

typedef struct {
  uint8_t  sec;
  uint8_t  min;
  uint8_t  hour;
  uint8_t  weekday;
  uint8_t  day;
  uint8_t  month;
  uint16_t year;
} date_t;

extern date_t date;

void daylight_saving(int8_t hours);
uint8_t month_len();
void clock_init();
void clock_loop();

#endif
