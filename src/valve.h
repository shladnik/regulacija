#ifndef __VALVE_H__
#define __VALVE_H__

typedef enum
{
#include "valve_list.h"
} VALVE;

#define VALVE_STATE_MIN 0
#define VALVE_STATE_MAX 100

#if   VALVE_STATE_MAX < (1 <<  8)
typedef uint8_t valve_state_t;
#elif VALVE_STATE_MAX < (1 << 16)
typedef uint16_t valve_state_t;
#elif VALVE_STATE_MAX < (1 << 32)
typedef uint32_t valve_state_t;
#else
#error VALVE_STATE_MAX too big
#endif

#define VALVE_TIMESCALE (TIMER_S(120) / VALVE_STATE_MAX)
#define VALVE_MIN_MOVE  DIV_CEIL(TIMER_S(5), VALVE_TIMESCALE)

void valve_init();
void valve_open(VALVE i);
void valve_close(VALVE i);
void valve_stop(VALVE i);
void valve_open_for(VALVE i, valve_state_t amount);
void valve_close_for(VALVE i, valve_state_t amount);
valve_state_t valve_get(VALVE i);
bool valve_opened(VALVE i);
bool valve_closed(VALVE i);

#endif
