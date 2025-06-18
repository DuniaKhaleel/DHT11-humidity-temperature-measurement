#ifndef LCD_H
#define LCD_H

#include <xc.h>
#include <stdint.h>

// LCD module connections
#define LCD_RS RD0
#define LCD_RW RD1
#define LCD_EN RD2
#define LCD_D4 RD3
#define LCD_D5 RD4
#define LCD_D6 RD5
#define LCD_D7 RD6

// LCD Functions
void LCD_Command(uint8_t cmd);
void LCD_Char(uint8_t data);
void LCD_String(const char *str);
void LCD_Init(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Clear(void);

#endif