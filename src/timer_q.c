#define MAX_TIMERS 16 /* also used as invalid value */
#define TIMER_DEBUG 1
typedef uint8_t ptr_t;

typedef struct {
  ptr_t next;
  timer_t cmp;
  void (*func)();
  void * arg;
  uint8_t level;
} slot_t;

static slot_t slot [MAX_TIMERS] __attribute__ ((section (".noinit")));
#if TIMER_DEBUG
static ptr_t first __attribute__ ((section (".noinit")));
#if TIMER_DEBUG > 1
static slot_t slot0 [MAX_TIMERS/2] __attribute__ ((section (".noinit")));
static ptr_t first0 __attribute__ ((section (".noinit")));
static timer_t now0 __attribute__ ((section (".noinit")));
static timer_t cmp0 __attribute__ ((section (".noinit")));
#endif
#if TIMER_DEBUG > 2
static slot_t slot1 [MAX_TIMERS/2] __attribute__ ((section (".noinit")));
static ptr_t first1 __attribute__ ((section (".noinit")));
static timer_t now1 __attribute__ ((section (".noinit")));
static timer_t cmp1 __attribute__ ((section (".noinit")));
#endif
#else
static ptr_t first = MAX_TIMERS;
#endif

#if TIMER_DEBUG
void timer_print()
{
  //printf("TIMSK %x TIFR %x (OCIE1A %x TOIE %x)\n", TIMSK, TIFR, 1 << OCIE1A, 1 << TOIE1);
#if TIMER_DEBUG > 2
  printf("---1---\n");
  printf("%x %lx %lx\n", first1, now1, cmp1);
  for (ptr_t i = 0; i < MAX_TIMERS/2; i++) {
    if (slot1[i].next != MAX_TIMERS) printf("%x %x %lx %x\n", i, slot1[i].next, slot1[i].cmp, (uint16_t)(slot1[i].func));
  }
#endif
#if TIMER_DEBUG > 1
  printf("---0---\n");
  printf("%x %lx %lx\n", first0, now0, cmp0);
  for (ptr_t i = 0; i < MAX_TIMERS/2; i++) {
    if (slot0[i].next != MAX_TIMERS) printf("%x %x %lx %x\n", i, slot0[i].next, slot0[i].cmp, (uint16_t)(slot0[i].func));
  }
  printf("---*---\n");
#endif
  printf("%x %lx\n", first, timer_now());
  for (ptr_t i = 0; i < MAX_TIMERS; i++) {
    if (slot[i].next != MAX_TIMERS) printf("%x %x %lx %x\n", i, slot[i].next, slot[i].cmp, (uint16_t)(slot[i].func));
  }
}
#endif

void timer_init()
{
#if TIMER_DEBUG
  timer_print();
  first = MAX_TIMERS;
#if TIMER_DEBUG > 1
  first0 = MAX_TIMERS;
#endif
#if TIMER_DEBUG > 2
  first1 = MAX_TIMERS;
#endif
#endif
  for (ptr_t i = 0; i < MAX_TIMERS; i++)
    slot[i].next = MAX_TIMERS;
  timer_start();
}

void timer_int()
{
  slot_t c = slot[first];
  timer_t now = timer_now();
  dbg32[0] = now;
  dbg16[0] = (uintptr_t)c.func;
  dbg32[2] = MAX(dbg32[4], now - c.cmp);
  slot[first].next = MAX_TIMERS;
 
  if (c.next == first) {
    first = MAX_TIMERS;
    timer_unset();
  } else {
    first = c.next;
    timer_set(c.cmp, slot[first].cmp);
  }
  
  if (c.level == (uint8_t)(-1)) {
    c.func(c.arg);
  } else {
    sch_add(c.func);
  }
}

void slot_insert(ptr_t p, ptr_t c, timer_t now)
{
  bool isOnly  = first == MAX_TIMERS;
  bool isFirst = isOnly || p == MAX_TIMERS;
  bool isLast  = isOnly || p == slot[p].next;

  if (isLast) {
    slot[c].next = c;
  } else if (isFirst) {
    slot[c].next = first;
  } else {
    slot[c].next = slot[p].next;
  }

  if (isFirst) {
    first = c;
    timer_set(now, slot[c].cmp);
  } else {
    slot[p].next = c;
  }
}

void slot_remove(ptr_t p, ptr_t c)
{
  bool isFirst = c == first;
  bool isLast  = c == slot[c].next;
  bool isOnly  = isFirst && isLast;
  
  if (isOnly) {
    first = MAX_TIMERS;
  } else if (isFirst) {
    first = slot[c].next;
  } else if (isLast) {
    slot[p].next = p;
  } else {
    slot[p].next = slot[c].next;
  }

  if (isOnly) {
    timer_unset();
  } else if (isFirst) {
    timer_t now  = timer_now();
    timer_t orig = slot[c].cmp;
    timer_t new  = slot[first].cmp;
    timer_set(in_range(now, orig, new) ? now : orig,  new);
  }

  slot[c].next = MAX_TIMERS;
}

void timer_add_cmp(timer_t now, timer_t cmp, void (*func)(), void * arg, uint8_t level)
{
#if TIMER_DEBUG > 2
  first1 = first0;
  memcpy(slot1, slot0, sizeof(slot1));
  now1 = now0;
  cmp1 = cmp0;
#endif
#if TIMER_DEBUG > 1
  first0 = first;
  memcpy(slot0, slot, sizeof(slot0));
  now0 = now;
  cmp0 = cmp;
#endif
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    /* find empty slot */
    ptr_t c;
    for (c = 0; slot[c].next != MAX_TIMERS; c++) {
      assert(c < MAX_TIMERS);
      assert(slot[c].next <= MAX_TIMERS);
    }
 
    slot[c].cmp   = cmp;
    slot[c].func  = func;
    slot[c].arg   = arg;
    slot[c].level = level;

    /* find position */
    ptr_t p = MAX_TIMERS;
    ptr_t n = first;
      
    while (p != n && in_range(slot[n].cmp, cmp, now)) {
      p = n;
      n = slot[n].next;
    }

    /* link it */
    slot_insert(p, c, now);
  }
}

void timer_add(timer_t cnt, void (*func)(), void * arg, uint8_t level)
{
  timer_t now = timer_now();
  timer_t cmp = now + cnt;
  timer_add_cmp(now, cmp, func, arg, level);
}

void timer_cancel(void (*func)(), void * arg)
{
  ptr_t p = MAX_TIMERS;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    ptr_t c = first;
 
    while (p != c && c != MAX_TIMERS) {
      if (slot[c].func == func && slot[c].arg == arg) {
        slot_remove(p, c);
      } else {
        p = c;
      }
      c = slot[c].next;
    }
  }
}

/* sleep via timer */
#if 0 // makes no sense to do that via interrupt since nothing can be done instead
static enum {
  IDLE,
  WAITING,
  DONE,
} volatile sleep_state;

void timer_sleep_cb()
{
  assert(sleep_state == WAITING);
  sleep_state = DONE;
}

void timer_sleep_ticks(timer_t t)
{
  assert(sleep_state == IDLE);
  sleep_state = WAITING;
  timer_add(t, timer_sleep_cb, 0 , -1);
  while (sleep_state == WAITING);
  sleep_state = IDLE;
}
#else
void timer_sleep_ticks(timer_t t)
{
  timer_t start = timer_now();
  timer_t end   = start + t;
  while (in_range(start, timer_now(), end));
}
#endif
