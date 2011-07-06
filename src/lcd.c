#define LCDP 0

static const double d0 = 2*1;
static const double d1 = 2*1520;
static const double d2 = 2*37;

static char fb [LCD_S+1];
static uint8_t lcd_pos;

void lcd_write_4b(bool ctrl, uint8_t dat)
{
  if (!ctrl) port_set_1(LCDP, 0x02);
  else       port_set_0(LCDP, 0x02);
  port_set_0(LCDP, ~dat & 0xf0);
  port_set_1(LCDP,  dat & 0xf0);
  _delay_us(d0);
  port_set_1(LCDP, 0x04);
  _delay_us(d0);
  port_set_0(LCDP, 0x04);
  //_delay_us(d0);
}

void lcd_write(bool ctrl, uint8_t dat)
{
  lcd_write_4b(ctrl, dat << 0);
  lcd_write_4b(ctrl, dat << 4);
  if (ctrl && !(dat & 0xfc)) _delay_us(d1);
  else                       _delay_us(d2);
}

void lcd_init()
{
  static bool power_on_reset = 1;
  if (power_on_reset) {
    power_on_reset = 0;
    timer_sleep_ms(40);
  }

  uint8_t d;

  d  = 0x30;
  lcd_write_4b(1, d);
  _delay_us(d2);

  d  = 0x20;     // Function set
  d |= (1 << 3); // 1/2 line mode
  d |= (0 << 2); // 5x8/5x11 dots
  lcd_write(1, d);
  lcd_write(1, d);

  d  = 0x08;     // Display on/off control
  d |= (1 << 2); // display enable
  d |= (0 << 1); // cursor enable
  d |= (0 << 0); // blink enable
  lcd_write(1, d);

  d  = 0x01;     // Display clear
  lcd_write(1, d);

  d  = 0x04;     // Entry mode set
  d |= (1 << 1); // dec/inc mode
  d |= (0 << 0); // entire shift
  lcd_write(1, d);

  lcd_write(1, 0x01);
  lcd_pos = 0;
  fb[LCD_S] = 0;
}

void lcd_set_adr(uint8_t y, uint8_t x)
{
  const uint8_t y_off [] = { 0x00, 0x40, 0x14, 0x54 };
  uint8_t pos = y_off[y] + x;
  lcd_write(1, 0x80 | pos);
  
  lcd_pos = y * LCD_W + x;
}

void lcd_pos_inc()
{
  lcd_pos++;
  if (lcd_pos >= LCD_S) lcd_pos = 0;

  for (uint8_t i = 0; i < LCD_H; i++) {
    uint8_t edge = i * LCD_W;
    if (lcd_pos == edge) lcd_set_adr(i, 0);
    if (lcd_pos <= edge) break;
  }
}

int lcd_putc(char c, FILE * f)
{
  (void)(f);

  fb[lcd_pos] = c;
  lcd_write(0, (uint8_t)c);

  lcd_pos_inc();

  return 0;
}

FILE lcd_str = FDEV_SETUP_STREAM(
  lcd_putc, 
  0,
  _FDEV_SETUP_WRITE);

int lprintf(uint8_t y, uint8_t x, const char * __fmt, ...)
{
  lcd_set_adr(y, x);
#if 0
  (void)__fmt;
  return 0;
#else
  va_list args;
  va_start(args, __fmt);
  return vfprintf(&lcd_str, __fmt, args);
#endif
}

void lcd_refresh()
{
  lprintf(0, 0, &fb[0]);
}

