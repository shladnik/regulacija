#ifndef __SENSOR_H__
#define __SENSOR_H__ 1

typedef enum {
#include "ds18b20_list.h"
} DS18B20;

typedef enum {
  RESOLUTION_9 = 0,
  RESOLUTION_10,
  RESOLUTION_11,
  RESOLUTION_12,
} RESOLUTION;

#define TEMP(x)   ((temp_t)(x * 256.0))
#define TEMP_ERR  INT16_MAX
#define TEMP2I(x) ((int8_t)(x >> 8))

typedef int16_t temp_t;

void ds18b20_init(DS18B20 i);
void ds18b20_init_all();
temp_t ds18b20_get_temp(DS18B20 i, RESOLUTION r, uint8_t rty);
void ds18b20_get_temp_tab(DS18B20 nr, RESOLUTION r, uint8_t rty, temp_t * tab);

#endif
