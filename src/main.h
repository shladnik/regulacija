#ifndef __MAIN_H__
#define __MAIN_H__

typedef void (*func_t)();

#define MIN(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) < (_y) ? (_x) : (_y)); }))
#define MAX(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) > (_y) ? (_x) : (_y)); }))
#define ABS(x)   (__extension__({ typeof (x) _x = x; (_x < 0 ? -_x : _x); }))
#define CLIP(x,min,max) MAX(min, MIN(x, max))

#define DIV_FLOOR(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x)                    ) / (_y); }))
#define DIV_ROUND(x,y) (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) + (((_y) + 1) >> 1)) / (_y); }))
#define DIV_CEIL(x,y)  (__extension__({ typeof (x) _x = x; typeof(y) _y = y; ((_x) + (_y) - 1         ) / (_y); }))

#endif
