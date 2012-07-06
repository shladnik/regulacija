#define TX_QUEUE_LEN 2

pac_state_t rx_state = ADR_SIZE;
pac_t       rx;
uint8_t     rx_crc = 0;
uint8_t     rx_buf [16];
bool        rx_timer = 0;

pac_state_t tx_state = ADR_SIZE;
pac_t       tx_queue [TX_QUEUE_LEN];
uint8_t     tx_crc = 0;

static uint8_t wp;
static uint8_t rp;

static uint8_t pinc(uint8_t p)
{
  p++;
  if (p >= TX_QUEUE_LEN) p = 0;
  return p;
}

void send(pac_t p)
{
#ifndef NDEBUG
  if (wp == rp && (UCSR0B & (1 << UDRIE0))) DBG_CNT_CLIP(tx_busy);
#endif
  while (wp == rp && (UCSR0B & (1 << UDRIE0)));
  tx_queue[wp] = p;
  
  DBG_ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    wp = pinc(wp);
    UCSR0B |= 1 << UDRIE0;
  }
}


void rx_reset()
{
  rx_state = ADR_SIZE;
  rx_crc = 0;
  UCSR0B &= ~(1 << RXEN0);
  UCSR0B |=  (1 << RXEN0);
}

void rx_timeout()
{
  rx_timer = 0;
  if (!(UCSR0A & (1 << RXC0))) {
#ifndef NDEBUG
    DBG_CNT_CLIP(rx_timeout_cnt);
#endif
    rx_reset();
  }
}
const sch_t rx_timeout_sch = { rx_timeout, 0, (uint8_t)-1 };

DBG_ISR(USART_RX_vect, ISR_BLOCK)
{
#if PLAIN_CONSOLE
  //assert(0);
  UDR0 = UDR0;
#else
  if (rx_timer) {
    rx_timer = 0;
    timer_cancel(rx_timeout_sch, 1); //assert
  }

  bool err = 0;

  if (UCSR0A & ((1<<FE0)|(1<<DOR0)|(1<<UPE0))) {
#ifndef NDEBUG
    DBG_CNT_CLIP(rx_ovf);
#endif
    err = 1;
  } else {
    uint8_t byte = UDR0;
    rx_crc = _crc_ibutton_update(rx_crc, byte);
    static uint8_t i;
 
    switch (rx_state) {
      case ADR_SIZE: {
        if ((byte & 0x7f) <= 2) {
          rx.write   = byte & 0x80;
          rx.adr_len = byte & 0x03;
          rx.adr = 0;
          i = rx.adr_len - 1;
          rx_state = rx.adr_len ? ADR : DAT_SIZE;
        } else {
          err = 1;
        }
        break;
      }
      case ADR: {
        uint8_t * adr_p = (uint8_t *)(&rx.adr);
        if (i) {
          adr_p[1] = byte;
          i = 0;
        } else {
          adr_p[0] = byte;
          rx_state = DAT_SIZE;
        }
        break;
      }
      case DAT_SIZE: {
        rx.dat_len = byte;
        if (rx.write) {
          if (rx.dat_len > sizeof(rx_buf)) {
            err = 1;
          } else {
            i = 0;
            rx_state = DAT;
          }
        } else {
          rx_state = CRC;
        }
        break;
      }
      case DAT: {
        rx_buf[i++] = byte;
        if (i >= rx.dat_len) rx_state = CRC;
        break;
      }
      default: {
        if (rx_crc) {
          err = 1;
        } else {
          if (rx.write) { // TODO limit writeable space?
            for (uint8_t i = 0; i < rx.dat_len; i++) {
              *(rx.adr+i) = rx_buf[i];
            }
          }
          
          pac_t response = rx;
          response.write = response.write ? 0 : 1;
          send(response);
          rx_state = ADR_SIZE;
        }
        break;
      }
    }
  }
 
  if (err) {
#ifndef NDEBUG
    DBG_CNT_CLIP(rx_err);
#endif
    rx_reset();
  } else if (rx_state != ADR_SIZE && !(UCSR0A & (1 << RXC0))) {
    rx_timer = 1;
    timer_add(TIMER_MS(100), rx_timeout_sch);
  }
#endif
}

DBG_ISR(USART_UDRE_vect, ISR_BLOCK)
{
#if PLAIN_CONSOLE
  if (print_buf_empty()) {
    if (print_buf_ovf) {
      uint8_t ovf = print_buf_ovf;
      print_buf_ovf = 0;
      printf("\n\n# LOST >= %d #\n\n", ovf);
    } else {
      UCSR0B &= ~(1 << UDRIE0);
    }
  } else
    UDR0 = print_buf_read();
#else
  uint8_t byte;

  switch (tx_state) {
    case ADR_SIZE: {
      byte = tx_queue[rp].adr_len;
      if (tx_queue[rp].write) byte |= 0x80;
      tx_state = tx_queue[rp].adr_len ? ADR : DAT_SIZE;
      break;
    }
    case ADR: {
      tx_queue[rp].adr_len--;
      uint8_t * adr_ptr = (uint8_t *)(&tx_queue[rp].adr);
      byte = adr_ptr[tx_queue[rp].adr_len];
      if (tx_queue[rp].adr_len == 0) tx_state = DAT_SIZE;
      break;
    }
    case DAT_SIZE: {
      byte = tx_queue[rp].dat_len;
      tx_state = tx_queue[rp].write && tx_queue[rp].dat_len ? DAT : CRC;
      break;
    }
    case DAT: {
      tx_queue[rp].dat_len--;
      byte = *tx_queue[rp].adr;
      tx_queue[rp].adr++;
      if (tx_queue[rp].dat_len == 0) tx_state = CRC;
      break;
    }
    default: { // CRC
      byte = tx_crc;
      tx_state = ADR_SIZE;

      rp = pinc(rp);
      if (wp == rp) UCSR0B &= ~(1 << UDRIE0);
      break;
    }
  }

  tx_crc = _crc_ibutton_update(tx_crc, byte);
  UDR0 = byte;
#endif
}

void uart_init()
{
  #include <util/setbaud.h>
  UBRR0H = UBRRH_VALUE;
  UBRR0L = UBRRL_VALUE;
#if USE_2X
  UCSR0A |=  (1 << U2X0);
#else
  UCSR0A &= ~(1 << U2X0);
#endif
 
#if __AVR_ATmega32__
  UCSRC = (1 << URSEL) | (3 << UCSZ0);
#else
  UCSR0C = (3 << UCSZ00);
#endif
#if 0 //PLAIN_CONSOLE
  UCSR0B = (1 << TXEN0);
#else
  UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
#endif
}
