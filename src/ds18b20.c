typedef enum {
  STATE_UNKNOWN,
  STATE_EEPROM_READY,
  STATE_READY,
} STATE;

typedef struct {
  STATE state;
  RESOLUTION resolution;
} ds18b20_t;

static const rom_t rom [] PROGMEM = {
#include "ds18b20_list.c"
};

static ds18b20_t ds18b20_tab [DS18B20_NR]; // zeros are fine - no need to initialize


static const uint8_t eeprom_val     = 0xbd;
static const uint8_t scratchpad_val = 0xdb;


//
// Error & Handling / Debug stuff
//
jmp_buf * ds18b20_err_handler = 0;

enum {
  ERR_NO_PRESENCE,
  ERR_SCRATCHPAD_0,
  ERR_SCRATCHPAD_CRC,
  ERR_EEPROM_WRITE,
  ERR_SCRATCHPAD_WRITE,
  ERR_RESOLUTION,
  ERR_TEMP,
  ERR_NR,
};

DBG uint8_t ds18b20_err_cnt[DS18B20_NR][ERR_NR]; 
DBG uint8_t ds18b20_max_rty[DS18B20_NR][2];

void ds18b20_error(DS18B20 i, uint8_t errno)
{
#ifndef NDEBUG
  if (ds18b20_err_cnt[i][errno] < (typeof(ds18b20_err_cnt[i][errno]))-1)
    ds18b20_err_cnt[i][errno]++;
#else
  (void)i;
#endif
  assert(ds18b20_err_handler);
  longjmp(*ds18b20_err_handler, errno);
}

void ds18b20_set_resolution(DS18B20 i, RESOLUTION r)
{
  if (ds18b20_tab[i].resolution != r) {
    ds18b20_tab[i].resolution = r;
    if (ds18b20_tab[i].state > STATE_EEPROM_READY)
        ds18b20_tab[i].state = STATE_EEPROM_READY;
  }
}

void ds18b20_reset(timer_t rst_time)
{
  onewire_0();
  timer_sleep_ticks(rst_time);
  for (/*DS18B20*/ uint8_t i = 0; i < DS18B20_NR; i++)
    if (ds18b20_tab[i].state > STATE_EEPROM_READY)
        ds18b20_tab[i].state = STATE_EEPROM_READY;
}

/* match rom wrapper */
static void ds18b20_match_rom(DS18B20 i)
{
  rom_t romi;
  for (uint8_t j = 0; j < sizeof(rom_t); j++)
    romi.rom[j] = pgm_read_byte(&rom[i].rom[j]);
  if (onewire_match_rom(romi)) ds18b20_error(i, ERR_NO_PRESENCE);
}



//
// FUNCTION commands
//

void ds18b20_convert_t(DS18B20 i)
{
  onewire_write8(0x44);
#if 0 // gain ~20% of time, but external power is required!
  while (!onewire_read());
#else
  timer_t pu_time = TIMER_MS(750.0) >> (3 - ds18b20_tab[i].resolution);
  onewire_pullup();
  timer_sleep_ticks(pu_time);
#endif
}

void ds18b20_read_scratchpad(DS18B20 i, uint8_t * scratchpad)
{
  const uint8_t scratchpad_size = 8;
 
  ds18b20_match_rom(i);
  onewire_write8(0xbe);
  onewire_read_l(scratchpad, scratchpad_size);
  
  uint8_t j = 0;
  while (scratchpad[j] == 0) if (++j > scratchpad_size)
    ds18b20_error(i, ERR_SCRATCHPAD_0);

  if (onewire_read8() != crc8(scratchpad, scratchpad_size))
    ds18b20_error(i, ERR_SCRATCHPAD_CRC);
}

bool ds18b20_check_scratchpad(DS18B20 i, const uint8_t * scratchpad)
{
  uint8_t readback [8];
  ds18b20_read_scratchpad(i, readback);
  
  for (uint8_t i = 0; i < 3; i++)
    if (readback[2+i] != scratchpad[i]) return 0;

  return 1;
}

void ds18b20_write_scratchpad(DS18B20 i, const uint8_t * scratchpad)
{
  ds18b20_match_rom(i);
  onewire_write8(0x4e);
  for (uint8_t i = 0; i < 3; i++)
    onewire_write8(scratchpad[i]);

  if (!ds18b20_check_scratchpad(i, scratchpad))
    ds18b20_error(i, ERR_SCRATCHPAD_WRITE);
}

