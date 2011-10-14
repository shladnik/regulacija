uintptr_t find_free(uintptr_t start, uintptr_t end)
{
#if 1 /* this is not needed as long as we don't use 'heap' */
  /* this will recurse very deep if ram is not initialized */
  while (start < end) {
    if (*(uint8_t *)start == 0x55) {
      break;
    } else {
      start++;
    }
  }

  while (start < end) {
    if (*(uint8_t *)end   == 0x55) {
      break;
    } else {
      end--;
    }
  }

  uintptr_t i;
  for (i = start; i < end; i++) {
    if (*(uint8_t *)i != 0x55) {
      break;
    }
  }

  uintptr_t free = i - start;
  uintptr_t free1 = i == end ? 0 : find_free(i, end);
  return MAX(free, free1);
#else
  uintptr_t i = start;
  while (i < end) {
    if (*(uint8_t *)i != 0x55) {
      break;
    } else {
      i++;
    }
  }

  return i - start;
#endif
}

__attribute__((used))
uintptr_t stack_check()
{
#if 0
  extern uint8_t __heap_start;
  uintptr_t start = (uintptr_t)&__heap_start;
  uintptr_t end   = SP;
  return find_free(start, end);
#else
  extern uint8_t __heap_start;
  uintptr_t max_len = 0;
  uintptr_t len     = 0;
  uint8_t * i;
  for (i = &__heap_start; i < (uint8_t *)SP; i++) {
    if (*i == 0x55) len++;
    else            len = 0;
    max_len = MAX(max_len, len);
  }

  return max_len;
#endif
}

__attribute__((used))
void stack_check_init()
{
  extern uint8_t _end;
  uint8_t * end = (uint8_t *)SP;
  for (uint8_t * i = &_end; i < end; i++) *i = 0x55;
}
