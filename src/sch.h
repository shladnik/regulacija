#ifndef __SCH_H__
#define __SCH_H__

typedef void (*sch_func_t)(void * arg);

typedef struct {
  volatile sch_func_t func;
  void * arg;
  uint8_t level;
} sch_t;

void sch_add(sch_t e);
void sch();

#endif
