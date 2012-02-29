#define ISC_LOW     ((0 << ISC01) | (0 << ISC00))
#define ISC_EDGE    ((0 << ISC01) | (1 << ISC00))
#define ISC_NEGEDGE ((1 << ISC01) | (0 << ISC00))
#define ISC_POSEDGE ((1 << ISC01) | (1 << ISC00))

void keys_init()
{
  MCUCR |= (ISC_NEGEDGE << ISC10) | (ISC_NEGEDGE << ISC00);
  GICR  |= (1 << INT1) | (1 << INT0);
}

void int0_cb()
{
  if (!port_get_pin(3, 2)) {
    DBG static uint8_t int0_cnt;
    int0_cnt++;
  }
  GIFR  = 1 << INTF0;
  GICR |= 1 << INT0;
}

DBG_ISR(INT0_vect)
{
  if (!port_get_pin(3, 2)) {
    GICR &= ~(1 << INT0);
    timer_add(TIMER_MS(25), int0_cb, 0, -1);
  }
}

void int1_cb()
{
  if (!port_get_pin(3, 3)) {
    DBG static uint8_t int1_cnt;
    int1_cnt++;
  }
  GIFR  = 1 << INTF1;
  GICR |= 1 << INT1;
}

DBG_ISR(INT1_vect)
{
  if (!port_get_pin(3, 3)) {
    GICR &= ~(1 << INT1);
    timer_add(TIMER_MS(50), int1_cb, 0, -1);
  }
}
