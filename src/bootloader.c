#if   SPM_PAGESIZE < (1 <<  8)
typedef uint8_t  pptr_t;
#elif SPM_PAGESIZE < (1 << 16)
typedef uint16_t pptr_t;
#else
#error
#endif

static const uint8_t ACK  = 0xa5;
static const uint8_t NACK = 0x00;

void restart()
{
  while (!(UCSR0A & (1 << UDRE0)));
  while (!(UCSR0A & (1 << TXC0)));
  wdt_enable(0);
  while(1);
}

uint8_t get()
{
  wdt_reset();
  while (!(UCSR0A & (1 << RXC0)));
  return UDR0;
}

void put(uint8_t dat)
{
  while (!(UCSR0A & (1 << UDRE0)));

  if (UCSR0A & ((1<<FE0)|(1<<DOR0)|(1<<UPE0))) {
    UDR0 = NACK;
    restart();
  } else {
    UDR0 = dat;
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

void flash_write_block(uint8_t * buf, uintptr_t adr, uintptr_t len, uint8_t check)
{
  uintptr_t start = adr;
  uintptr_t end   = adr + len;
  uintptr_t i = ~(SPM_PAGESIZE - 1) & start;
  uintptr_t e = i + SPM_PAGESIZE;
  uintptr_t end_a   = end   & ~0x1;

  eeprom_busy_wait();
  boot_spm_busy_wait();

  bool change = 0;

#if 0 // This is not needed for bootloader since it always writes blocks from the start
  uintptr_t start_a = start & ~0x1;
  for (; i < start_a; i += 2) boot_page_fill(i, pgm_read_word(i));
  if (start & 0x1) {
    uint16_t dat_old = pgm_read_word(i);
    uint16_t dat = (dat_old & 0x00ff) | (*buf << 8);
    change |= dat != dat_old;
    buf++;
    boot_page_fill(i, dat);
    i += 2;
  }
#endif
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
    if (boot_page_erase_checked(adr, check) != 0) restart();
    boot_spm_busy_wait();
    if (boot_page_write_checked(adr, check) != 0) restart();
    boot_spm_busy_wait();
  }
  boot_rww_enable();
}

__attribute__((naked))
int main()
{
  volatile uint8_t check = BOOT_SPM_CHECK_VAL;

  if (UCSR0B & (1 << TXEN0)) {
    /* Seems like we jumped here from firmware.
     * We will not reinitialize UART to keep the
     * same BAUD rate, but we have to flush it. */
    sei();
    while (UCSR0B & (1 << UDRIE0));
    while (!(UCSR0A & (1 << UDRE0)));
    while (!(UCSR0A & (1 << TXC0)));
  } else {
    #include <util/setbaud.h>
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
    #if USE_2X
    UCSR0A |=  (1 << U2X0);
    #else
    UCSR0A &= ~(1 << U2X0);
    #endif
    UCSR0C = (3 << UCSZ00);
    UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
    while (!(UCSR0A & (1 << UDRE0)));
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
      flash_write_block(&buf_jmp[0], 0, len, check);
    }
    
    flash_write_block(&buf[0], page_adr, len, check);
    
    len = SPM_PAGESIZE;
    buf_ptr = &buf[SPM_PAGESIZE];
    
    put(ACK);
  }
  
  /* bootloader sign, so fw knows that it was valid restart */
  PROGMEM static const uint8_t sign [] = { 'b', 'o', 'o', 't', 'l', 'o', 'a', 'd' };
  extern uint8_t __data_start;
  memcpy_P(&__data_start, sign, sizeof(sign));

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

