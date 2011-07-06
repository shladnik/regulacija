#ifndef __RELAY_H__
#define __RELAY_H__

typedef enum
{
#include "relay_list.h"
} RELAY;

void relay_on(RELAY i);
void relay_off(RELAY i);
bool relay_get(RELAY i);
void relay_toggle(RELAY i);
void relay_off_all();
void relay_on_all();

#endif
