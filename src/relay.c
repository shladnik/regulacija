typedef struct {
  const uint8_t port;
  const uint8_t mask;
} relay_t;

#if 0
#define PORT(i) (relay_tab[i].port)
#define MASK(i) (relay_tab[i].mask)

static const relay_t const relay_tab [] = {
#else
#define PORT(i) (pgm_read_byte(&relay_tab[i].port))
#define MASK(i) (pgm_read_byte(&relay_tab[i].mask))

static const relay_t const relay_tab [] PROGMEM = {
#endif
  { 2, 1 << 4 },
  { 2, 1 << 5 },
  { 1, 1 << 1 },
  { 1, 1 << 0 },
  { 2, 1 << 1 },
  { 2, 1 << 0 },
  { 2, 1 << 3 },
  { 2, 1 << 2 },
  { 3, 1 << 5 },
  { 3, 1 << 4 },
  { 3, 1 << 7 },
  { 3, 1 << 6 },
};

#if 0
inline void relay_on(RELAY i)     __attribute__((always_inline));
inline void relay_off(RELAY i)    __attribute__((always_inline));
inline bool relay_get(RELAY i)    __attribute__((always_inline));
inline void relay_toggle(RELAY i) __attribute__((always_inline));
inline void relay_off_all()       __attribute__((always_inline));
inline void relay_on_all()        __attribute__((always_inline));
#endif

void relay_off(RELAY i)
{
  port_set_0(PORT(i), MASK(i));
}

void relay_on(RELAY i)
{
  port_set_z(PORT(i), MASK(i));
}

bool relay_get(RELAY i)
{
  return port_get_state(PORT(i), MASK(i)) > PORT_0;
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
