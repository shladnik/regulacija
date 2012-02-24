typedef struct {
  const uint8_t relay_en;
  const uint8_t relay_dir;
  const bool neg;
} valve_relay_t;

typedef struct {
  valve_state_t state;
  valve_state_t goal;
  valve_state_t last_update;
} valve_t;

#define RELAY_DIR(i) (pgm_read_byte(&valve_relay[i].relay_dir))
#define RELAY_EN(i)  (pgm_read_byte(&valve_relay[i].relay_en ))
#define NEG(i)       (pgm_read_byte(&valve_relay[i].neg      ))

static const valve_relay_t valve_relay [] PROGMEM = {
#include "valve_list.c"
};

static valve_t valve_tab [VALVE_NR];

void valve_init()
{
  for (VALVE i = 0; i < VALVE_NR; i++) {
    valve_tab[i].state = VALVE_STATE_MAX / 2;
    valve_tab[i].goal  = VALVE_STATE_MAX / 2;
  }
}

void valve_open(VALVE i)
{
  if (NEG(i))
    relay_off(RELAY_DIR(i));
  else
    relay_on (RELAY_DIR(i));
  relay_on (RELAY_EN (i));
}

void valve_close(VALVE i)
{
  if (NEG(i))
    relay_on (RELAY_DIR(i));
  else
    relay_off(RELAY_DIR(i));
  relay_on (RELAY_EN (i));
}

void valve_stop(VALVE i)
{
  relay_off(RELAY_EN (i));
  //relay_off(RELAY_DIR(i));
}

void valve_refresh(VALVE i)
{
  valve_state_t now = timer_now() / VALVE_TIMESCALE;

  if (relay_get(RELAY_EN(i))) {
    valve_state_t amount = now - valve_tab[i].last_update;

    if (relay_get(RELAY_DIR(i)) != NEG(i))
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
  valve_state_t amount;

  if (VALVE_STATE_MIN < valve_tab[i].goal && valve_tab[i].goal < VALVE_STATE_MAX)
    amount = dir ? valve_tab[i].goal - valve_tab[i].state : valve_tab[i].state - valve_tab[i].goal;
  else
    amount = VALVE_STATE_MAX;

  if (amount > VALVE_MIN_MOVE) {
    if (dir) valve_open (i);
    else     valve_close(i);
    timer_add((timer_t)amount * VALVE_TIMESCALE, valve_stop_cb, (void *)(uintptr_t)i, -1);
  } else {
    valve_stop(i);
  }
}

void valve_open_for(VALVE i, valve_state_t amount)
{
  if (valve_tab[i].goal < VALVE_STATE_MAX) {
    valve_state_t curr = valve_get(i);
    valve_tab[i].goal = VALVE_STATE_MAX - curr < amount ? VALVE_STATE_MAX : curr + amount;
    valve_control(i);
  }
}

void valve_close_for(VALVE i, valve_state_t amount)
{
  if (valve_tab[i].goal > VALVE_STATE_MIN) {
    valve_state_t curr = valve_get(i);
    valve_tab[i].goal =                   curr < amount ? VALVE_STATE_MIN : curr - amount;
    valve_control(i);
  }
}

USED valve_state_t valve_get(VALVE i)
{
  valve_refresh(i);
  return valve_tab[i].state;
}

USED bool valve_opened(VALVE i)
{
  return valve_get(i) >= VALVE_STATE_MAX;
}

USED bool valve_closed(VALVE i)
{
  return valve_get(i) <= VALVE_STATE_MIN;
}
