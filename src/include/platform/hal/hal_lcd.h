#ifndef __HAL_LCD_H
#define __HAL_LCD_H

#define LCD_X_DOT           (128)
#define LCD_Y_DOT           (64)

void hal_lcd_init(void);
void hal_lcd_display(uint8_t state);
void hal_lcd_clear(void);
void hal_lcd_set_pixel(uint8_t x, uint8_t y, uint8_t pixel);
uint8_t hal_lcd_get_pixel(uint8_t x, uint8_t y);
void hal_lcd_backlight(bool_t state);
#endif