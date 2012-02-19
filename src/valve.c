typedef struct {
  const uint8_t relay_en;
  const uint8_t relay_dir;
} valve_relay_t;

typedef struct {
  valve_state_t state;
  valve_state_t goal;
  timer_t last_update;
} valve_t;

#define RELAY_DIR(i) (pgm_read_byte(&valve_relay[i].relay_dir))
#define RELAY_EN(i)  (pgm_read_byte(&valve_relay[i].relay_en ))

static const valve_relay_t valve_relay [] PROGMEM = {
#include "valve_list.c"
};

static valve_t valve_tab [VALVE_NR];

void valve_init()
{
  for (VALVE i = 0; i < VALVE_NR; i++)
    valve_tab[i].state = VALVE_STATE_MAX >> 1;
}

void valve_open(VALVE i)
{
  relay_on (RELAY_DIR(i));
  relay_on (RELAY_EN (i));
}

void valve_close(VALVE i)
{
  relay_off(RELAY_DIR(i));
  relay_on (RELAY_EN (i));
}

void valve_stop(VALVE i)
{
  relay_off(RELAY_EN (i));
  relay_off(RELAY_DIR(i));
}

void valve_refresh(VALVE i)
{
  timer_t now = timer_now();

  if (relay_get(RELAY_EN(i))) {
    valve_state_t amount = now - valve_tab[i].last_update;

    if (relay_get(RELAY_DIR(i)))
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

USED valve_state_t valve_get(VALVE i)
{
  valve_refresh(i);
  return valve_tab[i].state;
}

USED bool valve_opened(VALVE i)
{
  return valve_tab[i].state > (VALVE_STATE_MAX - VALVE_MIN_MOVE);
}

USED bool valve_closed(VALVE i)
{
  return valve_tab[i].state < (VALVE_STATE_MIN + VALVE_MIN_MOVE);
}
