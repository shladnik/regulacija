#ifndef __PRINT_H__
#define __PRINT_H__ 1

void print_buf_init();
#if PLAIN_CONSOLE
extern bool print_buf_empty();
extern bool    print_buf_block;
extern uint8_t print_buf_ovf;
char print_buf_read();
#endif

#endif
