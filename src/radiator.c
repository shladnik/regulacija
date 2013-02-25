void radiator_loop()
{ 
  temp_t collector = ds18b20_get_temp(DS18B20_OUTSIDE , RESOLUTION_9, 7);

  temp_t read_0 = ds18b20_get_temp(DS18B20_RADIATOR_U, RESOLUTION_11, 7);
  _delay_ms(1000);
  temp_t read_1 = ds18b20_get_temp(DS18B20_RADIATOR_U, RESOLUTION_11, 7);

  temp_t curr = read_1 + (read_1 - read_0) * 16;

  temp_t goal = CONFIG_GET(radiator_goal);
  if (goal < 0) {
    const temp_t wanted = TEMP(23);
    temp_t goal_auto = wanted + (wanted - collector);
    goal = MAX(wanted, goal_auto);
  }

  bool dir = curr > goal;
  temp_t diff = dir ? curr - goal : goal - curr;

  valve_state_t amount = (VALVE_STATE_MAX * (uint32_t)diff) >> (8 + 6);
  
  if (dir) valve_close_for(VALVE_RADIATOR, amount);
  else     valve_open_for (VALVE_RADIATOR, amount);
 
  if (valve_closed(VALVE_RADIATOR) || collector > TEMP(10)) // quick dirty hack - father wants radiator in bathroom warm, maybe we can achieve that by heating without pump
    relay_off(RELAY_PUMP_RADIATOR);
  else
    relay_on (RELAY_PUMP_RADIATOR);
}
#if 0
void radiator_loop_new()
{
  temp_t tab [] = {
    DS18B20_RADIATOR_U,
    DS18B20_RADIATOR_D,
    DS18B20_HOUSE_S_T,
  };
  
  ds18b20_temp_tab_fill(RESOLUTION_11, 7, tab);
  
  CONFIG static temp_t  ambient_temp = TEMP(23);
  CONFIG static uint8_t heating_f    = 4.0 * 0x10;
  CONFIG static temp_t  storage_off  = TEMP(10);

  temp_t goal = CONFIG_GET(ambient_temp);
  goal += (uint32_t)(tab[0] - tab[1]) * CONFIG_GET(heating_f) / 0x10;
  goal = MIN(goal, tab[2] - storage_off);

  valve_state_t amount = VALVE_STATE_MAX * 5 / 100;
  if (goal > tab[1]) valve_open_for (VALVE_RADIATOR, amount);
  else               valve_close_for(VALVE_RADIATOR, amount);
  
  if (valve_closed(VALVE_RADIATOR))
    relay_off(RELAY_PUMP_RADIATOR);
  else
    relay_on (RELAY_PUMP_RADIATOR);
}
#endif
