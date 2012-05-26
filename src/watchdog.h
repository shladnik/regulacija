#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

extern uint8_t schcheck;

void watchdog_reseter();
#define WATCHDOG_RESETER_PERIOD TIMER_S(1)
void syscheck_loop();
void watchdog_start();
void watchdog_mcu_reset();

#endif
