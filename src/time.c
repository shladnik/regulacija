
int timecmp(date_t d1, date_t d2)
{
  int r = 0;
  if (d1.year < UINT16_MAX &&
      d2.year < UINT16_MAX) {
    r = d1.year - d2.year;
    if (r) return r;
  }
  if (d1.month < 12 &&
      d2.month < 12) {
    r = d1.month - d2.month;
    if (r) return r;
  }
  if (d1.weekday < 7 &&
      d2.weekday < 7) {
    r = d1.weekday - d2.weekday;
    if (r) return r;
  }
  if (d1.day < 31 &&
      d2.day < 31) {
    uint8_t mlast = month_len() - 1;
    uint8_t day1 = MIN(mlast, d1.day);
    uint8_t day2 = MIN(mlast, d2.day);
    if (d1.weekday < 7 &&
        d2.weekday < 7) { // this must mean they are equal
      uint8_t min, max;
      if (day1 <= 3) {
        min = 0;
        max = 6;
      } else if (day1 >= mlast - 3) {
        min = mlast - 6;
        max = mlast;
      } else {
        min = day1 - 3;
        max = day1 + 3;
      }
      if (!(min <= day2 && day2 <= max)) return day1 - day2;
    } else {
      r = day1 - day2;
      if (r) return r;
    }
  }
  if (d1.hour < 24 &&
      d2.hour < 24) {
    r = d1.hour - d2.hour;
    if (r) return r;
  }
  if (d1.min < 60 &&
      d2.min < 60) {
    r = d1.min - d2.min;
    if (r) return r;
  }
  if (d1.sec < 60 &&
      d2.sec < 60) {
    r = d1.sec - d2.sec;
    if (r) return r;
  }
  return 0;
}
