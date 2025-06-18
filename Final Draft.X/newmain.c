/*
 * PIC16F877A with DHT11 sensor using XC8 v1.41
 * LCD displays temperature and humidity
 * LEDs indicate temperature range:
 *   Red (RC5)   - Temp > 25°C
 *   Green (RC6) - 20°C ? Temp ? 25°C
 *   Blue (RC7)  - Temp < 20°C
 *   Buzzer (RC4) - Temp > 30°C
 */

// CONFIG - Configuration bits for the PIC microcontroller
#pragma config FOSC = HS        // Use HS (High Speed) oscillator
#pragma config WDTE = OFF       // Disable Watchdog Timer
#pragma config PWRTE = OFF      // Disable Power-up Timer
#pragma config BOREN = OFF      // Disable Brown-out Reset
#pragma config LVP = OFF        // Disable Low-Voltage Programming
#pragma config CPD = OFF        // Disable Data EEPROM Code Protection
#pragma config WRT = OFF        // Disable Flash Program Memory Write Protection
#pragma config CP = OFF         // Disable Flash Program Memory Code Protection

#include <xc.h>                 // Include PIC compiler header file
#include <stdio.h>              // Standard I/O library
#include <stdlib.h>             // Standard library
#include <stdint.h>             // Standard integer types
#include "lcd.h"                // LCD library header
#define _XTAL_FREQ 8000000      // Define oscillator frequency for delay functions

// LED connections - Define macros for easy access to LED pins
#define RED_LED RC5             // Red LED connected to RC5
#define GREEN_LED RC6           // Green LED connected to RC6
#define BLUE_LED RC7            // Blue LED connected to RC7
#define BUZZER   RC4            // Buzzer connected to RC4

#define DHT11_PIN RB4           // DHT11 data pin connected to RB4

// Message buffers for LCD display
char message1[] = "Temp = 00.0 C  ";  // Temperature display template
char message2[] = "RH   = 00.0 %  ";  // Humidity display template
uint8_t Time_out;              // Flag for sensor timeout
uint8_t T_byte1, T_byte2, RH_byte1, RH_byte2, CheckSum;  // Variables for sensor data

// Function to send start signal to DHT11 sensor
void start_signal() {
    TRISB4 = 0;              // Set RB4 as output
    RB4 = 0;                 // Pull RB4 low
    __delay_ms(25);          // Wait 25ms (DHT11 spec requires at least 18ms)
    RB4 = 1;                 // Pull RB4 high
    __delay_us(30);          // Wait 30us (DHT11 spec requires 20-40us)
    TRISB4 = 1;              // Set RB4 as input
}

// Function to check DHT11 response after start signal
uint8_t check_response() {
    __delay_us(40);          // Wait 40us
    if(!RB4) {               // Check if DHT11 pulled line low
        __delay_us(80);      // Wait 80us
        if(RB4) {            // Check if DHT11 pulled line high
            __delay_us(50);  // Wait 50us
            return 1;        // Return success
        }
    }
    return 0;                // Return failure
}

// Function to read one byte of data from DHT11
uint8_t Read_Data() {
    uint8_t i, k, _data = 0; // i=bit counter, k=timeout counter, _data=received byte
    
    if(Time_out)             // If timeout occurred previously
        return 0;           // Return 0
        
    for(i = 0; i < 8; i++) {  // Read 8 bits
        k = 0;
        while(!RB4) {        // Wait while pin is low (start of bit)
            k++;
            if(k > 100) {   // Timeout check
                Time_out = 1;
                break;
            }
            __delay_us(1);  // Wait 1us
        }
        __delay_us(30);     // Wait 30us to check bit value
        if(!RB4)            // If pin is low after 30us, it's a '0' bit
            _data &= ~(1 << (7 - i));  // Clear the bit
        else {              // Otherwise it's a '1' bit
            _data |= (1 << (7 - i));   // Set the bit
            while(RB4) {    // Wait while pin is high
                k++;
                if(k > 100) {  // Timeout check
                    Time_out = 1;
                    break;
                }
                __delay_us(1);  // Wait 1us
            }
        }
    }
    return _data;           // Return the received byte
}

