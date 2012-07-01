typedef struct {
  const uint8_t port;
  const uint8_t mask;
  const bool    neg;
} relay_t;

#define PORT(i) (PGM_GET(relay_tab[i].port))
#define MASK(i) (PGM_GET(relay_tab[i].mask))
#define NEG(i)  (PGM_GET(relay_tab[i].neg ))

static const relay_t const relay_tab [] PROGMEM = {
#include "relay_list.c"
};

void relay_off(RELAY i)
{
  if (NEG(i))
    port_set_z(PORT(i), MASK(i));
  else
    port_set_0(PORT(i), MASK(i));
}

void relay_on(RELAY i)
{
  if (NEG(i))
    port_set_0(PORT(i), MASK(i));
  else
    port_set_z(PORT(i), MASK(i));
}

bool relay_get(RELAY i)
{
  return (port_get_state(PORT(i), MASK(i)) > PORT_0) != NEG(i);
}

void relay_toggle(RELAY i)
{
  if (relay_get(i))
    relay_off(i);
  else
    relay_on(i);
}

void relay_off_all()
{
  for (uint8_t i = 0; i < RELAY_NR; i++) relay_off(i);
  //for (RELAY i = 0; i < RELAY_NR; i++) relay_off(i);
}

void relay_on_all()
{
  for (uint8_t i = 0; i < RELAY_NR; i++) relay_on(i);
  //for (RELAY i = 0; i < RELAY_NR; i++) relay_on(i);
}
