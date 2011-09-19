#define MAX_CRONS 8

typedef struct {
  uint8_t  min;
  uint8_t  hour;
  uint8_t  weekday;
  uint8_t  day;
  uint8_t  month;
  func_t   func;
  uint16_t arg;
  uint8_t  repeat;
} cron_t;

cron_t crons [MAX_CRONS] = {
{ 0, 2, 6, 30, 2, daylight_saving, +1, -1},
{ 0, 9, 6, 30, 2, daylight_saving, -1, -1},
};

void cron()
{
  for (uint8_t i = 0; i < MAX_CRONS; i++) {
    if (crons[i].func) {
      if (crons[i].min     < 60 && crons[i].min     != date.min    ) continue;
      if (crons[i].hour    < 24 && crons[i].hour    != date.hour   ) continue;
      if (crons[i].weekday <  7 && crons[i].weekday != date.weekday) continue;
      if (crons[i].day     < 31) {
        uint8_t mlast = month_len() - 1;
        uint8_t day = MIN(mlast, crons[i].day);
        if (crons[i].weekday < 7) {
          uint8_t min, max;
          if (day <= 3) {
            min = 0;
            max = 6;
          } else if (day >= mlast - 3) {
            min = mlast - 6;
            max = mlast;
          } else {
            min = day - 3;
            max = day + 3;
          }
          if (!(min <= date.day && date.day <= max)) continue;
        } else if (day != date.day) continue;
      }
      if (crons[i].month   < 12 && crons[i].month   != date.month  ) continue;
      
      crons[i].func(crons[i].arg);
      
      if (crons[i].repeat) {
        if (crons[i].repeat != (uint8_t)-1) crons[i].repeat--;
      } else {
        crons[i].func = 0;
      }
    }
  }
}
