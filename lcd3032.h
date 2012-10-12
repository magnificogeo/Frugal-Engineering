/*
 * lcd3032.h
 *
 *  Created on: May 16, 2012
 *      Author: by
 */

#ifndef LCD3032_H_
#define LCD3032_H_

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_timer.h"
#include "lpc17xx.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_adc.h"

#define LCD_RS_PIN 2
#define LCD_RS_PORT 2

#define LCD_RW_PIN  3
#define LCD_RW_PORT 2

#define LCD_E_PIN 4
#define LCD_E_PORT 2


void LCD_init_E();
void LCD_init_RW();
void LCD_init_RS();

void LCD_set_E(int value);
void LCD_set_RW(int value);
void LCD_set_RS(int value);
void lcd_data(char c);
void lcd_port(char c);


void lcd_string(const char *s);
void lcd_1stline(void);
void lcd_2ndline(void);
void lcd_clr(void);
void lcd_init(void);
void lcd_back(void);
void lcd_nocursor(void);
void lcd_cursor(void);

#endif /* LCD3032_H_ */

