#ifndef __FLASH_H__
#define __FLASH_H__

void flash_write_block(uint8_t * buf, uintptr_t adr, uintptr_t len);
void flash_write(uint8_t * buf, uintptr_t adr, uintptr_t len);
void flash_read (uint8_t * buf, uintptr_t adr, uintptr_t len);

void pgm_read(uint8_t * buf, uintptr_t adr, uintptr_t len);
#define PGM_GET(src) __extension__(({ typeof(src) dst; pgm_read((uint8_t *)&(dst), (uintptr_t)&(src), sizeof(src)); dst; }))
#endif
