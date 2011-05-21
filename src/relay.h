#ifndef __RELAY_H__
#define __RELAY_H__

typedef enum
{
  RELAY_VALVE_SH0_DIR     ,
  RELAY_VALVE_SH0_EN      ,
  RELAY_VALVE_SH1_DIR     ,
  RELAY_VALVE_SH1_EN      ,
  RELAY_VALVE_RADIATOR_DIR,
  RELAY_VALVE_RADIATOR_EN ,
  RELAY_VALVE_FURNACE_DIR ,
  RELAY_VALVE_FURNACE_EN  ,
  RELAY_PUMP_SH           ,
  RELAY_PUMP_RADIATOR     ,
  RELAY_PUMP_FURNACE      ,
  RELAY_PUMP_COLLECTOR    ,
  RELAY_NR
} RELAY;

void relay_on(RELAY i);
void relay_off(RELAY i);
bool relay_get(RELAY i);
void relay_toggle(RELAY i);
void relay_off_all();
void relay_on_all();

#endif