void lcd_loop()
{
  lcd_init();
  lcd_refresh();
#if 0
  lprintf(0, 0,
    "His|Hle|Pec|Kol|Rad?"
    "   |   |   |   |    "
    "   |   |   |   |    "
    "   |   |   |   |    ");

  lprintf(1,  0, "%3d", TEMP2I(ds18b20_get_temp(DS18B20_HOUSE_0,    RESOLUTION_9)));
  lprintf(2,  0, "%3d", TEMP2I(ds18b20_get_temp(DS18B20_HOUSE_S_T,  RESOLUTION_9)));
  lprintf(3,  0, "%3d", TEMP2I(ds18b20_get_temp(DS18B20_HOUSE_S_B,  RESOLUTION_9)));

  lprintf(0,  3, "%c", pumping_state == PUMPING_S2H ? '<' : (pumping_state == PUMPING_H2S ? '>' : '|'));
  lprintf(1,  4, "%3d", TEMP2I(ds18b20_get_temp(DS18B20_STABLE_S_T, RESOLUTION_9)));
  lprintf(2,  4, "%3d", TEMP2I(ds18b20_get_temp(DS18B20_STABLE_S_B, RESOLUTION_9)));

  lprintf(1,  8, "%3d", TEMP2I(ds18b20_get_temp(DS18B20_FURNACE_T,  RESOLUTION_9)));
  lprintf(2,  8, "%3d", TEMP2I(ds18b20_get_temp(DS18B20_FURNACE_B,  RESOLUTION_9)));
  lprintf(3,  8, "%c",  relay_get(RELAY_PUMP_FURNACE) ? '*' : ' ');
  lprintf(3, 10, "%c",  valve_opened(VALVE_FURNACE) ? '|' : (valve_closed(VALVE_FURNACE) ? '-' : '/'));

  lprintf(1, 12, "%3d", TEMP2I(ds18b20_get_temp(DS18B20_COLLECTOR,  RESOLUTION_9)));
  lprintf(3, 12, "%c",  relay_get(RELAY_PUMP_COLLECTOR) ? '*' : ' ');

  lprintf(1, 16, "%3d", TEMP2I(ds18b20_get_temp(DS18B20_RADIATOR_U, RESOLUTION_9)));
  lprintf(2, 16, "%3d", TEMP2I(ds18b20_get_temp(DS18B20_RADIATOR_D, RESOLUTION_9)));
  lprintf(3, 16, "%c",  relay_get(RELAY_PUMP_RADIATOR)  ? '*' : ' ');
  lprintf(3, 18, "%c",  valve_opened(VALVE_RADIATOR)    ? '|' : (valve_closed(VALVE_RADIATOR) ? '-' : '/'));
#else
  lprintf(0, 0,
    "His%cHle|Pec|Kol|Rad "
    "%3d|%3d|%3d|%3d|%3d "
    "%3d|%3d|%3d|   |%3d "
    "%3d|   |%c %c|%c  |%c %c ",
      pumping_state == PUMPING_S2H ? '<' : (pumping_state == PUMPING_H2S ? '>' : '|'),
      
      TEMP2I(ds18b20_get_temp(DS18B20_HOUSE_0,    RESOLUTION_9, 1)),
      TEMP2I(ds18b20_get_temp(DS18B20_STABLE_S_T, RESOLUTION_9, 1)),
      TEMP2I(ds18b20_get_temp(DS18B20_FURNACE_T,  RESOLUTION_9, 1)),
      TEMP2I(ds18b20_get_temp(DS18B20_COLLECTOR,  RESOLUTION_9, 1)),
      TEMP2I(ds18b20_get_temp(DS18B20_RADIATOR_U, RESOLUTION_9, 1)),
  
      TEMP2I(ds18b20_get_temp(DS18B20_HOUSE_S_T,  RESOLUTION_9, 1)),
      TEMP2I(ds18b20_get_temp(DS18B20_STABLE_S_B, RESOLUTION_9, 1)),
      TEMP2I(ds18b20_get_temp(DS18B20_FURNACE_B,  RESOLUTION_9, 1)),
      TEMP2I(ds18b20_get_temp(DS18B20_RADIATOR_D, RESOLUTION_9, 1)),

      TEMP2I(ds18b20_get_temp(DS18B20_HOUSE_S_B,  RESOLUTION_9, 1)),
      relay_get(RELAY_PUMP_FURNACE)   ? '*' : ' ',
      valve_opened(VALVE_FURNACE)     ? '|' : (valve_closed(VALVE_FURNACE)  ? '-' : '/'),
      relay_get(RELAY_PUMP_COLLECTOR) ? '*' : ' ',
      relay_get(RELAY_PUMP_RADIATOR)  ? '*' : ' ',
      valve_opened(VALVE_RADIATOR)    ? '|' : (valve_closed(VALVE_RADIATOR) ? '-' : '/'));
#endif
}

void lcd_heartbeat()
{
  static char c = ' ';
  
  lprintf(0, LCD_W-1, "%c", c);
  
  switch (c) {
    case ' ': c = '.'; break;
    case '.': c = 'o'; break;
    case 'o': c = 'O'; break;
    case 'O': c = '#'; break;
    default : c = ' '; break;
  }
}

