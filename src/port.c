#if 0
inline void port_set_0 (uint8_t i, uint8_t mask) __attribute__((always_inline));
inline void port_set_z (uint8_t i, uint8_t mask) __attribute__((always_inline));
inline void port_set_pu(uint8_t i, uint8_t mask) __attribute__((always_inline));
inline void port_set_1 (uint8_t i, uint8_t mask) __attribute__((always_inline));

inline void port_set_8 (uint8_t i, uint8_t val) __attribute__((always_inline));

//inline PORT_STATE port_state_decode(uint8_t mask, uint8_t dir, uint8_t out) __attribute__((always_inline));
inline PORT_STATE port_get_state(uint8_t i, uint8_t mask) __attribute__((always_inline));
inline uint8_t port_get(uint8_t i, uint8_t mask) __attribute__((always_inline));
inline uint8_t port_get_8(uint8_t i) __attribute__((always_inline));
#endif

void port_set_0(uint8_t i, const uint8_t mask)
{
  volatile uint8_t * dir = &DDRA - 3 * i;
  volatile uint8_t * out = dir + 1;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    *out &= ~mask;
    *dir |=  mask;
  }
}

void port_set_z(uint8_t i, uint8_t mask)
{
  volatile uint8_t * dir = &DDRA - 3 * i;
  volatile uint8_t * out = dir + 1;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    *dir &= ~mask;
    *out &= ~mask;
  }
}

void port_set_pu(uint8_t i, uint8_t mask)
{
  volatile uint8_t * dir = &DDRA - 3 * i;
  volatile uint8_t * out = dir + 1;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    *dir &= ~mask;
    *out |=  mask;
  }
}

void port_set_1(uint8_t i, uint8_t mask)
{
  volatile uint8_t * dir = &DDRA - 3 * i;
  volatile uint8_t * out = dir + 1;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    *out |=  mask;
    *dir |=  mask;
  }
}

void port_set_8(uint8_t i, uint8_t val)
{
  volatile uint8_t * dir = &DDRA - 3 * i;
  volatile uint8_t * out = dir + 1;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    *out = val;
    *dir = 0xff;
  }
}

PORT_STATE port_state_decode(uint8_t mask, uint8_t dir, uint8_t out)
{
       if (mask &  dir &  out)   return PORT_1;
  else if (mask & ~dir &  out)   return PORT_PU;
  else if (mask & ~dir & ~out)   return PORT_Z;
  else  /*(mask &  dir & ~out)*/ return PORT_0;
}

PORT_STATE port_get_state(uint8_t i, uint8_t mask)
{
  volatile uint8_t * dir = &DDRA - 3 * i;
  volatile uint8_t * out = dir + 1;
  return port_state_decode(mask, *dir, *out);
}

uint8_t port_get(uint8_t i, uint8_t mask)
{
  volatile uint8_t * dir = &DDRA - 3 * i;
  volatile uint8_t * pin = dir - 1;
  return *pin & mask;
}

uint8_t port_get_8(uint8_t i)
{
  volatile uint8_t * dir = &DDRA - 3 * i;
  volatile uint8_t * pin = dir - 1;
  return *pin;
}
