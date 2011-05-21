#ifndef __TIMER_Q_H__
#define __TIMER_Q_H__ 1

#include "timer.h"

void timer_init();
timer_t timer_now();
void timer_add_cmp(timer_t now, timer_t cmp, void (*func)(), void * arg, uint8_t level);
void timer_add(timer_t cnt, void (*func)(), void * arg, uint8_t level);
void timer_cancel(void (*func)(), void * arg);

void timer_sleep_ticks(timer_t t);

#define timer_sleep_s(t)  timer_sleep_ticks(TIMER_S (t)) 
#define timer_sleep_ms(t) timer_sleep_ticks(TIMER_MS(t)) 
#define timer_sleep_us(t) timer_sleep_ticks(TIMER_US(t)) 

#endif
