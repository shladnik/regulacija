#ifndef __ONEWIRE_H__
#define __ONEWIRE_H__ 1

#define onewire_pullup     onewire_1
#define onewire_slave      onewire_z
#define onewire_idle       onewire_1
#define onewire_pullup_dly onewire_1_dly
#define onewire_slave_dly  onewire_z_dly
#define onewire_idle_dly   onewire_1_dly

typedef uint8_t rom_t [8];

void onewire_0();
void onewire_z();
void onewire_pu();
void onewire_1();

void onewire_write(bool val);
bool onewire_read();
void onewire_write8(uint8_t val);
uint8_t onewire_read8();
void onewire_write_l(const uint8_t * const dat, uint8_t len);
void onewire_read_l(uint8_t * dat, uint8_t len);

rom_t * onewire_search_rom();
bool onewire_match_rom(const rom_t rom); // 0 for skip

void onewire_0_dly (double t);
void onewire_z_dly (double t);
void onewire_pu_dly(double t);
void onewire_1_dly (double t);
#endif
