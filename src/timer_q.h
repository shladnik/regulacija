#ifndef __TIMER_Q_H__
#define __TIMER_Q_H__ 1

#include "timer.h"

void timer_init();
void timer_tracked_set(timer_t t);
timer_t timer_tracked_get();
timer_t timer_now();
void timer_add_cmp(timer_t cmp, sch_t func);
void timer_add(timer_t cnt, sch_t func);
uint8_t timer_cancel(sch_t func, uint8_t nr);
void timer_sleep_ticks(timer_t t);

#define timer_sleep_s(t)  timer_sleep_ticks(TIMER_S (t)) 
#define timer_sleep_ms(t) timer_sleep_ticks(TIMER_MS(t)) 
#define timer_sleep_us(t) timer_sleep_ticks(TIMER_US(t)) 

#endif
