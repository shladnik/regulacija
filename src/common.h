#ifndef __COMMON_H__
#define __COMMON_H__

#if __AVR_ATmega32__
#define USART_RX_vect USART_RXC_vect
#define UCSR0A  UCSRA
#define UDRE0   UDRE
#define TXC0    TXC
#define RXC0    RXC
#define UDR0    UDR
#define FE0     FE
#define DOR0    DOR
#define UPE0    PE
#define UCSR0B  UCSRB
#define TXEN0   TXEN
#define RXEN0   RXEN
#define UDRIE0  UDRIE
#define UBRR0H  UBRRH
#define UBRR0L  UBRRL
#define U2X0    U2X
#define UCSR0C  UCSRC
#define RXCIE0  RXCIE
#define UCSZ00  UCSZ0

#define TCCR0A  TCCR0
#define TCCR0B  TCCR0
#define TIFR0   TIFR
#define TIMSK0  TIMSK
#define TIFR1   TIFR
#define TIMSK1  TIMSK
#define TCCR2A  TCCR2
#define TCCR2B  TCCR2
#define TIFR2   TIFR
#define TIMSK2  TIMSK
#endif

#define NOINIT __attribute__((section(".noinit")))
#define USED   __attribute__((used))
#define INLINE __attribute__((always_inline))

typedef void (*func_t)();

#define MIN(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) < (_y) ? (_x) : (_y)); }))
#define MAX(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) > (_y) ? (_x) : (_y)); }))
#define ABS(x)   (__extension__({ typeof (x) _x = x; (_x < 0 ? -_x : _x); }))
#define CLIP(x,min,max) MAX(min, MIN(x, max))

#define DIV_FLOOR(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x)                    ) / (_y); }))
#define DIV_ROUND(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) + (((_y) + 1) >> 1)) / (_y); }))
#define DIV_CEIL(x,y)  (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) + (_y) - 1         ) / (_y); }))

#if 1
#define LAZY(code) () { return (code); }
#define LAZY_GET(name) (name())
#else
#define LAZY(code) = (code)
#define LAZY_GET(name) (name)
#endif

#define BOOT_SPM_CHECK_VAL 0xa5
uint8_t boot_page_erase_checked (uint16_t address, uint8_t check);
uint8_t boot_page_write_checked (uint16_t address, uint8_t check);

#endif
