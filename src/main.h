#ifndef __MAIN_H__
#define __MAIN_H__

typedef void (*func_t)();
extern func_t last_isr;
extern func_t last_sch_func;
extern func_t last_timer_func;
extern void * last_assert;
void log_adr();

#define DBG    __attribute__((section(".dbg")))
#define NOINIT __attribute__((section(".noinit")))

#ifdef NDEBUG
  #define assert(e)	((void)0)
#else
  extern void __assert();
  #define assert(e) ((e) ? (void)0 : __assert())
#endif

#ifdef NDEBUG
  #define DBG_ISR ISR
#else
  #define DBG_ISR(vector, ...) \
    void vector##_real(); \
    ISR(vector, __VA_ARGS__) { \
      last_isr = vector; \
      vector##_real(); \
      last_isr = 0; \
    } \
    void vector##_real()
#endif

      //extern void log_isr();
      //log_isr();
#define MIN(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) < (_y) ? (_x) : (_y)); }))
#define MAX(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) > (_y) ? (_x) : (_y)); }))
#define ABS(x)   (__extension__({ typeof (x) _x = x; (_x < 0 ? -_x : _x); }))
#define CLIP(x,min,max) MAX(min, MIN(x, max))

#define DIV_FLOOR(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x)                    ) / (_y); }))
#define DIV_ROUND(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) + (((_y) + 1) >> 1)) / (_y); }))
#define DIV_CEIL(x,y)  (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) + (_y) - 1         ) / (_y); }))

#endif
