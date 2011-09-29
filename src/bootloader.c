#if   SPM_PAGESIZE < (1 <<  8)
typedef uint8_t  pptr_t;
#elif SPM_PAGESIZE < (1 << 16)
typedef uint16_t pptr_t;
#else
#error
#endif

static const uint8_t ACK  = 0xa5;
static const uint8_t NACK = 0x5a;

void restart()
{
  while (!(UCSRA & (1 << UDRE)));
  while (!(UCSRA & (1 << TXC)));
  wdt_enable(0);
  while(1);
}

uint8_t get()
{
  wdt_reset();
  while (!(UCSRA & (1 << RXC)));
  return UDR;
}

void put(uint8_t dat)
{
  while (!(UCSRA & (1 << UDRE)));

  if (UCSRA & ((1<<FE)|(1<<DOR)|(1<<PE))) {
    UDR = NACK;
    restart();
  } else {
    UDR = dat;
  }
}

uint8_t getput()
{
  uint8_t b = get();
  put(b);
  return b;
}

uint16_t getput_word()
{
  uint16_t dat;
  uint8_t * dat_p = (uint8_t *)(&dat);
  dat_p[1] = getput();
  dat_p[0] = getput();
  return dat;
}

void flash_write_block(uint8_t * buf, uintptr_t adr, uintptr_t len)
{
  uintptr_t start = adr;
  uintptr_t end   = adr + len;
  uintptr_t i = ~(SPM_PAGESIZE - 1) & start;
  uintptr_t e = i + SPM_PAGESIZE;
  uintptr_t start_a = start & ~0x1;
  uintptr_t end_a   = end   & ~0x1;

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
  }
  boot_rww_enable();
}

__attribute__((naked))
int main()
{
  if (UCSRB & (1 << TXEN)) {
    /* Seems like we jumped here from firmware.
     * We will not reinitialize UART to keep the
     * same BAUD rate, but we have to flush it. */
    sei();
    while (UCSRB & (1 << UDRIE));
    while (!(UCSRA & (1 << UDRE)));
    while (!(UCSRA & (1 << TXC)));
  } else {
    #include <util/setbaud.h>
    UBRRH = UBRRH_VALUE;
    UBRRL = UBRRL_VALUE;
    #if USE_2X
    UCSRA |=  (1 << U2X);
    #else
    UCSRA &= ~(1 << U2X);
    #endif
    UCSRC = (1 << URSEL) | (3 << UCSZ0);
    UCSRB = (1 << RXCIE) | (1 << RXEN) | (1 << TXEN);
    while (!(UCSRA & (1 << UDRE)));
  }
  
  cli();
  put(ACK); // inform host we are up

  wdt_enable(WDTO_2S);

  uint8_t buf [SPM_PAGESIZE];
  uintptr_t size = getput_word();

  bool jump_written = 0;

  uintptr_t page_adr = DIV_CEIL(size, SPM_PAGESIZE) * SPM_PAGESIZE;
  pptr_t len = size % SPM_PAGESIZE;
  if (len == 0) len = SPM_PAGESIZE;
  uint8_t * buf_ptr = &buf[len];

  while (page_adr) {
    page_adr -= SPM_PAGESIZE;

    while (buf_ptr > &buf[0]) {
      buf_ptr--;
      *buf_ptr = getput();
    }

    if (get() != ACK) {
      put(NACK);
      restart();
    }
    
    if (!jump_written) {
      /* we are about to start writting - let's make sure 
       * we will wake back into bootloader in case we fail */
      jump_written = 1;
      extern uint8_t __jmp_load_start;
      extern uint8_t __jmp_load_end;
      uintptr_t len = &__jmp_load_end - &__jmp_load_start;
      uint8_t buf_jmp [len];
      for (uintptr_t i = 0; i < len; i++)
        buf_jmp[i] = pgm_read_byte(&__jmp_load_start + i);
      flash_write_block(&buf_jmp[0], 0, len);
    }
    
    flash_write_block(&buf[0], page_adr, len);
    
    len = SPM_PAGESIZE;
    buf_ptr = &buf[SPM_PAGESIZE];
    
    put(ACK);
  }

  put(ACK);
  restart();
}

/* jump to bootloader - replacement for reset vector after we start writting */
__attribute__((naked, used))
__attribute__((section(".jmp")))
void jmp() 
{
  extern void __vectors();
  __vectors();
}

