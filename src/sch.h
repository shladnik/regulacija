#ifndef __SCH_H__
#define __SCH_H__

#define QUEUE_LEN 12

extern volatile func_t sch_queue[QUEUE_LEN];
extern uint8_t sch_wp;
extern uint8_t sch_rp;

void sch_add(func_t func);
void sch();

#endif
