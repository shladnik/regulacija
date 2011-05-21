#ifndef __VALVE_H__
#define __VALVE_H__

typedef enum
{
  VALVE_SH0,
  VALVE_SH1,
  VALVE_RADIATOR,
  VALVE_FURNACE,
} VALVE;

typedef timer_t valve_state_t;
#define VALVE_STATE_MAX_CHECK (4ull * 60 * F_CPU)

#if VALVE_STATE_MAX_CHECK > TIMER_MAX
#error timer_t not sufficient for VALVE_STATE_MAX
#else

#define VALVE_STATE_MIN ((valve_state_t)(0))
#define VALVE_STATE_MAX ((valve_state_t)(VALVE_STATE_MAX_CHECK))
#define VALVE_MIN_MOVE  ((valve_state_t)(5ull * F_CPU))

#endif

void valve_init();
void valve_open_for(VALVE i, valve_state_t amount);
void valve_close_for(VALVE i, valve_state_t amount);
valve_state_t valve_get(VALVE i);
bool valve_opened(VALVE i);
bool valve_closed(VALVE i);

#endif
