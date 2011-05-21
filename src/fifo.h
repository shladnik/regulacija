
#define FIFO(NAME, TYPE, SIZE, VOLATILE_W, VOLATILE_R) \
typedef uint8_t NAME##_p; \
TYPE NAME [SIZE]; \
VOLATILE_W NAME##_p NAME##_wp = 0; \
VOLATILE_R NAME##_p NAME##_rp = 0; \
void NAME##_fill(); \
\
NAME##_p NAME##_level()\
{\
  return NAME##_wp >= NAME##_rp ? NAME##_wp - NAME##_rp :\
                           SIZE + NAME##_wp - NAME##_rp;\
}\
\
NAME##_p NAME##_space()\
{\
  return SIZE - 1 - NAME##_level();\
}\
\
bool NAME##_empty()\
{\
  return NAME##_rp == NAME##_wp;\
}\
\
bool NAME##_full()\
{\
  return NAME##_level() == SIZE - 1;\
}\
\
NAME##_p NAME##_pinc(NAME##_p p)\
{\
  p += 1;\
  if (p >= SIZE) p -= SIZE;\
  return p;\
}\
\
void NAME##_write(TYPE b)\
{\
  NAME[NAME##_wp] = b;\
  NAME##_wp = NAME##_pinc(NAME##_wp);\
}\
\
TYPE NAME##_read()\
{\
  TYPE r = NAME[NAME##_rp];\
  NAME##_rp = NAME##_pinc(NAME##_rp);\
  return r;\
}\
\
TYPE NAME##_peek(NAME##_p i)\
{\
  if (i < SIZE - NAME##_rp)\
    i += NAME##_rp;\
  else\
    i -= SIZE - NAME##_rp;\
  return NAME[i];\
}\

