typedef struct {
  const sch_t func;
  const timer_t period;
} loop_t;

const loop_t loops [] PROGMEM = {
#ifdef LOOP_CLOCK
  { { clock_loop      , 0, -1 }, TIMER_S(CLOCK_LOOP_PERIOD) }, // TODO: use some priority between highest & lowest
#endif
  { { watchdog_reseter, 0, -1 }, WATCHDOG_RESETER_PERIOD    },
  { { syscheck_loop   , 0,  0 }, TIMER_S(14)                },
  { { lcd_loop        , 0,  0 }, TIMER_S(11)                },
  { { pumping_loop    , 0,  0 }, TIMER_MIN(1)               },
  { { collector_loop  , 0,  0 }, TIMER_S(30)                },
  { { furnace_loop    , 0,  0 }, TIMER_S(10)                },
  { { radiator_loop   , 0,  0 }, TIMER_S(10)                },
};

#define LOOP_NR (sizeof(loops)/sizeof(loop_t))

static timer_t loop_next [LOOP_NR];

static void loop_handler(void * arg)
{
  uint8_t i = (uint16_t)arg;
  loop_t l; memcpy_P(&l, &loops[i], sizeof(loop_t));
  timer_t prev = loop_next[i];
  
  sch_add(l.func);

  timer_t next = prev + l.period;
  timer_add_cmp(next, (sch_t){ loop_handler, arg, (typeof(l.func.level))-1 });
  assert(in_range(prev, timer_now(), next));
  loop_next[i] = next;
}

void loops_start()
{
  for (uint8_t i = 0; i < LOOP_NR; i++) sch_add((sch_t){ loop_handler, (void *)(uintptr_t)i, (uint8_t)-1 });
}
