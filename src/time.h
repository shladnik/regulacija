#ifndef __TIME_H__
#define __TIME_H__

int timecmp(date_t a, date_t b);

#define timecmp_lt(a, b) (timecmp(a, b) <  0)
#define timecmp_ge(a, b) (timecmp(a, b) >= 0)
#define timecmp_eq(a, b) (timecmp(a, b) == 0)

#endif
