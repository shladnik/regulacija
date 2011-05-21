
temp_t radiator_goal;

void radiator_goal_loop()
{
#if 1
    //temp_t t_radiator_u = ds18b20_get_temp(DS18B20_RADIATOR_U, RESOLUTION_12, 8);
    //temp_t t_radiator_d = ds18b20_get_temp(DS18B20_RADIATOR_D, RESOLUTION_12, 8);
    
    // y = 1.25 * x - 5
    //temp_t t_expected = t_radiator_d + DIV_ROUND(t_radiator_d, 4) - TEMP(5.0);
    
    //radiator_goal = 2 * t_radiator_u - t_expected;
    
    temp_t t_collector = ds18b20_get_temp(DS18B20_COLLECTOR , RESOLUTION_9, 8);
    radiator_goal = TEMP(20) + (TEMP(20) - t_collector);

    //printf("Radiator goal: %d (expected: %d)\n", TEMP2I(radiator_goal), TEMP2I(t_expected));
    printf("Radiator goal: %d\n", TEMP2I(radiator_goal));
#else
  radiator_goal = TEMP(0);
#endif
}

void radiator_loop()
{
  temp_t read_0 = ds18b20_get_temp(DS18B20_RADIATOR_U, RESOLUTION_11, 8);
  _delay_ms(1000);
  temp_t read_1 = ds18b20_get_temp(DS18B20_RADIATOR_U, RESOLUTION_11, 8);

  temp_t curr = read_1 + (read_1 - read_0) * 16;

  temp_t goal = radiator_goal;

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

