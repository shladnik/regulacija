#ifndef __CLOCK_H__
#define __CLOCK_H__

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
void daylight_saving(uint16_t hours);
uint8_t month_len();
void clock_init();

#endif
