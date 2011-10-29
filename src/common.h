#ifndef __COMMON_H__
#define __COMMON_H__

#define NOINIT __attribute__((section(".noinit")))
#define META   __attribute__((section(".meta")))
#define CONFIG __attribute__((section(".config")))
#define USED   __attribute__((used))
#define INLINE __attribute__((always_inline))

typedef void (*func_t)();

#define MIN(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) < (_y) ? (_x) : (_y)); }))
#define MAX(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) > (_y) ? (_x) : (_y)); }))
#define ABS(x)   (__extension__({ typeof (x) _x = x; (_x < 0 ? -_x : _x); }))
#define CLIP(x,min,max) MAX(min, MIN(x, max))

#define DIV_FLOOR(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x)                    ) / (_y); }))
#define DIV_ROUND(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) + (((_y) + 1) >> 1)) / (_y); }))
#define DIV_CEIL(x,y)  (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) + (_y) - 1         ) / (_y); }))

#if 1
#define LAZY(code) () { return (code); }
#define LAZY_GET(name) (name())
#else
#define LAZY(code) = (code)
#define LAZY_GET(name) (name)
#endif

#endif
