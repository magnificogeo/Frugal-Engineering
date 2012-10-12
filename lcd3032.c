/*
 * lcd3032.c
 *
 *  Created on: May 16, 2012
 *      Author: by Tan Bing Yuan
 */
#include "lcd3032.h"
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

void LCD_init_E()
{
	 PINSEL_CFG_Type PinCfg;
    PinCfg.Funcnum = 0;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;
    PinCfg.Portnum = LCD_E_PORT;
    PinCfg.Pinnum = LCD_E_PIN;
    PINSEL_ConfigPin(&PinCfg);
    GPIO_SetDir(LCD_E_PORT, (1 << LCD_E_PIN), 1);	//port2.4

}
void LCD_init_RW()
{
	 PINSEL_CFG_Type PinCfg;
    PinCfg.Funcnum = 0;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;
    PinCfg.Portnum = LCD_RW_PORT;
    PinCfg.Pinnum = LCD_RW_PIN;
    PINSEL_ConfigPin(&PinCfg);
    GPIO_SetDir(LCD_RW_PORT, (1 << LCD_RW_PIN), 1);	//port 2.3

}
void LCD_init_RS()
{
	 PINSEL_CFG_Type PinCfg;
    PinCfg.Funcnum = 0;
    PinCfg.OpenDrain = 0;
    PinCfg.Pinmode = 0;
    PinCfg.Portnum = LCD_RS_PORT;
    PinCfg.Pinnum = LCD_RS_PIN;
    PINSEL_ConfigPin(&PinCfg);
    GPIO_SetDir(LCD_RS_PORT, (1 << LCD_RS_PIN), 1);	//port 2.2

}

void LCD_set_E(int value)
{
	switch(value)
	{
	case 0:
		GPIO_ClearValue( LCD_E_PORT, (1<<LCD_E_PIN) );
		break;

	case 1:
		GPIO_SetValue( LCD_E_PORT, (1<<LCD_E_PIN) );
		break;
	}
}

void LCD_set_RW(int value)
{
	switch(value)
		{
		case 0:
			GPIO_ClearValue( LCD_RW_PORT, (1<<LCD_RW_PIN) );
			break;

		case 1:
			GPIO_SetValue( LCD_RW_PORT, (1<<LCD_RW_PIN) );
			break;
		}
}

void LCD_set_RS(int value)
{
	switch(value)
		{
		case 0:
			GPIO_ClearValue( LCD_RS_PORT, (1<<LCD_RS_PIN) );
			break;

		case 1:
			GPIO_SetValue( LCD_RS_PORT, (1<<LCD_RS_PIN) );
			break;
		}
}

void lcd_port(char c)
{
	uint8_t temp = c;
	int i;

	for(i=28; i >= 21; i--)
	{
	  if(temp & 0x80)
	  {
		  GPIO_SetValue( 0, (1<<i) );
	  }
	  else
	  {
		  GPIO_ClearValue( 0, (1<<i) );
	  }
       temp = temp << 1;
	}
}

void lcd_data(char c) //Write the data into the LCD
{
//LCD_PORT = c;
    lcd_port(c);
	Timer0_us_Wait(3000);
	LCD_set_E(1);
	Timer0_us_Wait(3);
	LCD_set_E(0);
	Timer0_us_Wait(160);
}

void lcd_string(const char *s)//Display the string in the LCD
{

  LCD_set_RS(1);//Enable write mode
  while(*s)                   //The loop runs till the end of the string
  lcd_data(*s++);           //Write each character in the LCD
  LCD_set_RS(0);                //Disable write mode
}

void lcd_2ndline(void)        //Takes the cursor to the next line
{
  LCD_set_RS(0);
  lcd_data(0xC0);
}

void lcd_1stline(void) //Takes the cursor to the next line
{
    LCD_set_RS(0);
    lcd_data(0x80);
}

void lcd_clr(void)            //Clears the screen
{
  LCD_set_RS(0);
  lcd_data(0x01);
  Timer0_us_Wait(1800);
}

void lcd_init(void)           //Iniatialize the LCD
{


  LCD_set_RS(0);
  Timer0_Wait(15);
  lcd_data(0x01);             //Clears the screen
  lcd_data(0x03);             //Returns the cursor to the home position
  lcd_data(0x06);             //Enable entry mode
  lcd_data(0x0F);             //Display off, blink cursor on
  lcd_data(0x38);             //Function set for 8-bit interface, set 0x28 for 4-bit interface
}

void lcd_back(void)
{
	LCD_set_RS(0);
	lcd_data(0x10);
	Timer0_us_Wait(1800);
}

void lcd_nocursor()
{
	    LCD_set_RS(0);
		lcd_data(0x0C);
		Timer0_us_Wait(1800);
}

void lcd_cursor()
{
	    LCD_set_RS(0);
		lcd_data(0x0E);
		Timer0_us_Wait(1800);
}
