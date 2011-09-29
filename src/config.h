#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "config_list.h"

#define CONFIG_READ(dst, src) typeof(src) dst; flash_read((uint8_t *)&dst, (uintptr_t)&src, sizeof(src))

#endif
