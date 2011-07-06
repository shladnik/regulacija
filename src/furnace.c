
void furnace_loop()
{
  temp_t t_furnace_t = ds18b20_get_temp(DS18B20_FURNACE_T, RESOLUTION_9, 7);
#if 1
  //temp_t t_furnace_b = ds18b20_get_temp(DS18B20_FURNACE_B, RESOLUTION_9, 7);
  
  if      (t_furnace_t > TEMP(65)) relay_on (RELAY_PUMP_FURNACE);
  else if (t_furnace_t < TEMP(60)) relay_off(RELAY_PUMP_FURNACE);

  temp_t goal = MIN(TEMP(50), t_furnace_t - TEMP(10));
#else
  temp_t t_furnace_b = ds18b20_get_temp(DS18B20_FURNACE_B, RESOLUTION_9, 7);
  
  bool pump = relay_get(RELAY_PUMP_FURNACE);
  if (t_furnace_t > TEMP(70))                    relay_on (RELAY_PUMP_FURNACE);
  else if (t_furnace_t - t_furnace_b > TEMP(10)) relay_on (RELAY_PUMP_FURNACE);
  else if (t_furnace_t - t_furnace_b < TEMP( 5)) relay_off(RELAY_PUMP_FURNACE);

  temp_t goal = pump ? TEMP(52) : TEMP(48);
#endif
  
  temp_t curr  = ds18b20_get_temp(DS18B20_FURNACE_B, RESOLUTION_12, 7);
  _delay_ms(1000);
  temp_t curr2 = ds18b20_get_temp(DS18B20_FURNACE_B, RESOLUTION_12, 7);
  
  curr = curr2 + (curr2 - curr) * 16;

  bool dir = curr < goal;
  temp_t diff = dir ? goal - curr : curr - goal;

#if 0
  valve_state_t amount = (VALVE_STATE_MAX * (uint64_t)diff) >> (8 + 6);
#else
  valve_state_t amount = (VALVE_STATE_MAX * (uint64_t)diff) >> (8 + 6);
#endif

  if (dir) valve_open_for (VALVE_FURNACE, amount);
  else     valve_close_for(VALVE_FURNACE, amount);
}

