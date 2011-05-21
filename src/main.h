#ifndef __MAIN_H__
#define __MAIN_H__

#define DBG16_SIZE 5
#define DBG32_SIZE 16
#if DBG16_SIZE
extern uint16_t dbg16 [DBG16_SIZE] __attribute__ ((section (".noinit")));
#endif
#if DBG32_SIZE
extern uint32_t dbg32 [DBG32_SIZE] __attribute__ ((section (".noinit")));
#endif

#ifdef NDEBUG
#define assert(e)	((void)0)
#else
extern void __assert();
#define assert(e) ((e) ? (void)0 : __assert())
#endif

#define MIN(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) < (_y) ? (_x) : (_y)); }))
#define MAX(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) > (_y) ? (_x) : (_y)); }))
#define ABS(x)   (__extension__({ typeof (x) _x = x; (_x < 0 ? -_x : _x); }))
#define CLIP(x,min,max) MAX(min, MIN(x, max))

#define DIV_FLOOR(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x)                    ) / (_y); }))
#define DIV_ROUND(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) + (((_y) + 1) >> 1)) / (_y); }))
#define DIV_CEIL(x,y)  (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) + (_y) - 1         ) / (_y); }))


#endif
