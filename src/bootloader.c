#if   SPM_PAGESIZE < (1 <<  8)
typedef uint8_t  pptr_t;
#elif SPM_PAGESIZE < (1 << 16)
typedef uint16_t pptr_t;
#else
#error
#endif

const uint8_t ACK  = 0xa5;
const uint8_t NACK = 0x5a;

BOOTLOADER_SECTION void bootloader_restart()
{
  while (!(UCSRA & (1 << UDRE)));
  while (!(UCSRA & (1 << TXC)));
#if 1
  wdt_enable(0);
  while(1);
#else
  asm("call %0" : : "n" (0x0000));
#endif
}

BOOTLOADER_SECTION uint8_t bootloader_get()
{
  wdt_reset();
  while (!(UCSRA & (1 << RXC)));
  return UDR;
}

BOOTLOADER_SECTION void bootloader_put(uint8_t dat)
{
  while (!(UCSRA & (1 << UDRE)));

  if (UCSRA & ((1<<FE)|(1<<DOR)|(1<<PE))) {
    UDR = NACK;
    bootloader_restart();
  } else {
    UDR = dat;
  }
}

BOOTLOADER_SECTION uint8_t bootloader_getput()
{
  uint8_t b = bootloader_get();
  bootloader_put(b);
  return b;
}

BOOTLOADER_SECTION uint16_t bootloader_getput_word()
{
  uint16_t dat;
  uint8_t * dat_p = (uint8_t *)(&dat);
  dat_p[1] = bootloader_getput();
  dat_p[0] = bootloader_getput();
  return dat;
}

/* jump to bootjmp - replacement for reset vector after we start writting */
__attribute__((naked))
BOOTLOADER_SECTION void bootloader_jmp() 
{
  asm("call %0" : : "n" (0x7000));
}

__attribute__((used, naked, section((".bootjmp"))))
void bootjmp()
{
  bootloader();
}

USED __attribute__((section(".writeallow"))) void bootloader()
{
  if (UCSRB & (1 << TXEN)) { // flush USART
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
  bootloader_put(ACK); // inform host we are up

  wdt_enable(WDTO_2S);

  uint8_t buf [SPM_PAGESIZE];
  uintptr_t size = bootloader_getput_word();

  bool jump_written = 0;

  uintptr_t page_adr = DIV_CEIL(size, SPM_PAGESIZE) * SPM_PAGESIZE;
  pptr_t len = size % SPM_PAGESIZE;
  if (len == 0) len = SPM_PAGESIZE;
  uint8_t * buf_ptr = &buf[len];

  while (page_adr) {
    page_adr -= SPM_PAGESIZE;

    while (buf_ptr > &buf[0]) {
      buf_ptr--;
      *buf_ptr = bootloader_getput();
    }

    if (bootloader_get() != ACK) {
      bootloader_put(NACK);
      bootloader_restart();
    }
    
    if (!jump_written) {
      /* we are about to start writting - let's make sure 
       * we will wake back into bootloader in case we fail */
      jump_written = 1;
      uint32_t jmp = pgm_read_dword((uintptr_t)&bootloader_jmp << 1);
      flash_write_block((uint8_t *)&jmp, 0, sizeof(jmp)); 
    }
    
    flash_write_block(&buf[0], page_adr, len);
    
    len = SPM_PAGESIZE;
    buf_ptr = &buf[SPM_PAGESIZE];
    
    bootloader_put(ACK);
  }

  bootloader_put(ACK);
  bootloader_restart();
}
