bool config_range(uintptr_t adr, uintptr_t len)
{
  extern uint8_t __config_start;
  extern uint8_t __config_end;
  uintptr_t first = adr;
  uintptr_t last  = adr + len - 1;
  return first <= last && (uintptr_t)&__config_start <= first && last < (uintptr_t)&__config_end;
}

bool meta_range(uintptr_t adr, uintptr_t len)
{
  extern uint8_t __meta_start;
  extern uint8_t __meta_end;
  uintptr_t first = adr;
  uintptr_t last  = adr + len - 1;
  return first <= last && (uintptr_t)&__meta_start <= first && last < (uintptr_t)&__meta_end;
}

bool writeallow_range(uintptr_t adr, uintptr_t len)
{
  extern uint8_t __bootloader_adr;
  extern uint8_t __flash_size;
  uintptr_t first = adr;
  uintptr_t last  = adr + len - 1;
  return first <= last && (uintptr_t)&__bootloader_adr <= first && last < (uintptr_t)&__flash_size;
}

__attribute__((section(".flash_write")))
void flash_write_block(uint8_t * buf, uintptr_t adr, uintptr_t len)
{
  volatile uint8_t check;
  check = BOOT_SPM_CHECK_VAL;

  if (config_range(adr, len) || writeallow_range(adr, len)) {
    uintptr_t start = adr;
    uintptr_t end   = adr + len;
    uintptr_t i = ~(SPM_PAGESIZE - 1) & start;
    uintptr_t e = i + SPM_PAGESIZE;
    uintptr_t start_a = start & ~0x1;
    uintptr_t end_a   = end   & ~0x1;
  
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
        assert(boot_page_erase_checked(adr, check) == 0);
        boot_spm_busy_wait();
        assert(boot_page_write_checked(adr, check) == 0);
        boot_spm_busy_wait();
      }
      boot_rww_enable();
    }
  }
}


__attribute__((used)) NOINIT uint8_t flash_buf [SPM_PAGESIZE]; // for external usage with exexec

USED void flash_write(uint8_t * buf, uintptr_t adr, uintptr_t len)
{
  uintptr_t start_floor =  adr        &  (SPM_PAGESIZE - 1);
  uintptr_t end_floor   = (adr + len) & ~(SPM_PAGESIZE - 1);
  
  if (start_floor) {
    uintptr_t len_s = MIN(len, SPM_PAGESIZE - start_floor);
    flash_write_block(buf, adr, len_s);
    buf += len_s;
    adr += len_s;
    len -= len_s;
  }
  
  while (adr < end_floor) {
    flash_write_block(buf, adr, SPM_PAGESIZE);
    buf += SPM_PAGESIZE;
    adr += SPM_PAGESIZE;
    len -= SPM_PAGESIZE;
  }

  if (len) {
    flash_write_block(buf, adr, len);
    buf += len;
    adr += len;
    len -= len;
  }
}

USED void flash_read(uint8_t * buf, uintptr_t adr, uintptr_t len)
{
  if (config_range(adr, len) || meta_range(adr, len)) pgm_read(buf, adr, len);
}

INLINE void pgm_read(uint8_t * buf, uintptr_t adr, uintptr_t len)
{
  if (__builtin_constant_p(len)) {
    switch (len) {
      case 1:
        *buf = pgm_read_byte(adr);
        break;
      case 2:
        *(uint16_t *)buf = pgm_read_word(adr);
        break;
      case 4:
        *(uint32_t *)buf = pgm_read_dword(adr);
        break;
      default:
        memcpy_P(buf, (uint8_t *)adr, len);
        break;
    }
  } else {
    memcpy_P(buf, (uint8_t *)adr, len);
  }
}
