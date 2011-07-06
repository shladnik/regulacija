#define SIZE 255
#if SIZE <= 256
typedef uint8_t print_buf_p;
#else
typedef uint16_t print_buf_p;
#endif

DBG char print_buf [SIZE];
DBG print_buf_p print_buf_wp;
DBG print_buf_p print_buf_rp;
DBG uint16_t print_buf_ovf;
//volatile bool    print_buf_block  = 0;
#define print_buf_block 0


print_buf_p print_buf_level()
{
  return (print_buf_wp >= print_buf_rp ? 0 : SIZE) + print_buf_wp - print_buf_rp;
}

print_buf_p print_buf_space()
{
  return SIZE - 1 - print_buf_level();
}

bool print_buf_empty()
{
  return print_buf_rp == print_buf_wp;
}

bool print_buf_full()
{
  return print_buf_level() == SIZE - 1;
}

print_buf_p print_buf_padd(print_buf_p p, print_buf_p a)
{
  print_buf_p o = SIZE - p;
  if (o <= a) p = a - o;
  else        p += a;
  return p;
}

print_buf_p print_buf_pinc(print_buf_p p)
{
#if 1
  p += 1;
  if (p >= SIZE) p -= SIZE;
  return p;
#else
  return print_buf_padd(p, 1);
#endif
}

void print_buf_write(char b)
{
  print_buf[print_buf_wp] = b;
  print_buf_wp = print_buf_pinc(print_buf_wp);
}

char print_buf_read()
{
  char r = print_buf[print_buf_rp];
  print_buf_rp = print_buf_pinc(print_buf_rp);
  return r;
}

char print_buf_peek(print_buf_p i)
{
  if (i < SIZE - print_buf_rp)
    i += print_buf_rp;
  else
    i -= SIZE - print_buf_rp;
  return print_buf[i];
}

int print_buf_putc(char c, FILE * f)
{
  (void)(f);
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    if (print_buf_block) {
      while (print_buf_full()) {
#if PLAIN_CONSOLE
        if (UCSRA & (1 << UDRE)) UDR = print_buf_read();
#else
        if (UCSRA & (1 << UDRE)) {
          NONATOMIC_BLOCK(NONATOMIC_FORCEOFF) {
            __asm__("nop");
            __asm__("nop");
          }
        }
#endif
      }
      print_buf_write(c);
    } else {
      if (print_buf_full() || print_buf_ovf) {
        if (print_buf_ovf < UINT16_MAX)
          print_buf_ovf++;
      } else {
        print_buf_write(c);
      }
    }
#if PLAIN_CONSOLE
    UCSRB |= (1 << UDRIE);
#endif
  }
  return 0;
}

static FILE print_str = FDEV_SETUP_STREAM(
  print_buf_putc, 
  0,
  _FDEV_SETUP_WRITE);

void print_buf_init()
{
  stdout = &print_str;
  stderr = &print_str;
#if 0
  /* Check if console is messed up (and almost certainly is on power-up) */
  if ((print_buf_ovf && (print_buf_pinc(print_buf_wp) != print_buf_rp)) ||
       print_buf_wp >= SIZE ||
       print_buf_rp >= SIZE) {
    print_buf_ovf = 0;
    if (print_buf_rp >= SIZE) print_buf_rp = 0;
    print_buf_wp = print_buf_rp;
  }
#endif
}
