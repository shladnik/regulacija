
pumping_state_t pumping_state __attribute__ ((section (".noinit")));

void pumping_loop()
{
#if 0
  temp_t t_stable_s_t = ds18b20_get_temp(DS18B20_STABLE_S_T, RESOLUTION_9, 7);
  temp_t t_house_0    = ds18b20_get_temp(DS18B20_HOUSE_0   , RESOLUTION_9, 7);
  temp_t t_house_s_t  = ds18b20_get_temp(DS18B20_HOUSE_S_T , RESOLUTION_9, 7);

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
#else
  temp_t tab [] = {
    DS18B20_HOUSE_S_B,
    DS18B20_HOUSE_S_T,
    DS18B20_STABLE_S_B,
    DS18B20_STABLE_S_T,
    DS18B20_COLLECTOR,
  };

  ds18b20_get_temp_tab(sizeof(tab)/sizeof(temp_t), RESOLUTION_9, 7, tab);
  
  temp_t t_house_s_b  = tab[0];
  temp_t t_house_s_t  = tab[1];
  temp_t t_stable_s_b = tab[2];
  temp_t t_stable_s_t = tab[3];
  temp_t t_collector  = tab[4];

  temp_t diff_on       LAZY(TEMP(5));
  temp_t diff_off      LAZY(TEMP(2));
  temp_t diff          LAZY(t_house_s_t - t_stable_s_t);
  bool diff_h2s        LAZY( LAZY_GET(diff) >= LAZY_GET(diff_on) || (pumping_state == PUMPING_H2S &&  LAZY_GET(diff) >= LAZY_GET(diff_off)));
  bool diff_s2h        LAZY(-LAZY_GET(diff) >= LAZY_GET(diff_on) || (pumping_state == PUMPING_S2H && -LAZY_GET(diff) >= LAZY_GET(diff_off)));
  temp_t min_house_s_b LAZY(TEMP(55));
  bool furnace         LAZY(relay_get(RELAY_PUMP_FURNACE) && t_house_s_b >= LAZY_GET(min_house_s_b));
  bool collector       LAZY(t_collector > MIN(t_house_s_b, t_stable_s_b));
  bool milking_time    LAZY(0); // TODO
  bool h2s             LAZY( LAZY_GET(milking_time) && LAZY_GET(diff_h2s));
  bool s2h             LAZY(!LAZY_GET(milking_time) && LAZY_GET(diff_s2h));
  
  if      (LAZY_GET(furnace)   || LAZY_GET(h2s)) pumping_state = PUMPING_H2S;
  else if (LAZY_GET(collector) || LAZY_GET(s2h)) pumping_state = PUMPING_S2H;
  else                                           pumping_state = PUMPING_IDLE;
  
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
#endif
}
