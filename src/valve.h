#ifndef __VALVE_H__
#define __VALVE_H__

typedef enum
{
#include "valve_list.h"
} VALVE;

typedef timer_t valve_state_t;

#define VALVE_STATE_MIN 0
#define VALVE_STATE_MAX TIMER_MIN(4)
#define VALVE_MIN_MOVE  TIMER_S(5)

void valve_init();
void valve_open_for(VALVE i, valve_state_t amount);
void valve_close_for(VALVE i, valve_state_t amount);
valve_state_t valve_get(VALVE i);
bool valve_opened(VALVE i);
bool valve_closed(VALVE i);

#endif
