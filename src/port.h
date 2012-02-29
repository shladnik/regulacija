#ifndef __PORT_H__
#define __PORT_H__

typedef enum {
  PORT_0,
  PORT_Z,
  PORT_PU,
  PORT_1
} PORT_STATE;

void port_set_0 (uint8_t i, uint8_t mask);
void port_set_z (uint8_t i, uint8_t mask);
void port_set_pu(uint8_t i, uint8_t mask);
void port_set_1 (uint8_t i, uint8_t mask);

void port_set_8 (uint8_t i, uint8_t val);

PORT_STATE port_get_state(uint8_t i, uint8_t mask);
uint8_t port_get(uint8_t i, uint8_t mask);
bool port_get_pin(uint8_t i, uint8_t bit);
uint8_t port_get_8(uint8_t i);

#endif
