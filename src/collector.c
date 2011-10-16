#if 0
inline bool temp_in_range(temp_t temp, temp_t min, temp_t max) __attribute__((always_inline));
bool temp_in_range(temp_t temp, temp_t min, temp_t max)
{
  return min <= temp && temp <= max;
}
#else
#define temp_in_range(temp, min, max) (min <= temp && temp <= max)
#endif

#if 0
void temp_ctl_relay(temp_t temp, RELAY relay, temp_t start, temp_t stop)
{
  bool 
  if      (temp > start) relay_on (relay);
  else if (temp < stop ) relay_off(relay);
}
#define temp_ctl_relay(temp, relay, start, stop) { start < stop
#endif

void collector_loop()
{
  temp_t tab [] = {
    DS18B20_STABLE_S_B,
    DS18B20_COLLECTOR,
  };

  ds18b20_get_temp_tab(sizeof(tab)/sizeof(temp_t), RESOLUTION_9, 7, tab);
  temp_t t_stable_s_b = tab[0];
  temp_t t_collector  = tab[1];

  if (!temp_in_range(t_collector,  TEMP(-30), TEMP(90)) ||
      !temp_in_range(t_stable_s_b, TEMP(-30), TEMP(90))) {
    relay_on (RELAY_PUMP_COLLECTOR);
  } else {

  if (t_collector - t_stable_s_b > CONFIG_GET(collector_diff_on))
    relay_on (RELAY_PUMP_COLLECTOR);
  else if (t_collector - t_stable_s_b < CONFIG_GET(collector_diff_off))
    relay_off(RELAY_PUMP_COLLECTOR);
  }
}
