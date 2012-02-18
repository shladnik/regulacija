
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

  temp_t diff_on       () { return TEMP(5); }
  temp_t diff_off      () { return TEMP(2); }
  temp_t diff          () { return t_house_s_t - t_stable_s_t; }
  bool diff_h2s        () { return  diff() >= diff_on() || (pumping_state == PUMPING_H2S &&  diff() >= diff_off()); }
  bool diff_s2h        () { return -diff() >= diff_on() || (pumping_state == PUMPING_S2H && -diff() >= diff_off()); }
  temp_t min_house_s_b () { return TEMP(55); }
  bool furnace         () { return relay_get(RELAY_PUMP_FURNACE) && t_house_s_b >= min_house_s_b(); }
  bool collector       () { return t_collector > MIN(t_house_s_b, t_stable_s_b) && t_house_s_b < t_stable_s_b; }
  date_t mms = (date_t){ 0, 0,  4, -1, -1, -1, -1 };
  date_t mme = (date_t){ 0, 0,  8, -1, -1, -1, -1 };
  date_t mes = (date_t){ 0, 0, 16, -1, -1, -1, -1 };
  date_t mee = (date_t){ 0, 0, 20, -1, -1, -1, -1 };
  bool milking_time    () { return (timecmp_lt(mms, date) && timecmp_ge(mme, date)) || (timecmp_lt(mes, date) && timecmp_ge(mee, date)); }
  //bool milking_time    () { return 1; }
  bool h2s             () { return  milking_time() && diff_h2s(); }
  bool s2h             () { return !milking_time() && diff_s2h(); }

  DBG static bool   d_mms          ; d_mms           = timecmp_lt(mms, date);
  DBG static bool   d_mme          ; d_mme           = timecmp_ge(mme, date);
  DBG static temp_t d_diff_on      ; d_diff_on       = diff_on      ();
  DBG static temp_t d_diff_off     ; d_diff_off      = diff_off     ();
  DBG static temp_t d_diff         ; d_diff          = diff         ();
  DBG static bool   d_diff_h2s     ; d_diff_h2s      = diff_h2s     ();
  DBG static bool   d_diff_s2h     ; d_diff_s2h      = diff_s2h     ();
  DBG static temp_t d_min_house_s_b; d_min_house_s_b = min_house_s_b();
  DBG static bool   d_furnace      ; d_furnace       = furnace      ();
  DBG static bool   d_collector    ; d_collector     = collector    ();
  DBG static bool   d_milking_time ; d_milking_time  = milking_time ();
  DBG static bool   d_h2s          ; d_h2s           = h2s          ();
  DBG static bool   d_s2h          ; d_s2h           = s2h          ();
  
  if      (furnace()   || h2s()) pumping_state = PUMPING_H2S;
  else if (collector() || s2h()) pumping_state = PUMPING_S2H;
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
