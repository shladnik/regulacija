
pumping_state_t pumping_state __attribute__ ((section (".noinit")));

void pumping_loop()
{
  temp_t t_stable_s_t = ds18b20_get_temp(DS18B20_STABLE_S_T, RESOLUTION_9, 8);
  temp_t t_house_0    = ds18b20_get_temp(DS18B20_HOUSE_0   , RESOLUTION_9, 8);
  temp_t t_house_s_t  = ds18b20_get_temp(DS18B20_HOUSE_S_T , RESOLUTION_9, 8);

  /* pumping stable <-> house */
  switch (pumping_state) {
    case PUMPING_H2S:
      if      (/*t_house_s_t < radiator_goal - TEMP(2) && */
               t_house_0 < TEMP(45) || (t_house_s_t  - t_stable_s_t < TEMP(3))) pumping_state = PUMPING_IDLE;
      break;
    case PUMPING_S2H:
      if      (/*t_house_0 > TEMP(55) ||*/ (t_stable_s_t - t_house_s_t  < TEMP(3))) pumping_state = PUMPING_IDLE;
      break;
    default:
      if      (/*t_house_s_t > radiator_goal + TEMP(2) && */
               t_house_0 > TEMP(55) && (t_house_s_t  - t_stable_s_t > TEMP(7))) pumping_state = PUMPING_H2S;
      else if (/*t_house_0 < TEMP(45) &&*/ (t_stable_s_t - t_house_s_t  > TEMP(7))) pumping_state = PUMPING_S2H;
      break;
  }

  switch (pumping_state) {
    case PUMPING_H2S:
      if (valve_closed(VALVE_SH0) && valve_opened(VALVE_SH1)) relay_on(RELAY_PUMP_SH);
      valve_close_for(VALVE_SH0, VALVE_STATE_MAX);
      valve_open_for (VALVE_SH1, VALVE_STATE_MAX);
      break;
    case PUMPING_S2H:
      if (valve_opened(VALVE_SH0) && valve_closed(VALVE_SH1)) relay_on(RELAY_PUMP_SH);
      valve_open_for (VALVE_SH0, VALVE_STATE_MAX);
      valve_close_for(VALVE_SH1, VALVE_STATE_MAX);
      break;
    default:
      relay_off(RELAY_PUMP_SH);
      valve_close_for(VALVE_SH0, VALVE_STATE_MAX);
      valve_close_for(VALVE_SH1, VALVE_STATE_MAX);
      break;
  }
}