// Function to control LEDs based on temperature
void update_leds(float temperature) {
    // Turn off all LEDs first
    RED_LED = 0;
    GREEN_LED = 0;
    BLUE_LED = 0;
    BUZZER = 0;
    
    // Determine which LED to turn on based on temperature
    if (temperature > 25.0) {
        RED_LED = 1;    // Turn on red LED for high temperature
        LCD_Clear();
        LCD_SetCursor(0,4);
        LCD_String("Warning");
    }
    else if (temperature >= 20.0 && temperature <= 25.0) {
        GREEN_LED = 1;  // Turn on green LED for normal temperature
    }
    else if(temperature < 20.0) {
        BLUE_LED = 1;   // Turn on blue LED for low temperature
    }
    
    // Activate buzzer if temperature is too high
    if(temperature > 25.0) {
        BUZZER = 1;
    }
    else {
        BUZZER = 0;
    }
}

// Main program
void main(void) {
    // Configure ports
    TRISD = 0x00;   // PORTD as output for LCD
    TRISC5 = 0;     // RC5 as output for red LED
    TRISC6 = 0;     // RC6 as output for green LED
    TRISC7 = 0;     // RC7 as output for blue LED
    TRISB4 = 1;     // RB4 as input for DHT11
    TRISC4 = 0;     // RC4 as output for buzzer
    
    // Initialize all outputs to off
    RED_LED = 0;
    GREEN_LED = 0;
    BLUE_LED = 0;
    BUZZER = 0;
    
    // Initialize LCD
    LCD_Init();     // Initialize LCD module
    LCD_Clear();    // Clear LCD display
    __delay_ms(1000);  // Wait 1 second for stabilization
    
    // Main program loop
    while(1) {
        Time_out = 0;           // Reset timeout flag
        start_signal();         // Send start signal to DHT11
        
        if(check_response()) {  // If DHT11 responds
            // Read all 5 bytes from DHT11
            RH_byte1 = Read_Data();  // Humidity integer part
            RH_byte2 = Read_Data();  // Humidity decimal part
            T_byte1 = Read_Data();   // Temperature integer part
            T_byte2 = Read_Data();   // Temperature decimal part
            CheckSum = Read_Data();  // Checksum
            
            if(Time_out) {      // If reading timed out
                LCD_Clear();
                LCD_SetCursor(0, 4);  // Position cursor
                LCD_String("Time out!");  // Display error
            }
            else {
                // Verify checksum
                if(CheckSum == ((RH_byte1 + RH_byte2 + T_byte1 + T_byte2) & 0xFF)) {
                    // Calculate temperature value
                    float temperature = T_byte1 + (T_byte2 / 10.0);
                    
                    // Update display strings with new values
                    // Temperature integer part (tens digit)
                    message1[7] = T_byte1/10 + 48;  // Convert to ASCII
                    // Temperature integer part (units digit)
                    message1[8] = T_byte1%10 + 48;   // Convert to ASCII
                    // Temperature decimal part
                    message1[10] = T_byte2/10 + 48;  // Convert to ASCII
                    // Humidity integer part (tens digit)
                    message2[7] = RH_byte1/10 + 48;  // Convert to ASCII
                    // Humidity integer part (units digit)
                    message2[8] = RH_byte1%10 + 48;  // Convert to ASCII
                    // Humidity decimal part
                    message2[10] = RH_byte2/10 + 48; // Convert to ASCII
                    message1[11] = 223;              // Degree symbol
                    
                    // Display messages on LCD
                    LCD_SetCursor(0, 0);  // First row, first column
                    LCD_String(message1); // Display temperature
                    LCD_SetCursor(1, 0);  // Second row, first column
                    LCD_String(message2); // Display humidity
                    
                    // Update LEDs based on temperature
                    update_leds(temperature);
                }
                else {
                    // Checksum error handling
                    LCD_Clear();
                    LCD_SetCursor(0, 0);
                    LCD_String("Checksum Error!");
                }
            }
        }
        else {
            // No response from sensor
            LCD_Clear();
            LCD_SetCursor(0, 2);
            LCD_String("No response");
            LCD_SetCursor(1, 0);
            LCD_String("from the sensor");
        }
        __delay_ms(1000);  // Wait 1 second before next reading
    }
}