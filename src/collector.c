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
  temp_t t_collector  = ds18b20_get_temp(DS18B20_COLLECTOR , RESOLUTION_9, 7);
  temp_t t_stable_s_b = ds18b20_get_temp(DS18B20_STABLE_S_B, RESOLUTION_9, 7);

  if (!temp_in_range(t_collector,  TEMP(-30), TEMP(90)) ||
      !temp_in_range(t_stable_s_b, TEMP(-30), TEMP(90))) {
    relay_on (RELAY_PUMP_COLLECTOR);
  } else {

  if (t_collector - t_stable_s_b > TEMP(10))
    relay_on (RELAY_PUMP_COLLECTOR);
  else if (t_collector - t_stable_s_b < TEMP(5))
    relay_off(RELAY_PUMP_COLLECTOR);
  }
}