void ds18b20_copy_scratchpad(DS18B20 i)
{
  ds18b20_match_rom(i);
  onewire_write8(0x48);
#if 0
  timer_t pu_time = TIMER_MS(10.0);
  onewire_pullup();
  timer_sleep_ticks(pu_time);
#else
  onewire_pullup_dly(10000.0);
#endif
}

void ds18b20_recall_eeprom(DS18B20 i)
{
  ds18b20_match_rom(i);
  onewire_write8(0xb8);
  //while (!onewire_read()); - I think this is not really needed - if it is, timeout should be implemented also
}

bool ds18b20_read_power_supply(DS18B20 i)
{
  ds18b20_match_rom(i);
  onewire_write8(0xb4);
  return onewire_read();
}

//
// INIT functions
//
void ds18b20_init(DS18B20 i)
{
  switch (ds18b20_tab[i].state) {
    case STATE_UNKNOWN: {
      const uint8_t eeprom [] = { eeprom_val, eeprom_val, (RESOLUTION_9 << 5) | 0x1f };
      ds18b20_recall_eeprom(i);
      if (!ds18b20_check_scratchpad(i, eeprom)) {
        ds18b20_write_scratchpad(i, eeprom);
        ds18b20_copy_scratchpad(i);
        ds18b20_recall_eeprom(i);
        if (!ds18b20_check_scratchpad(i, eeprom)) ds18b20_error(i, ERR_EEPROM_WRITE);
      }
      ds18b20_tab[i].state = STATE_EEPROM_READY;
      // don't break - continue with scratchpad initialization
    }
    case STATE_EEPROM_READY: {
      const uint8_t scratchpad [] = { scratchpad_val, scratchpad_val, (ds18b20_tab[i].resolution << 5) | 0x1f };
      ds18b20_write_scratchpad(i, scratchpad);
      ds18b20_tab[i].state = STATE_READY;
    }
    case STATE_READY: {
    }
  }
}

//
// Temperature reading functions
//

temp_t ds18b20_read_temp(DS18B20 i)
{
  uint8_t scratchpad [8];
  ds18b20_read_scratchpad(i, scratchpad);
  
  if (((scratchpad[4] >> 5) & 0x3) != ds18b20_tab[i].resolution) {
    if (ds18b20_tab[i].state > STATE_EEPROM_READY)
        ds18b20_tab[i].state = STATE_EEPROM_READY;
    ds18b20_error(i, ERR_RESOLUTION);
  }

  if (scratchpad[0] == 0x50 &&
      scratchpad[1] == 0x05 &&
      scratchpad[2] != scratchpad_val) {
    ds18b20_error(i, ERR_TEMP);
  }

  return (scratchpad[1] << 12)
       | (scratchpad[0] <<  4);
}

temp_t ds18b20_get_temp_bare(DS18B20 i, RESOLUTION r)
{
  ds18b20_set_resolution(i, r);
  ds18b20_init(i);

  ds18b20_match_rom(i);
  ds18b20_convert_t(i);
  return ds18b20_read_temp(i);
}

USED temp_t ds18b20_get_temp(DS18B20 i, RESOLUTION r, uint8_t rty)
{
  static volatile uint8_t try; // I had to add static (in a case when this func was called on one place only, i think)
  try = 0;
  static volatile timer_t rst_time;
  rst_time = TIMER_MS(10);

  jmp_buf tmp_eh;
  ds18b20_err_handler = &tmp_eh;
  uint8_t errno = setjmp(tmp_eh);
  
  temp_t val;

#ifndef NDEBUG
  if (ds18b20_max_rty[i][0] < try) {
    ds18b20_max_rty[i][0] = try;
    ds18b20_max_rty[i][1] = errno;
  }
#else
  (void)i;
  (void)errno;
#endif

  if (try <= rty) {
    if (try >= 2) {
      ds18b20_reset(rst_time);
      rst_time <<= 1; // double time for next try
    }

    try++;
    val = ds18b20_get_temp_bare(i, r);
  } else {
    val = INT16_MAX;
  }
  
  ds18b20_err_handler = 0;

  return val;
}
