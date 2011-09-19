#ifndef __EXEXEC_H__
#define __EXEXEC_H__

typedef uint64_t (*exexec_func_t)(uint64_t args);
extern exexec_func_t exexec_func;

void exexec();

#endif
