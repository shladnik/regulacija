#define DBG    __attribute__((used, section(".dbg")))
#define DBG2CP __attribute__((used, section(".dbg2cp")))

#ifdef NDEBUG
  #define DBG_CNT(d)
  #define DBG_VAR(d, v)
  #define DBG2CP_VAR(d, v)
  #define assert(e)	        ((void)0)
  #define DBG_COPY(name)    ((void)(name))
  #define DBG_ISR           ISR
  #define DBG_ATOMIC_BLOCK  ATOMIC_BLOCK
#else

#define DBG_CNT(d)       do { DBG    static uint8_t d; d++;     } while(0)
#define DBG_VAR(d, v)    do { DBG    static typeof(v) d; d = v; } while(0)
#define DBG2CP_VAR(d, v) do { DBG2CP static typeof(v) d; d = v; } while(0)

extern func_t last_isr;
extern volatile void * last_adr;
extern void * last_assert;
extern func_t  isr_max;
void log_adr();

#define DBG_COPY(name) \
  DBG static typeof(name) name##_cp; \
  memcpy(&name##_cp, &name, sizeof(name));


  extern void __assert();
  #define assert(e) ((e) ? (void)0 : __assert())
  
#if 0
  #define DBG_ISR(vector, ...) \
    void vector##_real(); \
    ISR(vector, __VA_ARGS__) { \
      extern ISR(TIMER1_OVF_vect); \
      timer_t start = vector == TIMER1_OVF_vect ? timer_now() + 0x10000 : timer_now(); \
      log_adr(); \
      last_isr = vector; \
      vector##_real(); \
      last_isr = 0; \
      timer_t end = timer_now(); \
      timer_t time = end - start; \
      extern timer_t isr_max_time; \
      if (time > isr_max_time) { \
        isr_max_time = time; \
        isr_max = vector; \
      } \
    } \
    void vector##_real()
#else
  #define DBG_ISR           ISR
#endif

#define DBG_ATOMIC_BLOCK ATOMIC_BLOCK // TODO

#endif
