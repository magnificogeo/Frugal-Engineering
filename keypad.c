/*
 * keypad.c
 *
 *  Created on: 27-Jun-2012
 *      Author: AdminNUS
 */

#include "keypad.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"

void init_keypad()
{
	// Initialize K
	    PINSEL_CFG_Type PinCfg;
	    PinCfg.Funcnum = 0;
	    PinCfg.OpenDrain = 0;
	    PinCfg.Pinmode = 0;
	    PinCfg.Portnum = 2;
	    PinCfg.Pinnum = 12;
	    PINSEL_ConfigPin(&PinCfg);
	    GPIO_SetDir(2, 1<<12, 1);  //output


	    // Initialize J
	        PINSEL_CFG_Type PinCfg1;
	        PinCfg1.Funcnum = 0;
	        PinCfg1.OpenDrain = 0;
	        //PinCfg1.Pinmode = 0;
	        PinCfg1.Portnum = 2;
	        PinCfg1.Pinnum = 11;
	        PINSEL_ConfigPin(&PinCfg1);
	        GPIO_SetDir(2, 1<<11, 1);  //output

	        // Initialize H
	            PINSEL_CFG_Type PinCfg2;
	            PinCfg2.Funcnum = 0;
	            PinCfg2.OpenDrain = 0;
	            //PinCfg2.Pinmode = 0;
	            PinCfg2.Portnum = 2;
	            PinCfg2.Pinnum = 10;
	            PINSEL_ConfigPin(&PinCfg2);
	            GPIO_SetDir(2, 1<<10, 1);  //output

	            // Initialize G
	                        PINSEL_CFG_Type PinCfg3;
	                        PinCfg3.Funcnum = 0;
	                        PinCfg3.OpenDrain = 0;
	                        //PinCfg3.Pinmode = 0;
	                        PinCfg3.Portnum = 2;
	                        PinCfg3.Pinnum = 8;
	                        PINSEL_ConfigPin(&PinCfg3);
	                        GPIO_SetDir(2, 1<<8, 1);  //output

	        // Initialize F
	               PINSEL_CFG_Type PinCfg4;
	               PinCfg4.Funcnum = 0;
	               PinCfg4.OpenDrain = 0;
	               PinCfg4.Pinmode = 0;
	               PinCfg4.Portnum = 2;
	               PinCfg4.Pinnum = 7;
	               PINSEL_ConfigPin(&PinCfg4);
	               GPIO_SetDir(2, 1<<7, 0);  //input

	               // Initialize E
	                              PINSEL_CFG_Type PinCfg5;
	                              PinCfg5.Funcnum = 0;
	                              PinCfg5.OpenDrain = 0;
	                              PinCfg5.Pinmode = 0;
	                              PinCfg5.Portnum = 2;
	                              PinCfg5.Pinnum = 6;
	                              PINSEL_ConfigPin(&PinCfg5);
	                              GPIO_SetDir(2, 1<<6, 0);  //input

	                   // Initialize D
	                            PINSEL_CFG_Type PinCfg6;
	                            PinCfg6.Funcnum = 0;
	                            PinCfg6.OpenDrain = 0;
	                            PinCfg6.Pinmode = 0;
	                            PinCfg6.Portnum = 2;
	                            PinCfg6.Pinnum = 5;
	                            PINSEL_ConfigPin(&PinCfg6);
	                            GPIO_SetDir(2, 1<<5, 0);  //input
}


int keypad()
{
	int i = -1;
	int d = 1;
	int e = 1;
	int f = 1;

							/*GPIO_SetValue(2, 1<<3);   //Set G
		               	    GPIO_SetValue(2, 1<<4);   //Set H
		               	    GPIO_SetValue(2, 1<<5);  //Set J
		               	    GPIO_SetValue(2, 1<<6);   //Set K*/

	while(i == -1)
	{       	    GPIO_ClearValue(2, 1<<8);   //Clear G
	               	    GPIO_SetValue(2, 1<<10);   //Set H
	               	    GPIO_SetValue(2, 1<<11);  //Set J
	               	    GPIO_SetValue(2, 1<<12);   //Set K

				    d = (GPIO_ReadValue(2) >> 5) & 0x01 ;
	               	            e = (GPIO_ReadValue(2) >> 6) & 0x01;
	               	            f = (GPIO_ReadValue(2) >> 7) & 0x01;


	               	         if (e == 0)   //read from E
	               	             	    	    i = 0;
	               	             	    	if (f == 0)  //read from F
	               	             	    		i = 10;
	               	             	    	if (d == 0)   //read from D
	               	             	    		i = 11;




			GPIO_SetValue(2, 1<<8);   //Set G
	               	GPIO_ClearValue(2, 1<<10);   //Clear H
	               	GPIO_SetValue(2, 1<<11);  //Set J*/
	               	GPIO_SetValue(2, 1<<12);   //Set K




			d = (GPIO_ReadValue(2) >> 5) & 0x01 ;
	                e = (GPIO_ReadValue(2) >> 6) & 0x01;
	                f = (GPIO_ReadValue(2) >> 7) & 0x01;



				if (e == 0)   //read from E
	               	        i = 8;
	               	        if (f == 0)  //read from F
	               	         i = 7;
	               	  if (d == 0)   //read from d
	               		i = 9;



	               	             	    	          GPIO_SetValue(2, 1<<8);   //Set G
	               	             	    	          GPIO_SetValue(2, 1<<10);   //Set H
	               	             	    	          GPIO_ClearValue(2, 1<<11);  //Clear J*/
	               	             	    	          GPIO_SetValue(2, 1<<12);   //Set K


	     d = (GPIO_ReadValue(2) >> 5) & 0x01 ;
	     e = (GPIO_ReadValue(2) >> 6) & 0x01;
	     f = (GPIO_ReadValue(2) >> 7) & 0x01;

	               	             	  if (e == 0)   //read from E
	               	             	  i = 5;
	               	             	  if (f == 0)  //read from F
	               	             	  i = 4;
	               	             	  if (d == 0)   //read from D
	               	             	  i = 6;

	     GPIO_SetValue(2, 1<<8);   //Set G
	     GPIO_SetValue(2, 1<<10);   //Set H
	     GPIO_SetValue(2, 1<<11);  //Set J*/
	     GPIO_ClearValue(2, 1<<12);   //Clear K

         d = (GPIO_ReadValue(2) >> 5) & 0x01;
	 e = (GPIO_ReadValue(2) >> 6) & 0x01;
	 f = (GPIO_ReadValue(2) >> 7) & 0x01;

	     if (e == 0)   //read from E
	     i = 2;
	     if (f == 0)  //read from F
	     i = 1;
	     if (d == 0)   //read from D
	     i = 3;

	}

return i;

}



