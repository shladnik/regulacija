#define MAX_TIMERS 16 /* also used as invalid value */

typedef uint8_t ptr_t;

typedef struct {
  ptr_t next;
  timer_t cmp;
  sch_t func;
} slot_t;

DBG2CP static slot_t slot [MAX_TIMERS];
DBG2CP static ptr_t first;
DBG2CP static timer_t timer_tracked;

void timer_init()
{
  first = MAX_TIMERS;
  for (ptr_t i = 0; i < MAX_TIMERS; i++) slot[i].next = MAX_TIMERS;
  timer_start();
}

timer_t timer_tracked_get()
{
  timer_t now = timer_now();
  if (first == MAX_TIMERS || in_range(timer_tracked, now, slot[first].cmp + 1))
    timer_tracked = now;
  else {
    DBG static timer_t timer_late_max_0;
    timer_late_max_0 = MAX(timer_late_max_0, now - timer_tracked);
  }

  return timer_tracked;
}

void timer_int()
{
  slot_t c = slot[first];
  slot[first].next = MAX_TIMERS;

#ifndef NDEBUG
  timer_t now = timer_now();
  if (in_range(timer_tracked_get(), c.cmp, now)) {
    DBG static timer_t timer_late_max;
    timer_late_max = MAX(timer_late_max, now - c.cmp);
  }
#endif

  if (c.next == first) {
    first = MAX_TIMERS;
    timer_unset();
  } else {
    timer_tracked = c.cmp;
    first = c.next;
    timer_set(slot[first].cmp);
  }
  
  sch_add(c.func);
}

void slot_insert(ptr_t p, ptr_t c)
{
  bool isOnly  = first == MAX_TIMERS;
  bool isFirst = isOnly || p == MAX_TIMERS;
  bool isLast  = isOnly || (!isFirst && p == slot[p].next);

  if (isLast) {
    slot[c].next = c;
  } else if (isFirst) {
    slot[c].next = first;
  } else {
    slot[c].next = slot[p].next;
  }

  if (isFirst) {
    first = c;
    timer_set(slot[c].cmp);
  } else {
    slot[p].next = c;
  }
}

ptr_t slot_remove(ptr_t p, ptr_t c)
{
  bool isFirst = p == MAX_TIMERS;
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
    timer_set(slot[first].cmp);
  }

  slot[c].next = MAX_TIMERS;
  return isFirst ? first : slot[p].next;
}

void timer_add_cmp(timer_t cmp, sch_t func)
{
  DBG_ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    /* find empty slot */
    ptr_t c = 0;
    while (slot[c].next != MAX_TIMERS) {
      c++;
      assert(c < MAX_TIMERS);
    }

    slot[c].cmp  = cmp;
    slot[c].func = func;

    /* find position */
    ptr_t p = MAX_TIMERS;
    ptr_t n = first;
    
    timer_t now = timer_tracked_get();
    while (p != n && in_range(now, slot[n].cmp, cmp + 1)) {
      p = n;
      n = slot[n].next;
    }

    /* link it */
    slot_insert(p, c);
  }
}

void timer_add(timer_t cnt, sch_t func)
{
  DBG_ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    timer_t cmp = timer_tracked_get() + cnt;
    timer_add_cmp(cmp, func);
  }
}

ptr_t timer_cancel(sch_t func, ptr_t nr)
{
  ptr_t cnt = 0;
  ptr_t p = MAX_TIMERS;
  DBG_ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    ptr_t c = first;
    while (p != c) {
      if (memcmp(&func, &slot[c].func, sizeof(sch_t)) == 0) {
        c = slot_remove(p, c);
        if (++cnt >= nr) break;
      } else {
        p = c;
        c = slot[c].next;
      }
    }
  }
  return cnt;
}

void timer_sleep_ticks(timer_t t)
{
  timer_t start = timer_now();
  timer_t end   = start + t;
  while (in_range(start, timer_now(), end));
}
