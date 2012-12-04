#ifndef __PRINT_H__
#define __PRINT_H__ 1

#define PRINT_BUF_SIZE 0

void print_buf_init();
extern bool print_buf_empty();
extern bool    print_buf_block;
extern uint8_t print_buf_ovf;
char print_buf_read();

#endif
