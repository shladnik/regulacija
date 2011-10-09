#ifndef __UART_H__
#define __UART_H__ 1

typedef enum {
  ADR_SIZE,
  ADR     ,
  DAT_SIZE,
  DAT     ,
  CRC     ,
} pac_state_t;

typedef struct {
  bool        write;
  uint8_t     adr_len;
  uint8_t *   adr;
  uint8_t     dat_len;
} pac_t;

void uart_init();
void send(pac_t p);

#endif
