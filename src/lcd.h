#ifndef __LCD_H__
#define __LCD_H__

#define LCD_W (20)
#define LCD_H (4)
#define LCD_S (LCD_W * LCD_H)

typedef char lcd_fb_t [LCD_H] [LCD_W];

extern FILE lcd_str;
int lprintf(uint8_t y, uint8_t x, const char * __fmt, ...);

void lcd_init();
void lcd_refresh();

lcd_fb_t * lcd_get_fb();
void lcd_set_pos(uint8_t y, uint8_t x, char c);

void lcd_loop();
void lcd_heartbeat();

#endif
