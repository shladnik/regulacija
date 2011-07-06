
typedef struct {
  const uint8_t relay_en;
  const uint8_t relay_dir;
} valve_relay_t;

typedef struct {
  valve_state_t state;
  valve_state_t goal;
  timer_t last_update;
} valve_t;

static const valve_relay_t valve_relay [] PROGMEM = {
#include "valve_list.c"
};

static valve_t valve_tab [VALVE_NR];

#if 0
inline void valve_open (VALVE i) __attribute__((always_inline));
inline void valve_close(VALVE i) __attribute__((always_inline));
inline void valve_stop (VALVE i) __attribute__((always_inline));
inline void valve_refresh(VALVE i) __attribute__((always_inline));
inline void valve_stop_cb(void * arg) __attribute__((always_inline));
inline void valve_control(VALVE i) __attribute__((always_inline));

inline void valve_open_for(VALVE i, valve_state_t amount)  __attribute__((always_inline));
inline void valve_close_for(VALVE i, valve_state_t amount) __attribute__((always_inline));
inline valve_state_t valve_get(VALVE i)                    __attribute__((always_inline));
inline bool valve_opened(VALVE i)                          __attribute__((always_inline));
inline bool valve_closed(VALVE i)                          __attribute__((always_inline));
#endif

void valve_init()
{
  for (VALVE i = 0; i < VALVE_NR; i++)
    valve_tab[i].state = VALVE_STATE_MAX >> 1;
}

void valve_open(VALVE i)
{
  relay_on (pgm_read_byte(&valve_relay[i].relay_dir));
  relay_on (pgm_read_byte(&valve_relay[i].relay_en ));
}

void valve_close(VALVE i)
{
  relay_off(pgm_read_byte(&valve_relay[i].relay_dir));
  relay_on (pgm_read_byte(&valve_relay[i].relay_en ));
}

void valve_stop(VALVE i)
{
  relay_off(pgm_read_byte(&valve_relay[i].relay_en ));
  relay_off(pgm_read_byte(&valve_relay[i].relay_dir));
}

void valve_refresh(VALVE i)
{
  timer_t now = timer_now();

  if (relay_get(pgm_read_byte(&valve_relay[i].relay_en))) {
    valve_state_t amount = now - valve_tab[i].last_update;

    if (relay_get(pgm_read_byte(&valve_relay[i].relay_dir)))
      valve_tab[i].state = VALVE_STATE_MAX - valve_tab[i].state < amount ? VALVE_STATE_MAX : valve_tab[i].state + amount;
    else
      valve_tab[i].state =                   valve_tab[i].state < amount ? VALVE_STATE_MIN : valve_tab[i].state - amount;
  }

  valve_tab[i].last_update = now;
}

void valve_stop_cb(void * arg)
{
  VALVE i = (uint16_t)arg;
  valve_refresh(i);
  valve_stop(i);
}

void valve_control(VALVE i)
{
  timer_cancel(valve_stop_cb, (void *)(uintptr_t)i);

  bool dir = valve_tab[i].state < valve_tab[i].goal;
  valve_state_t amount = dir ? valve_tab[i].goal - valve_tab[i].state : valve_tab[i].state - valve_tab[i].goal;

  if (amount > VALVE_MIN_MOVE) {
    if (dir) valve_open (i);
    else     valve_close(i);
    timer_add(amount, valve_stop_cb, (void *)(uintptr_t)i, -1);
  } else {
    valve_stop(i);
  }
}

void valve_open_for(VALVE i, valve_state_t amount)
{
  valve_refresh(i);
  valve_tab[i].goal = VALVE_STATE_MAX - valve_tab[i].state < amount ? VALVE_STATE_MAX : valve_tab[i].state + amount;
  valve_control(i);
}

void valve_close_for(VALVE i, valve_state_t amount)
{
  valve_refresh(i);
  valve_tab[i].goal =                   valve_tab[i].state < amount ? VALVE_STATE_MIN : valve_tab[i].state - amount;
  valve_control(i);
}

__attribute__((used))
valve_state_t valve_get(VALVE i)
{
  valve_refresh(i);
  return valve_tab[i].state;
}

__attribute__((used))
bool valve_opened(VALVE i)
{
  return valve_tab[i].state > (VALVE_STATE_MAX - VALVE_MIN_MOVE);
}

__attribute__((used))
bool valve_closed(VALVE i)
{
  return valve_tab[i].state < (VALVE_STATE_MIN + VALVE_MIN_MOVE);
}
