#ifndef __PUMPING_H__
#define __PUMPING_H__

typedef enum {
  PUMPING_IDLE,
  PUMPING_S2H,
  PUMPING_H2S,
} pumping_state_t;

extern pumping_state_t pumping_state;

void pumping_loop();

#endif
