#include "lcd.h"
#include <xc.h>
#include <stdint.h>
#define _XTAL_FREQ 8000000  // Define CPU frequency for delay functions

// Function to send a command to the LCD
void LCD_Command(uint8_t cmd) {
    LCD_RS = 0;  // RS=0 for command mode
    LCD_RW = 0;  // RW=0 for write mode
    
    // Send upper nibble (bits 7-4)
    LCD_D7 = (cmd & 0x80) ? 1 : 0;  // Set D7 to bit 7 of command
    LCD_D6 = (cmd & 0x40) ? 1 : 0;  // Set D6 to bit 6 of command
    LCD_D5 = (cmd & 0x20) ? 1 : 0;  // Set D5 to bit 5 of command
    LCD_D4 = (cmd & 0x10) ? 1 : 0;  // Set D4 to bit 4 of command
    
    LCD_EN = 1;    // Enable pulse high
    __delay_us(1); // Wait 1us (minimum enable pulse width)
    LCD_EN = 0;    // Enable pulse low
    __delay_us(100); // Wait 100us (command execution time)
    
    // Send lower nibble (bits 3-0)
    LCD_D7 = (cmd & 0x08) ? 1 : 0;  // Set D7 to bit 3 of command
    LCD_D6 = (cmd & 0x04) ? 1 : 0;  // Set D6 to bit 2 of command
    LCD_D5 = (cmd & 0x02) ? 1 : 0;  // Set D5 to bit 1 of command
    LCD_D4 = (cmd & 0x01) ? 1 : 0;  // Set D4 to bit 0 of command
    
    LCD_EN = 1;    // Enable pulse high
    __delay_us(1); // Wait 1us
    LCD_EN = 0;    // Enable pulse low
    __delay_us(100); // Wait 100us
}

// Function to send a character to the LCD
void LCD_Char(uint8_t data) {
    LCD_RS = 1;  // RS=1 for data mode
    LCD_RW = 0;  // RW=0 for write mode
    
    // Send upper nibble (bits 7-4)
    LCD_D7 = (data & 0x80) ? 1 : 0;  // Set D7 to bit 7 of data
    LCD_D6 = (data & 0x40) ? 1 : 0;  // Set D6 to bit 6 of data
    LCD_D5 = (data & 0x20) ? 1 : 0;  // Set D5 to bit 5 of data
    LCD_D4 = (data & 0x10) ? 1 : 0;  // Set D4 to bit 4 of data
    
    LCD_EN = 1;    // Enable pulse high
    __delay_us(1); // Wait 1us
    LCD_EN = 0;    // Enable pulse low
    __delay_us(100); // Wait 100us
    
    // Send lower nibble (bits 3-0)
    LCD_D7 = (data & 0x08) ? 1 : 0;  // Set D7 to bit 3 of data
    LCD_D6 = (data & 0x04) ? 1 : 0;  // Set D6 to bit 2 of data
    LCD_D5 = (data & 0x02) ? 1 : 0;  // Set D5 to bit 1 of data
    LCD_D4 = (data & 0x01) ? 1 : 0;  // Set D4 to bit 0 of data
    
    LCD_EN = 1;    // Enable pulse high
    __delay_us(1); // Wait 1us
    LCD_EN = 0;    // Enable pulse low
    __delay_us(100); // Wait 100us
}

// Function to send a string to the LCD
void LCD_String(const char *str) {
    while (*str) {          // Loop until null terminator
        LCD_Char(*str++);   // Send each character and increment pointer
    }
}

// Function to initialize the LCD
void LCD_Init(void) {
    __delay_ms(20);         // Wait for LCD power stabilization
    
    // Initialization sequence for 4-bit mode
    LCD_Command(0x02);    // Initialize LCD in 4-bit mode
    LCD_Command(0x28);    // 2 lines, 5x7 matrix (4-bit mode)
    LCD_Command(0x0C);    // Display on, cursor off
    LCD_Command(0x06);    // Increment cursor (auto-increment)
    LCD_Command(0x01);    // Clear display
    __delay_ms(2);        // Wait for clear command to complete
}

// Function to set cursor position
void LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t address;
    if (row == 0)
        address = 0x80 + col;  // First row starts at 0x80
    else
        address = 0xC0 + col;  // Second row starts at 0xC0
    LCD_Command(address);     // Send cursor position command
}

// Function to clear the LCD display
void LCD_Clear(void) {
    LCD_Command(0x01);    // Clear display command
    __delay_ms(2);        // Wait for command to complete
}