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
void onewire_search_rom()
{
  printf("Searching for 1-Wire devices...\n");

  uint64_t device [16];
  uint8_t nr = 0;
  uint64_t min;
  uint8_t colision = 0;

  do {
    device[nr] = 0;

    min = nr ? (device[nr - 1] & (((uint64_t)1 << colision) - 1)) | ((uint64_t)1 << colision): 0;

    if (onewire_reset()) {
      printf("None found!\n");
      break;
    }
    onewire_write8(0xf0);

    for (uint8_t i = 0; i < 64; i++) {
      bool zero = !onewire_read();
      bool one  = !onewire_read();

      if (zero && one) {
        if (min & ((uint64_t)1 << i)) {
          zero = 0;
        } else
          colision = i;
      }

      if (zero) {
        onewire_write(0);
      } else {    
        onewire_write(1);
        device[nr] |= ((uint64_t)1 << i);
      }
    }

    printf("[%d] %02x:%04x%08lx:%02x\n", nr, (uint8_t)(device[nr] >> 56), 
                                            (uint16_t)(device[nr] >> 40),
                                            (uint32_t)(device[nr] >>  8), 
                                             (uint8_t)(device[nr] >>  0));
    
    if (*((uint8_t *)&device[nr] + 7) != crc8((uint8_t *)&device[nr], 7))
      printf("CRC error!\n");

    device[++nr] = 0;
  } while (!(min & (uint64_t)1 << colision));

  printf("done!\n");
}

bool onewire_match_rom(const rom_t rom)
{
  /* skip rom if pointer is 0 */
  if (onewire_reset()) {
    return 1;
  } else {
    if (rom) {
      onewire_write8(0x55);
      onewire_write_l(rom, 8);
    } else {
      onewire_write8(0xcc);
    }
    return 0;
  }
}
