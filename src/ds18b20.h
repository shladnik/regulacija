#ifndef __SENSOR_H__
#define __SENSOR_H__ 1

#define ERR_NO_PRESENCE       1
#define ERR_CRC_ROM           2
#define ERR_SCRATCHPAD_0      3
#define ERR_SCRATCHPAD_CRC    4
#define ERR_EEPROM_WRITE      5
#define ERR_SCRATCHPAD_WRITE  6
#define ERR_RESOLUTION        7
#define ERR_TEMP              8

typedef enum
{
  DS18B20_FURNACE_T,  // pec z
  DS18B20_HOUSE_S_B,  // hisa s
  DS18B20_COLLECTOR,  // kolektorji
  DS18B20_HOUSE_S_T,  // hisa z
  DS18B20_FURNACE_B,  // pec s
  DS18B20_STABLE_S_T, // hlev z
  DS18B20_RADIATOR_U, // radiatorji dvizni
  DS18B20_STABLE_S_B, // hlev s
  DS18B20_RADIATOR_D, // radiatorji povratni
  DS18B20_HOUSE_0,    // bojler
  DS18B20_NEW,        // new
} DS18B20;

typedef enum {
  RESOLUTION_9 = 0,
  RESOLUTION_10,
  RESOLUTION_11,
  RESOLUTION_12,
} RESOLUTION;

#define TEMP(x)   ((temp_t)(x * 256.0))
#define TEMP2I(x) ((char)(x >> 8))
#define TEMP_ERR  ((temp_t)(-1))

typedef int16_t temp_t;

void ds18b20_init(DS18B20 i);
void ds18b20_init_all();
temp_t ds18b20_get_temp(DS18B20 i, RESOLUTION r, uint8_t rty);

void ds18b20_sprint_temp_int(char * str, temp_t temp);
void ds18b20_sprint_temp_fp (char * str, temp_t temp);

#endif
