#define ISC_LOW     (((0 << ISC01) | (0 << ISC00)) >> ISC00)
#define ISC_EDGE    (((0 << ISC01) | (1 << ISC00)) >> ISC00)
#define ISC_NEGEDGE (((1 << ISC01) | (0 << ISC00)) >> ISC00)
#define ISC_POSEDGE (((1 << ISC01) | (1 << ISC00)) >> ISC00)
#define ISC_MASK    ISC_POSEDGE
#if 0
static const timer_t t_settle = TIMER_MS(20);

typedef struct {
  func_t press;
  func_t release;
} key_t;

key_t keys [1];

void keys_init()
{
  MCUCR |= (ISC_NEGEDGE << ISC10) | (ISC_NEGEDGE << ISC00);
  GIFR   = (1 << INTF1) | (1 << INTF0);
  GICR  |= (1 << INT1 ) | (1 << INT0 );
}

static bool int_edge_get(uint8_t int_nr)
{
  const uint8_t shift = int_nr ? ISC10 : ISC00;
  return ((MCUCR >> shift) & ISC_MASK) == ISC_POSEDGE;
}

static void int_edge_set(uint8_t int_nr, bool edge)
{
  const uint8_t shift = int_nr ? ISC10 : ISC00;
  MCUCR &= ~(ISC_MASK << shift);
  MCUCR |= (edge ? ISC_POSEDGE : ISC_NEGEDGE) << shift;
}

void int0_cb()
{
  GIFR  = 1 << INTF0;

  bool edge = int_edge_get(0);
  if (port_get_pin(3, 2) == edge) {
    if (edge) DBG_CNT(int0_posedge);
    else      DBG_CNT(int0_negedge);
    int_edge_set(0, !edge);
  } else {
    if (edge) DBG_CNT(int0_posedge_false);
    else      DBG_CNT(int0_negedge_false);
  }

  GICR |= 1 << INT0;
}

DBG_ISR(INT0_vect,)
{
  if (port_get_pin(3, 2) == int_edge_get(0)) {
    GICR &= ~(1 << INT0);
    if (t_settle) timer_add(t_settle, int0_cb, 0, -1);
    else          int0_cb();
  }
}

DBG_ISR(INT1_vect,)
{
  DBG_CNT(int1_posedge);
}
#else
void keys_init()
{
}
#endif
