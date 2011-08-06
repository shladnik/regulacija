#ifndef __TIMER_H__
#define __TIMER_H__

#define PRESCALER 8
#if F_CPU % PRESCALER
  #error Cannot exactly match 1 second with timer
#else
  #define F_TIMER (F_CPU / PRESCALER)
#endif

typedef uint32_t timer_t;

#define TIMER_MIN(t)  ((timer_t)(t * F_TIMER * 60))
#define TIMER_S(t)    ((timer_t)(t * F_TIMER))
#define TIMER_MS(t)   ((timer_t)(t * F_TIMER * 0.001))
#define TIMER_US(t)   ((timer_t)(t * F_TIMER * 0.000001))

void timer_start();
timer_t timer_now();
bool in_range(timer_t s, timer_t val, timer_t e);
void timer_set(timer_t start, timer_t cmp);
void timer_unset();

#endif
