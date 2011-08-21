typedef struct {
  const func_t func;
  const timer_t dly;
} loop_t;

const loop_t loop [] PROGMEM = {
//  { console_loop        , TIMER_S(10)  },
  { lcd_loop            , TIMER_S(10)  },
//  { lcd_heartbeat       , TIMER_S(1)   },
  { pumping_loop        , TIMER_MIN(1) },
  { collector_loop      , TIMER_S(30)  },
  { furnace_loop        , TIMER_S(10)  },
//  { radiator_goal_loop  , TIMER_MIN(5) },
  { radiator_loop       , TIMER_S(10)  },
//  { stack_check         , TIMER_MIN(1) },
};

#define LOOP_NR (sizeof(loop)/sizeof(loop_t))

static timer_t loop_next [LOOP_NR];

static void loop_handler(void * arg)
{
  uint8_t i = (uint16_t)arg;
  sch_add((func_t)pgm_read_word(&loop[i].func));
  timer_t next = loop_next[i] + pgm_read_dword(&loop[i].dly);
  timer_add_cmp(next, loop_handler, arg, -1);
  loop_next[i] = next;
}

void loops_start()
{
  for (uint8_t i = 0; i < LOOP_NR; i++) loop_handler((void *)(uint16_t)i);
}

void loops_check()
{
  assert(LOOP_NR == timer_count(loop_handler));
}
