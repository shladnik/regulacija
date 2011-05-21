#ifndef __TIMER_H__
#define __TIMER_H__

typedef uint32_t timer_t;
#define TIMER_MAX UINT32_MAX

#define TIMER_MIN(t)  ((timer_t)(t * F_CPU * 60))
#define TIMER_S(t)    ((timer_t)(t * F_CPU))
#define TIMER_MS(t)   ((timer_t)(t * F_CPU * 0.001))
#define TIMER_US(t)   ((timer_t)(t * F_CPU * 0.000001))

void timer_start();
timer_t timer_now();
bool in_range(timer_t s, timer_t val, timer_t e);
void timer_set(timer_t start, timer_t cmp);
void timer_unset();

#endif
