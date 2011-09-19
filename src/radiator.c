DBG temp_t radiator_goal_calc;

void radiator_loop()
{
  temp_t read_0 = ds18b20_get_temp(DS18B20_RADIATOR_U, RESOLUTION_11, 7);
  _delay_ms(1000);
  temp_t read_1 = ds18b20_get_temp(DS18B20_RADIATOR_U, RESOLUTION_11, 7);

  temp_t curr = read_1 + (read_1 - read_0) * 16;

  temp_t goal;
  CONFIG_READ(goal, radiator_goal);

  if (goal < 0) {
    temp_t t_collector = ds18b20_get_temp(DS18B20_COLLECTOR , RESOLUTION_9, 7);
    goal = TEMP(20) + (TEMP(20) - t_collector);
  }

  radiator_goal_calc = goal;

  bool dir = curr > goal;
  temp_t diff = dir ? curr - goal : goal - curr;

  valve_state_t amount = (VALVE_STATE_MAX * (uint64_t)diff) >> (8 + 6);
  
  if (dir) valve_open_for (VALVE_RADIATOR, amount);
  else     valve_close_for(VALVE_RADIATOR, amount);
  
  if (valve_opened(VALVE_RADIATOR))
    relay_off(RELAY_PUMP_RADIATOR);
  else
    relay_on (RELAY_PUMP_RADIATOR);
}

