#define MAX_CRONS 8

typedef struct {
  uint8_t  min;
  uint8_t  hour;
  uint8_t  weekday;
  uint8_t  day;
  uint8_t  month;
  sch_func_t   func;
  void *   arg;
  uint8_t  repeat;
} cron_t;

CONFIG cron_t crons [MAX_CRONS] = {
{  0,  2,  6, 30,  2, (sch_func_t)daylight_saving, (void *)+1, -1 },
{  0,  3,  6, 30,  9, (sch_func_t)daylight_saving, (void *)-1, -1 }, // TODO this should happen only once per year (or day or whatever) - currently it will loop!
};

void cron()
{
  for (uint8_t i = 0; i < MAX_CRONS; i++) {
    cron_t c = CONFIG_GET(crons[i]);
    DBG2CP static uint8_t cron_i; cron_i = i;
    DBG2CP static cron_t cronjob; cronjob = c;

    if (c.func) {
      if (c.min     < 60 && c.min     != date.min    ) continue;
      if (c.hour    < 24 && c.hour    != date.hour   ) continue;
      if (c.weekday <  7 && c.weekday != date.weekday) continue;
      if (c.day     < 31) {
        uint8_t mlast = month_len() - 1;
        uint8_t day = MIN(mlast, c.day);
        if (c.weekday < 7) {
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
      if (c.month   < 12 && c.month   != date.month  ) continue;
      
      c.func(c.arg);
      
      if (c.repeat) {
        if (c.repeat != (typeof(c.repeat))-1) {
          c.repeat--;
          CONFIG_SET(crons[i].repeat, c.repeat);
        }
      } else {
        c.func = 0;
        CONFIG_SET(crons[i].func, c.func);
      }
    }
    c = (cron_t){ 0, 0, 0, 0, 0, 0, 0, 0 };
  }
}
