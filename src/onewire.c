/* pin selection */
#define OW_PORT 1
#define OW_BIT  3
#define OW_MASK (1 << OW_BIT)

#define SMART_PULLUP 0 // TODO: implement timeout!

#if 0
static const double t_r         = 500.0;
static const double t_r_pullup  = 15.0;
static const double t_r_sample  = 75.0;

static const double t_slot      = 80.0;
static const double t_recovery  = 10.0;
static const double t_init      = 2.0;
static const double t_sample    = 15.0;
#else // by spec?
static const double t_r         = 480.0;
static const double t_r_pullup  = 15.0;
static const double t_r_sample  = 75.0; // 60-75

static const double t_slot      = 60.0;
static const double t_recovery  = 1.0;
static const double t_init      = 1.0;
static const double t_sample    = 15.0;
#endif

//
// PHY level
//
inline void onewire_0_dly (double t) __attribute__((always_inline));
inline void onewire_z_dly (double t) __attribute__((always_inline));
inline void onewire_pu_dly(double t) __attribute__((always_inline));
inline void onewire_1_dly (double t) __attribute__((always_inline));

void onewire_0_dly(double t)
{
  onewire_0();
  _delay_us(t);
}

void onewire_z_dly(double t)
{
  onewire_z();
  _delay_us(t);
}

void onewire_pu_dly(double t)
{
  onewire_pu();
  _delay_us(t);
}

void onewire_1_dly(double t)
{
  onewire_1();
  _delay_us(t);
}

void onewire_0()
{
  port_set_0(OW_PORT, OW_MASK);
}

void onewire_z()
{
  port_set_z(OW_PORT, OW_MASK);
}

void onewire_pu()
{
  port_set_pu(OW_PORT, OW_MASK);
}

void onewire_1()
{
  port_set_1(OW_PORT, OW_MASK);
}

bool onewire_val()
{
  return port_get(OW_PORT, OW_MASK);
}

//
// SLOT level
//

bool onewire_reset()
{
  bool failed;
  DBG_ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    onewire_0_dly(t_r);
    onewire_idle_dly(t_r_pullup);
    onewire_z_dly(t_r_sample - t_r_pullup);
    failed = onewire_val();
#if SMART_PULLUP
    while (!onewire_val());
    onewire_idle_dly(t_r - t_r_sample);
#else
    onewire_slave_dly(t_r - t_r_sample);
#endif
  }
  return failed;
}

void onewire_write0()
{
  DBG_ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    onewire_0_dly(t_slot);
    onewire_idle_dly(t_recovery);
  }
}

void onewire_write1()
{
  DBG_ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    onewire_0_dly(t_init);
    onewire_idle_dly(t_slot - t_init + t_recovery);
  }
}

void onewire_write(bool val)
{
  if (val) onewire_write1();
  else     onewire_write0();
}

bool onewire_read()
{ 
  bool val;
  DBG_ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    onewire_0_dly(t_init);
    onewire_z_dly(t_sample - t_init);
    val = onewire_val();
#if SMART_PULLUP
    while (!onewire_val());
    //onewire_1_dly(t_slot - t_sample);
#else
    onewire_slave_dly(t_slot - t_sample);
#endif
    onewire_idle_dly(t_recovery);
  }
  return val;
}

//
// BYTE level
//

void onewire_write8(uint8_t val)
{
  for (uint8_t i = 0; i < 8; i++) {
    onewire_write(val & 0x1);
    val >>= 1;
  }
}

uint8_t onewire_read8()
{
  uint8_t val = 0;
  for (uint8_t i = 0; i < 8; i++) {
    val >>= 1;
    val |= onewire_read() << 7;
  }
  return val;
}

void onewire_write_l(const uint8_t * const dat, uint8_t len)
{
  for (uint8_t i = 0; i < len; i++) onewire_write8(dat[i]);
}

void onewire_read_l(uint8_t * dat, uint8_t len)
{
  for (uint8_t i = 0; i < len; i++) dat[i] = onewire_read8();
}

//
// ROM commands
//
USED uint8_t onewire_search_rom(rom_t * tab, uint8_t tab_len, rom_t min)
{
  min = (rom_t){ { 0 } }; // TODO remove - this was done for testing with exexec
  uint8_t nr = 0;
  uint8_t colision [2] = {8, 8};

  while (1) {
    if (onewire_reset()) break;
    onewire_write8(0xf0);

    for (uint8_t i = 0; i < 8; i++) {
      for (uint8_t j = 0; j < 8; j++) {
        bool zero = !onewire_read();
        bool one  = !onewire_read();

        if (zero && one) {
          if (min.rom[i] & (1 << j)) {
            zero = 0;
          } else {
            colision[0] = i;
            colision[1] = j;
          }
        }

        if (zero) {
          onewire_write(0);
        } else {    
          onewire_write(1);
          min.rom[i] |= 1 << j;
        }
      }
    }

    //if (crc8(&curr.rom[0], 7) != curr.rom[7]) continue; // TODO?
  
    if (nr < tab_len) tab[nr] = min;
    nr++;
   
    if (colision[0] < 8) {
      min.rom[colision[0]] &= (1 << colision[1]) - 1;
      min.rom[colision[0]] |= (1 << colision[1]);
      for (uint8_t i = colision[0] + 1; i < 8; i++) min.rom[i] = 0x00;
      colision[0] = 8;
    } else {
      break;
    }
  }

  return nr;
}

bool onewire_match_rom(const rom_t rom)
{
  /* skip rom if pointer is 0 */
  if (onewire_reset()) {
    return 1;
  } else {
    if (rom.rom[0]/* || rom.rom[1] || ...*/) {
      onewire_write8(0x55);
      onewire_write_l(rom.rom, 8);
    } else {
      onewire_write8(0xcc);
    }
    return 0;
  }
}
