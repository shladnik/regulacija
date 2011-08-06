DBG uint8_t flash_point;

BOOTLOADER_SECTION void flash_write_block(uintptr_t adr, uint8_t * buf, uintptr_t len)
{
  assert(0);
  extern uint8_t _etext;
  if (adr <= ((uintptr_t)&_etext | (SPM_PAGESIZE - 1))) {
    static uint8_t flash_magic; // this should NEVER be set from program itself
    assert(flash_magic == 0xa5); // magic
  }

  uintptr_t start = adr;
  uintptr_t end   = adr + len;
  uintptr_t i = ~(SPM_PAGESIZE - 1) & start;
  uintptr_t e = i + SPM_PAGESIZE;
  uintptr_t start_a = start & ~0x1;
  uintptr_t end_a   = end   & ~0x1;

  timer_t tstart = timer_now();
  DBG_ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    eeprom_busy_wait();
    boot_spm_busy_wait();

    bool change = 0;

    for (; i < start_a; i += 2) boot_page_fill(i, pgm_read_word(i));
    if (start & 0x1) {
      uint16_t dat_old = pgm_read_word(i);
      uint16_t dat = (dat_old & 0x00ff) | (*buf << 8);
      change |= dat != dat_old;
      buf++;
      boot_page_fill(i, dat);
      i += 2;
    }
    for (; i < end_a; i += 2) {
      uint16_t dat_old = pgm_read_word(i);
      uint16_t dat = *buf;
      buf++;
      dat |= *buf << 8;
      buf++;
      change |= dat != dat_old;
      boot_page_fill(i, dat);
    }
    if (end & 0x1) {
      uint16_t dat_old = pgm_read_word(i);
      uint16_t dat = (dat_old & 0xff00) | *buf;
      change |= dat != dat_old;
      buf++;
      boot_page_fill(i, dat);
      i += 2;
    }
    for (; i < e; i += 2) boot_page_fill(i, pgm_read_word(i));
    
    if (change) {
      boot_page_erase(adr);
      boot_spm_busy_wait();
      boot_page_write(adr);
      boot_spm_busy_wait();
      boot_rww_enable();
    }
  }

  timer_t tend = timer_now();
  DBG static timer_t flash_write_time_max;
  flash_write_time_max = MAX(flash_write_time_max, tend - tstart);
}

__attribute__((used)) void flash_write(uintptr_t adr, uint8_t * buf, uintptr_t len)
{
  assert(0);
  flash_point = 3;
  uintptr_t start_floor =  adr        &  (SPM_PAGESIZE - 1);
  uintptr_t end_floor   = (adr + len) & ~(SPM_PAGESIZE - 1);
  
  flash_point = 4;
  if (start_floor) {
    uintptr_t len_s = MIN(len, SPM_PAGESIZE - start_floor);
    flash_write_block(adr, buf, len_s);
    buf += len_s;
    adr += len_s;
    len -= len_s;
  }
  flash_point = 5;
  
  while (adr < end_floor) {
    flash_write_block(adr, buf, SPM_PAGESIZE);
    buf += SPM_PAGESIZE;
    adr += SPM_PAGESIZE;
    len -= SPM_PAGESIZE;
  }
  flash_point = 6;

  if (len) {
    flash_write_block(adr, buf, len);
    buf += len;
    adr += len;
    len -= len;
  }
  flash_point = 7;
}

__attribute__((used))
void flash_test(uintptr_t adr, uintptr_t len)
{
  assert(0);
  flash_point = 1;
  uint8_t buf [len];
  for (uintptr_t i = 0; i < len; i++) buf[i] = i+1;
  flash_point = 2;
  flash_write(adr, buf, len);
  
  flash_point = 0xfe;
  DBG static uint8_t bla [0x200];
  for (uintptr_t i = 0; i < sizeof(bla); i++)
    bla[i] = pgm_read_byte(0x4000+i);
  flash_point = 0xff;
}
