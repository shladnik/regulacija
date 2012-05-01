#ifndef __CONFIG_H__
#define __CONFIG_H__

#define CONFIG __attribute__((section(".config")))
#include "config_list.h"
#define CONFIG_GET(src) __extension__(({ typeof(src) dst; flash_read((uint8_t *)&(dst), (uintptr_t)&(src), sizeof(src)); dst; }))
#define CONFIG_SET(dst, val) flash_write((uint8_t *)&(val), (uintptr_t)&(dst), sizeof(val))

#endif
