#ifndef __MAIN_H__
#define __MAIN_H__

typedef void (*func_t)();

extern func_t last_isr;
extern func_t last_sch_func;
extern func_t last_timer_func;
extern volatile void * last_adr;
extern void * last_assert;
extern func_t  isr_max;
extern timer_t isr_max_time;
void log_adr();

#define DBG    __attribute__((section(".dbg")))

#ifdef NDEBUG
  #define assert(e)	((void)0)
#else
  extern void __assert();
  #define assert(e) ((e) ? (void)0 : __assert())
#endif

#ifdef NDEBUG
  #define DBG_ISR ISR
  #define DBG_ATOMIC_BLOCK ATOMIC_BLOCK
#else
  #define DBG_ISR ISR
  //#define DBG_ISR(vector, ...) \
  //  void vector##_real(); \
  //  ISR(vector, __VA_ARGS__) { \
  //    extern ISR(TIMER1_OVF_vect); \
  //    timer_t start = vector == TIMER1_OVF_vect ? timer_now() + 0x10000 : timer_now(); \
  //    log_adr(); \
  //    last_isr = vector; \
  //    vector##_real(); \
  //    last_isr = 0; \
  //    timer_t end = timer_now(); \
  //    timer_t time = end - start; \
  //    if (time > isr_max_time) { \
  //      isr_max_time = time; \
  //      isr_max = vector; \
  //    } \
  //  } \
  //  void vector##_real()

  #define DBG_ATOMIC_BLOCK ATOMIC_BLOCK // TODO
#endif

#define MIN(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) < (_y) ? (_x) : (_y)); }))
#define MAX(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) > (_y) ? (_x) : (_y)); }))
#define ABS(x)   (__extension__({ typeof (x) _x = x; (_x < 0 ? -_x : _x); }))
#define CLIP(x,min,max) MAX(min, MIN(x, max))

#define DIV_FLOOR(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x)                    ) / (_y); }))
#define DIV_ROUND(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) + (((_y) + 1) >> 1)) / (_y); }))
#define DIV_CEIL(x,y)  (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) + (_y) - 1         ) / (_y); }))

#endif
