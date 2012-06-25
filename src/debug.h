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

#define DBG_CNT(d)              do { DBG    static uint8_t d; d++;                        } while(0)
#define DBG_CNT_CLIP(d)         do { DBG    static uint8_t d; if (d < (typeof(d))-1) d++; } while(0)
#define DBG_VAR(d, v)           do { DBG    static typeof(v) d; d = v;                    } while(0)
#define DBG_MAX(d, v)           do { DBG    static typeof(v) d; d = MAX(v, d);            } while(0)
#define DBG_LOG(d, v, l)        do { DBG    static typeof(v) d [l]; memmove(&d, &d[1], sizeof(d) - sizeof(d[0])); d[l-1] = v; } while(0)
#define DBG_LOG_FINITE(d, v, l) do { DBG    static typeof(v) d [l]; DBG static uint8_t i = 0; if (i < l) { d[i] = v; i++; }   } while(0)
#define DBG2CP_VAR(d, v)        do { DBG2CP static typeof(v) d; d = v;                    } while(0)
#define DBG2CP_LOG(d, v, l)     do { DBG2CP static typeof(v) d [l]; memmove(&d, &d[1], sizeof(d) - sizeof(d[0])); d[l-1] = v; } while(0)

void log_adr();

#define DBG_COPY(name) \
  DBG static typeof(name) name##_cp; \
  memcpy(&name##_cp, &name, sizeof(name));


  extern void __assert();
  #define assert(e) ((e) ? (void)0 : __assert())
  
#if 1
  #define DBG_ISR(vector, ...) \
    void vector##_real();      \
    ISR(vector, __VA_ARGS__) { \
      log_adr();               \
      vector##_real();         \
      log_adr();               \
    }                          \
    void vector##_real()
#else
  #define DBG_ISR           ISR
#endif

#define DBG_ATOMIC_BLOCK ATOMIC_BLOCK // TODO

#endif
