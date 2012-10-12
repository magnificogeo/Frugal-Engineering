/*****************************************************************************
 *   EE2024 Project 2
 *
 *   Done By:
 *   George Moh Yao Cong A0069483R
 *   Oh Joo Siong A0072689M
 *
 *   Copyright(C) 2011, EE2024
 *   All rights reserved.
 *
 ******************************************************************************/


#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_uart.h"
#include "string.h"
#include "light.h"
#include "temp.h"
#include "joystick.h"
#include "pca9532.h"
#include "acc.h"
#include "oled.h"
#include "rgb.h"
#include "led7seg.h"
#include "stdio.h"
#include "uart2.h"
#include "xprintf.h"
#include "rtc.h"
#include "ff.h"
#include "diskio.h"
#include "sound.h"
#include "LPC1700.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h> // for itoa() call
#include "time.h" //wait();
#include "lcd3032.h"
#include "keypad.h"

static void init_LCDpin(void)
   {
	   PINSEL_CFG_Type PinCfg;
	   PinCfg.Funcnum = 0;
	   PinCfg.OpenDrain = 0;
	   PinCfg.Pinmode = 0;
	   PinCfg.Portnum = 0;
	   PinCfg.Pinnum = 21;
	   PINSEL_ConfigPin(&PinCfg);
	   GPIO_SetDir(0, (1 << 21), 1);//CONFIGURE PORT 0 PIN 0

	   PinCfg.Funcnum = 0;
	   PinCfg.OpenDrain = 0;
	   PinCfg.Pinmode = 0;
	   PinCfg.Portnum = 0;
	   PinCfg.Pinnum = 22;
	   PINSEL_ConfigPin(&PinCfg);
	   GPIO_SetDir(0, (1 << 22), 1);//CONFIGURE PORT 0 PIN 1

	   PinCfg.Funcnum = 0;
	   PinCfg.OpenDrain = 0;
	   PinCfg.Pinmode = 0;
	   PinCfg.Portnum = 0;
	   PinCfg.Pinnum = 23;
	   PINSEL_ConfigPin(&PinCfg);
	   GPIO_SetDir(0, (1 << 23), 1);//CONFIGURE PORT 0 PIN 2

	   PinCfg.Funcnum = 0;
	   PinCfg.OpenDrain = 0;
	   PinCfg.Pinmode = 0;
	   PinCfg.Portnum = 0;
	   PinCfg.Pinnum = 24;
	   PINSEL_ConfigPin(&PinCfg);
	   GPIO_SetDir(0, (1 << 24), 1);//CONFIGURE PORT 0 PIN 3

	   PinCfg.Funcnum = 0;
	   PinCfg.OpenDrain = 0;
	   PinCfg.Pinmode = 0;
	   PinCfg.Portnum = 0;
	   PinCfg.Pinnum = 25;
	   PINSEL_ConfigPin(&PinCfg);
	   GPIO_SetDir(0, (1 << 25), 1);//CONFIGURE PORT 0 PIN 4

	   PinCfg.Funcnum = 0;
	   PinCfg.OpenDrain = 0;
	   PinCfg.Pinmode = 0;
	   PinCfg.Portnum = 0;
	   PinCfg.Pinnum = 26;
	   PINSEL_ConfigPin(&PinCfg);
	   GPIO_SetDir(0, (1 << 26), 1);//CONFIGURE PORT 0 PIN 5

	   PinCfg.Funcnum = 0;
	   PinCfg.OpenDrain = 0;
	   PinCfg.Pinmode = 0;
	   PinCfg.Portnum = 0;
	   PinCfg.Pinnum = 27;
	   PINSEL_ConfigPin(&PinCfg);
	   GPIO_SetDir(0, (1 << 27), 1);//CONFIGURE PORT 0 PIN 6

	   PinCfg.Funcnum = 0;
	   PinCfg.OpenDrain = 0;
	   PinCfg.Pinmode = 0;
	   PinCfg.Portnum = 0;
	   PinCfg.Pinnum = 28;
	   PINSEL_ConfigPin(&PinCfg);
	   GPIO_SetDir(0, (1 << 28), 1);//CONFIGURE PORT 0 PIN 7
   }

void waitx ( int seconds )
{
  clock_t endwait;
  endwait = clock () + seconds * CLOCKS_PER_SEC ;
  while (clock() < endwait); {}
}

//Variable msTicks declaration
static uint32_t msTicks = 0;

DWORD AccSize;				/* Work register for fs command */
WORD AccFiles, AccDirs;
FILINFO Finfo;
#if _USE_LFN
char Lfname[512];
#endif

char Line[256];				/* Console input buffer */

FATFS Fatfs[_VOLUMES];		/* File system object for each logical drive */
FIL File[2];				/* File objects */
DIR Dir;					/* Directory object */
BYTE Buff[16384] __attribute__ ((aligned (4))) ;	/* Working buffer */

volatile UINT Timer;


/*---------------------------------------------*/
/* 1kHz timer process                          */
/*---------------------------------------------*/

void SysTick_Handler (void)
{
	static int led_timer;


	STCTRL;		/* Clear IRQ */

	Timer++;	/* Increment performance counter */

	if (++led_timer >= 500) {
		led_timer = 0;
		FIO0PIN ^= 1 << 22;		/* Toggle LED state */
	}

	disk_timerproc();	/* Disk timer process */
}



/*---------------------------------------------------------*/
/* User Provided RTC Function for FatFs module             */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support an RTC.                     */
/* This function is not required in read-only cfg.         */


DWORD get_fattime ()
{
	RTC rtc;

	/* Get local time */
	rtc_gettime(&rtc);

	/* Pack date and time into a DWORD variable */
	return	  ((DWORD)(rtc.year - 1980) << 25)
			| ((DWORD)rtc.month << 21)
			| ((DWORD)rtc.mday << 16)
			| ((DWORD)rtc.hour << 11)
			| ((DWORD)rtc.min << 5)
			| ((DWORD)rtc.sec >> 1);
}


/*--------------------------------------------------------------------------*/
/* Monitor                                                                  */
/*--------------------------------------------------------------------------*/

static
FRESULT scan_files (
	char* path		/* Pointer to the path name working buffer */
)
{
	DIR dirs;
	FRESULT res;
	BYTE i;
	char *fn;


	if ((res = f_opendir(&dirs, path)) == FR_OK) {
		i = strlen(path);
		while (((res = f_readdir(&dirs, &Finfo)) == FR_OK) && Finfo.fname[0]) {
			if (_FS_RPATH && Finfo.fname[0] == '.') continue;
#if _USE_LFN
			fn = *Finfo.lfname ? Finfo.lfname : Finfo.fname;
#else
			fn = Finfo.fname;
#endif
			if (Finfo.fattrib & AM_DIR) {
				AccDirs++;
				*(path+i) = '/'; strcpy(path+i+1, fn);
				res = scan_files(path);
				*(path+i) = '\0';
				if (res != FR_OK) break;
			} else {
			//	xprintf("%s/%s\n", path, fn);
				AccFiles++;
				AccSize += Finfo.fsize;
			}
		}
	}

	return res;
}



static
void put_rc (FRESULT rc)
{
	const char *str =
		"OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
		"INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
		"INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
		"LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";
	FRESULT i;

	for (i = 0; i != rc && *str; i++) {
		while (*str++) ;
	}
	xprintf("rc=%u FR_%s\n", (UINT)rc, str);
}

//Callback function required for temp_init()
static uint32_t getTicks(void)
{
return msTicks;
}



static void init_ssp(void)
{
SSP_CFG_Type SSP_ConfigStruct;
PINSEL_CFG_Type PinCfg;


/*
* Initialize SPI pin connect
* P0.15 - SCK;
* P0.17 - MISO
* P0.18 - MOSI
* P0.16 - SSEL
*/
PinCfg.Funcnum = 2;
PinCfg.OpenDrain = 0;
PinCfg.Pinmode = 0;
PinCfg.Portnum = 0;
PinCfg.Pinnum = 15;
PINSEL_ConfigPin(&PinCfg);
PinCfg.Pinnum = 17;
PINSEL_ConfigPin(&PinCfg);
PinCfg.Pinnum = 18;
PINSEL_ConfigPin(&PinCfg);
PinCfg.Pinnum = 16;
PINSEL_ConfigPin(&PinCfg);



SSP_ConfigStructInit(&SSP_ConfigStruct);


// Initialize SSP peripheral with parameter given in structure above
SSP_Init(LPC_SSP0, &SSP_ConfigStruct);


// Enable SSP peripheral
SSP_Cmd(LPC_SSP0, ENABLE);


}

struct StudentRecord{
int student_id[3];
int teethrecord[2][16];
};

void clearArray(BYTE *bytetransfer) {
	int i;
	for (i=0;i<=100;i++) {
		bytetransfer[i] = 0;
	}
}

void texttoHTML(struct StudentRecord student,int date[]) {


    //int fclose(FILE *a_file);
    int counter = 1;
    int var;
    int i;

    char dateformat[7];
    char studentid[4];


    FATFS fs[2];         /* Work area (file system object) for logical drives */
    FIL fsrc, fdst;      /* fibuhle objects */
    BYTE buffer[4096];   /* file copy buffer */
    FRESULT res;         /* FatFs function common result code */
    DSTATUS dres;
    UINT br, bw;         /* File read/write count */
    DSTATUS stat;
    BYTE bytetransfer[100]={};
    int k;

    sprintf(dateformat,"%d%d %d%d %d%d",date[0],date[1],date[2],date[3],date[4],date[5],date[6]);
    sprintf(studentid,"%d%d%d",student.student_id[0],student.student_id[1],student.student_id[2]);

    char format[] = "%s.htm";
    char filename[sizeof format];
    //filename[sizeof format+1] = '\0';
    //sprintf(filename,format,schoolcode,studentid,dateformat);
    sprintf(filename,format,studentid);
    //FILE *fp = fopen(filename,"w");


    /* Register work area for each volume (Always succeeds regardless of disk status) */
    stat=disk_initialize(0);
  //stat=disk_initialize(1);

    f_mount(0, &fs[0]);
  //f_mount(1, &fs[1]);

    dres= disk_status (0);
  //dres= disk_status (1);

    res = f_open(&fsrc,filename, FA_CREATE_ALWAYS | FA_WRITE);

    for (;;) {
             res = f_read(&fsrc, buffer, sizeof buffer, &br);     /* Read a chunk of src file */
             if (res || br == 0) break; /* error or eof */
         }
    printf("%c",buffer[0]);


    /* Hard code for html skeleton */
	// Creating HTML header
    sprintf(bytetransfer,"<!DOCTYPE HTML>\n");
    res = f_write(&fsrc,bytetransfer,(BYTE)sizeof(bytetransfer),&br );
    clearArray(bytetransfer);
	sprintf(bytetransfer, "<HTML lang='en'>\n");
    res = f_write(&fsrc,bytetransfer,(BYTE)sizeof(bytetransfer),&br );
    clearArray(bytetransfer);
	sprintf(bytetransfer, "<head>\n");
    res = f_write(&fsrc,bytetransfer,(BYTE)sizeof(bytetransfer),&br );
    clearArray(bytetransfer);
	sprintf(bytetransfer, "<meta charset=utf-8>\n");
	res = f_write(&fsrc,bytetransfer,(BYTE)sizeof(bytetransfer),&br );
	clearArray(bytetransfer);
	sprintf(bytetransfer,"<title>Dental Record Form for Student %s on Date %s </title>\n",studentid,dateformat);
	res = f_write(&fsrc,bytetransfer,(BYTE)sizeof(bytetransfer),&br ) ;
	clearArray(bytetransfer);
	sprintf(bytetransfer, "<link rel='stylesheet' type='text/css' href=donotdelete/main.css>\n");
	res = f_write(&fsrc,bytetransfer,(BYTE)sizeof(bytetransfer),&br );
	clearArray(bytetransfer);
	sprintf(bytetransfer, "</head>\n");
	res = f_write(&fsrc,bytetransfer,(BYTE)sizeof(bytetransfer),&br );
	clearArray(bytetransfer);
	//End of HTML Header

	//creating HTML body
	sprintf(bytetransfer, "<body>\n");
	res = f_write(&fsrc,bytetransfer,(BYTE)sizeof(bytetransfer),&br );
	clearArray(bytetransfer);
	sprintf(bytetransfer,"<div class='headerwrapper'>\n");
	res = f_write(&fsrc,bytetransfer,(BYTE)sizeof(bytetransfer),&br );
	clearArray(bytetransfer);
	sprintf(bytetransfer,"<h1>Dental Record for Student %s on Date %s</h1>\n",studentid,dateformat);
	res = f_write(&fsrc,bytetransfer,(BYTE)sizeof(bytetransfer),&br );
	clearArray(bytetransfer);
	sprintf(bytetransfer,"</div>\n");
	res = f_write(&fsrc,bytetransfer,(BYTE)sizeof(bytetransfer),&br );
	clearArray(bytetransfer);



	/* This portion generates the HTML codes for the teeth and the input of values passed in from the device */
	sprintf(bytetransfer, "<div class='teethwrapper'>\n");
	res = f_write(&fsrc,bytetransfer,(BYTE)sizeof(bytetransfer),&br );
	clearArray(bytetransfer);
	sprintf(bytetransfer, "<ul>\n");
	res = f_write(&fsrc,bytetransfer,(BYTE)sizeof(bytetransfer),&br );
	clearArray(bytetransfer);

	counter = 1;
    for (i=counter-1;i<16;i++) {

           sprintf(bytetransfer, "<li><span id='t%d'>%d<button id='statusChange'></span></button></li>\n",counter,student.teethrecord[0][i],counter);
           res = f_write(&fsrc,bytetransfer,(BYTE)sizeof(bytetransfer),&br );
           clearArray(bytetransfer);
           counter++;
           }



    sprintf(bytetransfer, "</ul>\n");
    res = f_write(&fsrc,bytetransfer,(BYTE)sizeof(bytetransfer),&br );
    clearArray(bytetransfer);
    sprintf(bytetransfer, "<ul>\n");
    res = f_write(&fsrc,bytetransfer,(BYTE)sizeof(bytetransfer),&br );
    clearArray(bytetransfer);

    counter=1;
    for (i=counter-1;i<16;i++) {

        sprintf(bytetransfer, "<li><span id='b%d'>%d<button id='statusChange'></span></button></li>\n",counter,student.teethrecord[1][i],counter);
        res = f_write(&fsrc,bytetransfer,(BYTE)sizeof(bytetransfer),&br );
        clearArray(bytetransfer);
        counter++;
        }


	sprintf(bytetransfer, "</ul>\n");
	res = f_write(&fsrc,bytetransfer,(BYTE)sizeof(bytetransfer),&br );
	clearArray(bytetransfer);
	sprintf(bytetransfer, "</div>\n"); //end of teeth
	res = f_write(&fsrc,bytetransfer,(BYTE)sizeof(bytetransfer),&br );
	clearArray(bytetransfer);


	//End of HTML body
	sprintf(bytetransfer,"</body></html>\n");
	res = f_write(&fsrc,bytetransfer,(BYTE)sizeof(bytetransfer),&br );
	clearArray(bytetransfer);
	//fclose(fp);

	if (res) printf("no");
	    else
	    	 printf("yes,write ok"); //Can display SAVE OK on LCD screen


    /* Close open files */
      f_close(&fsrc);
    //f_close(&fdst);

      /* Unregister work area prior to discard it */
      f_mount(0, NULL);
    //f_mount(1, NULL);



}


int check_id(struct StudentRecord data1[100], int temp[7], int j) //j is the number of records in data1
{
	int check = 0;
	int i;
	int k = 0;
	int found = 0;

	 while ((k <= (j+1)) && check==0)//j is the number of records stored in array data1

	    	        	 {
	    	        		 found = 0;
	    	        	 for (i = 0; i < 3; i++)
	    	        	 {
	    	        		 if (data1[k].student_id[i] == temp[i])
	    	        			 {
	    	        			 found = found + 0;
	    	        			 }
	    	        			 else
	    	        			 found++;
	    	        	 }

	    	        	 if (found==0)
	    	        		 {
	    	        		  check = 1;
	    	        		 }
	    	        	 else
	    	        		 k++;

	    	        	 }
	return check;
}

// Function to initialise GPIO to access RED LED

/*************************** Add-on Functions **********************************/


/*************************** END of Add-on Functions ***************************/
int main (void)
{
	    	init_LCDpin();
	    	LCD_init_E();
	    	LCD_set_E(0);
	    	LCD_init_RW();
	    	LCD_init_RS();
	    	init_keypad();

	        LCD_set_RW(0);
	        lcd_init();
	        lcd_clr();
	        lcd_string("Give me a");
	        lcd_2ndline();
	        lcd_string("hell yea!");


	        int j = 0;
	              int k = 0;
	              int intake[9] = {0};
	              char topleft[16];
	              char topright[16];
	              char bottomleft[16];
	              char bottomright[16];

	              int modans;
	               int school_code[3] = {0};
	               int date[6] = {0};
	               int ans;
	               int max;
	               char buffer[10];
	               int record_id[3] = {0};
	               int g = 0;


	               int toothcode[3];
	               int toothstatus;
	               int morestudents;

	               int found = 0;
	               int check = 0;
	               int mode0 = 0;
	               int mode1 = 0;
	               int mode2 = 0;

	               int original_id = 0;
	               int temp_id[7] = {0};
	               int temp_data[17] = {0};
	               int mode;
	               int jaw = 0;
	               int Tcode = 0;
	               int i = 0;

	               struct StudentRecord data1[100] = {0};

	               lcd_clr();

	                       	printf("start\n");
	                       	//school code//

	                           lcd_string("School Code: ");
	                       	//lcd_2ndline();

	                           i = 0;


	                           for (i = 0; i < 4; i++)
	                                    {
	                                        temp_id[i] = keypad();
	                                        Timer0_Wait(250);
	                                        if (temp_id[i] == 10)
	                                            {
	                                              i = i - 2;
	                                              lcd_back();

	                                              }
	                                               		    else
	                                               		    {
	                                               		    if (i != 3)
	                                               			{sprintf(buffer, "%d", temp_id[i]);
	                                               			lcd_string(buffer);
	                                               			Timer0_Wait(250);
	                                               			}
	                                               		    }


	                                    }


	                       	while(temp_id[3] != 11)
	                       	{
	                       		temp_id[3] = keypad();
	                       		Timer0_Wait(250);
	                       	}

	                       	for(i = 0; i < 3; i++)
	                              {
	                       		school_code[i] = temp_id[i];
	                              }


	                           lcd_clr();
	                           //end of school code//


	                           //date//
	                                  	 lcd_string("Enter Date:");
	                                  	 lcd_2ndline();



	                                    for (i = 0; i < 7; i++)
	                                    {
	                                        temp_id[i] = keypad();
	                                        Timer0_Wait(250);
	                                        if (temp_id[i] == 10)
	                                            {
	                                              i = i - 2;
	                                              lcd_back();

	                                              }
	                                               		    else
	                                               		    {
	                                               		    if (i != 6)
	                                               			{sprintf(buffer, "%d", temp_id[i]);
	                                               			lcd_string(buffer);
	                                               			Timer0_Wait(250);
	                                               			}
	                                               		    }


	                                    }

	                                    while(temp_id[6] != 11)
	                                    {
	                                        temp_id[6] = keypad();
	                                        Timer0_Wait(250);
	                                    }

	                                    for(i = 0; i < 6; i++)
	                                       {
	                                        date[i] = temp_id[i];
	                                       }

                     //date


	                                    mode = 3;

	                                    while (mode > 2) {
	                                   	 lcd_clr();
	                                   	         lcd_string("Enter Mode:");
	                                   	         mode = keypad();
	                                   	         sprintf(buffer, "%d", mode);
	                                   	         lcd_string(buffer);
	                                   	         Timer0_Wait(250);
	                                    }

	                                    j =0;
	                                    mode2 = 0;

	                                    //start of infinite while loop
	                                             while(1)
	                                             {
	                                        //start of entering data by tooth code
	                                            if (mode == 0)
	                                            {
	                                                 ans = 1;

	                                            	 morestudents = 1;

	                                            	 if (mode1 == 0 && mode0 == 0)
	                                            	 {j = 0;
	                                            	 }
	                                            	 if (mode2 == 1)
	                                            	 {
	                                            		 j++;
	                                            		 mode2 = 0;
	                                            	 }

	                                            	 mode0 = 1;



	                                            	 while(morestudents == 1)
	                                            	 {
	                                                 lcd_clr();
	                                                 lcd_string("Student ID:");
	                                                 lcd_2ndline();


	                                                 data1[j].student_id[0] = 0;
	                                                 data1[j].student_id[1] = 0;
	                                                 data1[j].student_id[2] = 0;
	                                                 check = 1;
	                                                 while(check == 1)
	                                                 {
	                                                 for (i = 0; i < 4; i++)
	                                                 {
	                                                     temp_id[i] = keypad();
	                                                     Timer0_Wait(250);

	                                                     if(temp_id[i] == 10)
	                                                     {
	                                                    	 i = i - 2;
	                                                    	 lcd_back();
	                                                     }
	                                                     else if(i != 3)
	                                                     {
	                                                	 sprintf(buffer, "%d", temp_id[i]);
	                                                	 lcd_string(buffer);
	                                                	 Timer0_Wait(250);
	                                                     }
	                                                 }
	                                                 check = check_id(data1, temp_id, j);

	                                                 if (check == 1)
	                                                	 {lcd_clr();
	                                                	 lcd_string("ID Exists");
	                                                	 lcd_2ndline();
	                                                	 lcd_string("Student ID:");
	                                                	 }

	                                                 }
	                                                 while(temp_id[3] != 11)
	                                                         	{
	                                                         		temp_id[3] = keypad();
	                                                         		Timer0_Wait(500);
	                                                         	}



	                                                 for(i = 0; i < 3; i++)
	                                                 {
	                                                	 data1[j].student_id[i] = temp_id[i];
	                                                 }


	                                                 //Timer0_Wait(250);

	                                                 Timer0_Wait(500);
	                                                 lcd_clr();

	                                            	ans = 1;

	                                            	 while (ans == 1){

	                                                 lcd_clr();

	                                                 lcd_string("Jaw");


	                                                 for (i = 0; i < 2; i++)
	                                                             {
	                                                                 temp_id[i] = keypad();
	                                                                 Timer0_Wait(250);

	                                                                 if(temp_id[i] == 10)
	                                                                 {
	                                                                	 i = i - 2;
	                                                                	 lcd_back();
	                                                                 }
	                                                                 else if(i != 1)
	                                                                 {
	                                                            	 sprintf(buffer, "%d", temp_id[i]);
	                                                            	 lcd_string(buffer);
	                                                            	 Timer0_Wait(250);
	                                                                 }
	                                                             }

	                                                 while(temp_id[1] != 11)
	                                                      {
	                                                        temp_id[1] = keypad();
	                                                        Timer0_Wait(250);
	                                                      }

	                                                 jaw = temp_id[0];

	                                                 toothcode[0] = 0;
	                                                 toothcode[1] = 0;

	                                            	 lcd_string("Tcode:");

	                                            	 for (i = 0; i < 3; i++)
	                                            	                         {
	                                            	                             temp_id[i] = keypad();
	                                            	                             Timer0_Wait(250);

	                                            	                             if(temp_id[i] == 10)
	                                            	                             {
	                                            	                            	 i = i - 2;
	                                            	                            	 lcd_back();
	                                            	                             }
	                                            	                             else if(i != 2)
	                                            	                             {
	                                            	                        	 sprintf(buffer, "%d", temp_id[i]);
	                                            	                        	 lcd_string(buffer);
	                                            	                        	 Timer0_Wait(250);
	                                            	                             }
	                                            	                         }

	                                            	             while(temp_id[2] != 11)
	                                            	                  {
	                                            	                    temp_id[2] = keypad();
	                                            	                    Timer0_Wait(250);
	                                            	                  }

	                                            	             for (i = 0; i < 2; i++)
	                                            	             {
	                                            	            	 toothcode[i] = temp_id[i];
	                                            	             }


	                                                 lcd_2ndline();
	                                                 lcd_string("Tooth Status:");

	                                                 for (i = 0; i < 2; i++)
	                                                                         {
	                                                                             temp_id[i] = keypad();
	                                                                             Timer0_Wait(250);

	                                                                             if(temp_id[i] == 10)
	                                                                             {
	                                                                            	 i = i - 2;
	                                                                            	 lcd_back();
	                                                                             }
	                                                                             else if(i != 1)
	                                                                             {
	                                                                        	 sprintf(buffer, "%d", temp_id[i]);
	                                                                        	 lcd_string(buffer);
	                                                                        	 Timer0_Wait(250);
	                                                                             }
	                                                                         }

	                                                             while(temp_id[1] != 11)
	                                                                  {
	                                                                    temp_id[1] = keypad();
	                                                                    Timer0_Wait(250);
	                                                                  }

	                                                             toothstatus = temp_id[0];

	                                                 Tcode = toothcode[0]*10 + toothcode[1];

	                                                 data1[j].teethrecord[jaw][Tcode] = toothstatus;

	                                                 lcd_clr();

	                                                 lcd_nocursor();
	                                                 for(i = 0; i < 2; i++)
	                                                 {
	                                                	 for (g = 0; g < 16; g++)
	                                                	 {
	                                                		 sprintf(buffer, "%d", data1[j].teethrecord[i][g]);
	                                                		 lcd_string(buffer);
	                                                	 }

	                                                	 lcd_2ndline();
	                                                 }

	                                                ans = 1;

	                                                texttoHTML(data1[j],date);



	                                                while (ans != 11)
	                                                {
	                                                	ans = keypad();
	                                                	Timer0_Wait(250);

	                                                }


	                                                 lcd_clr();

	                                                 lcd_cursor();


	                                                 lcd_string("More Records?");

	                                                        	 //oled_putString(10,10,buffer,OLED_COLOR_WHITE,OLED_COLOR_BLACK);

	                                                  for (i = 0; i < 2; i++)
	                                                                                                 {
	                                                                                                     temp_id[i] = keypad();
	                                                                                                     Timer0_Wait(250);

	                                                                                                     if(temp_id[i] == 10)
	                                                                                                     {
	                                                                                                    	 i = i - 2;
	                                                                                                    	 lcd_back();
	                                                                                                     }
	                                                                                                     else if(i != 1)
	                                                                                                     {
	                                                                                                	 sprintf(buffer, "%d", temp_id[i]);
	                                                                                                	 lcd_string(buffer);
	                                                                                                	 Timer0_Wait(250);
	                                                                                                     }
	                                                                                                 }

	                                                                                     while(temp_id[1] != 11)
	                                                                                          {
	                                                                                            temp_id[1] = keypad();
	                                                                                            Timer0_Wait(250);
	                                                                                          }

	                                                                                     ans = temp_id[0];
	                                                 }

	                                            	 lcd_clr();
	                                            	 lcd_string("New Student?");

	                                            	 for (i = 0; i < 2; i++)
	                                            	                                     {
	                                            	                                         temp_id[i] = keypad();
	                                            	                                         Timer0_Wait(250);

	                                            	                                         if(temp_id[i] == 10)
	                                            	                                         {
	                                            	                                        	 i = i - 2;
	                                            	                                        	 lcd_back();
	                                            	                                         }
	                                            	                                         else if(i != 1)
	                                            	                                         {
	                                            	                                    	 sprintf(buffer, "%d", temp_id[i]);
	                                            	                                    	 lcd_string(buffer);
	                                            	                                    	 Timer0_Wait(250);
	                                            	                                         }
	                                            	                                     }

	                                            	                         while(temp_id[1] != 11)
	                                            	                              {
	                                            	                                temp_id[1] = keypad();
	                                            	                                Timer0_Wait(250);
	                                            	                              }

	                                            	                         morestudents = temp_id[0];

	                                                         	       	// Timer0_Wait(500);
	                                            	 if (morestudents == 1)
	                                            	 {
	                                            		 j = j + 1;
	                                            	 }

	                                            	 }


	                                                 lcd_clr();

	                                           lcd_string("Which Mode?");

	                                           for (i = 0; i < 2; i++)
	                                                                               {
	                                                                                   temp_id[i] = keypad();
	                                                                                   Timer0_Wait(250);

	                                                                                   if(temp_id[i] == 10)
	                                                                                   {
	                                                                                  	 i = i - 2;
	                                                                                  	 lcd_back();
	                                                                                   }
	                                                                                   else if(i != 1)
	                                                                                   {
	                                                                              	 sprintf(buffer, "%d", temp_id[i]);
	                                                                              	 lcd_string(buffer);
	                                                                              	 Timer0_Wait(250);
	                                                                                   }
	                                                                               }

	                                                                   while(temp_id[1] != 11)
	                                                                        {
	                                                                          temp_id[1] = keypad();
	                                                                          Timer0_Wait(250);
	                                                                        }

	                                                                  mode = temp_id[0];
	                                                                  j++; //increment before going to another mode

	                                            }

	                                        //end of entering data by tooth code

	                                            //start of entering data tooth by tooth
	                                               if (mode == 1)
	                                               {

	                                                 ans = 1;
	                                                 morestudents = 1;


	                                                 if (mode0 == 0 && mode1 == 0)
	                                                 { j = 0;
	                                                 }

	                                                 if (mode2 == 1)
	                                                 {
	                                               	  j++;
	                                               	  mode2 = 0;
	                                                 }
	                                                 mode1 = 1;



	                                               lcd_clr();

	                                               while (morestudents == 1)
	                                               {
	                                                 lcd_clr();
	                                                 lcd_string("Student ID");

	                                                 lcd_2ndline();


	                                                              data1[j].student_id[0] = 0;
	                                                                          data1[j].student_id[1] = 0;
	                                                                          data1[j].student_id[2] = 0;
	                                                                          check = 1;
	                                                                          while(check == 1)
	                                                                          {
	                                                                          for (i = 0; i < 4; i++)
	                                                                          {
	                                                                              temp_id[i] = keypad();
	                                                                              Timer0_Wait(250);

	                                                                              if(temp_id[i] == 10)
	                                                                              {
	                                                                             	 i = i - 2;
	                                                                             	 lcd_back();
	                                                                              }
	                                                                              else if(i != 3)
	                                                                              {
	                                                                         	 sprintf(buffer, "%d", temp_id[i]);
	                                                                         	 lcd_string(buffer);
	                                                                         	 Timer0_Wait(250);
	                                                                              }
	                                                                          }
	                                                                          check = check_id(data1, temp_id, j);

	                                                                          if (check == 1)
	                                                                         	 {lcd_clr();
	                                                                         	 lcd_string("ID Exists");
	                                                                         	 lcd_2ndline();
	                                                                         	 lcd_string("Student ID:");
	                                                                         	 }

	                                                                          }
	                                                                          while(temp_id[3] != 11)
	                                                                                  	{
	                                                                                  		temp_id[3] = keypad();
	                                                                                  		Timer0_Wait(500);
	                                                                                  	}



	                                                                          for(i = 0; i < 3; i++)
	                                                                          {
	                                                                         	 data1[j].student_id[i] = temp_id[i];
	                                                                          }

	                                                        lcd_clr();

	                                                        lcd_string("Enter Data");
	                                                        Timer0_Wait(300);
	                                                        lcd_clr();

	                                                        for (i = 0; i < 2; i++)
	                                                        {
	                                                        	for (g = 0; g < 16; g++)
	                                                        	{
	                                                        		data1[j].teethrecord[i][g] = keypad();
	                                                        		Timer0_Wait(250);
	                                                        		sprintf(buffer, "%d", data1[j].teethrecord[i][g]);
	                                                        	    lcd_string(buffer);
	                                                        	}
	                                                        	lcd_2ndline();
	                                                        }

	                                                        /*for (i = 0; i < 17; i++)
	                                                                             {
	                                                                                 temp_data[i] = keypad();
	                                                                                 Timer0_Wait(250);
	                                                                                 if (temp_data[i] == 10)
	                                                                                     {
	                                                                                       i = i - 2;
	                                                                                       lcd_back();

	                                                                                       }
	                                                                                        		    else
	                                                                                        		    {
	                                                                                        		    if (i != 16)
	                                                                                        			{sprintf(buffer, "%d", temp_data[i]);
	                                                                                        			lcd_string(buffer);
	                                                                                        			Timer0_Wait(250);
	                                                                                        			}
	                                                                                        		    }


	                                                                             }


	                                                                	while(temp_data[16] != 11)
	                                                                	{
	                                                                		temp_data[16] = keypad();
	                                                                		Timer0_Wait(250);
	                                                                	}

	                                                                	for(i = 0; i < 16; i++)
	                                                                       {
	                                                                		data1[j].teethrecord[0][i] = temp_id[i];
	                                                                       }

	                                                                	lcd_2ndline();

	                                                                	for(i = 0; i < 17; i++)
	                                                                	{
	                                                                		temp_data[i] = 0;
	                                                                	}

	                                                                	 for (i = 0; i < 17; i++)
	                                                                	                                  {
	                                                                	                                      temp_data[i] = keypad();
	                                                                	                                      Timer0_Wait(250);
	                                                                	                                      if (temp_data[i] == 10)
	                                                                	                                          {
	                                                                	                                            i = i - 2;
	                                                                	                                            lcd_back();

	                                                                	                                            }
	                                                                	                                             		    else
	                                                                	                                             		    {
	                                                                	                                             		    if (i != 16)
	                                                                	                                             			{sprintf(buffer, "%d", temp_data[i]);
	                                                                	                                             			lcd_string(buffer);
	                                                                	                                             			Timer0_Wait(250);
	                                                                	                                             			}
	                                                                	                                             		    }


	                                                                	                                  }

	                                                                	                     lcd_nocursor();

	                                                                	                     	while(temp_data[16] != 11)
	                                                                	                     	{
	                                                                	                     		temp_data[16] = keypad();
	                                                                	                     		Timer0_Wait(250);
	                                                                	                     	}

	                                                                	                     	for(i = 0; i < 16; i++)
	                                                                	                            {
	                                                                	                     		data1[j].teethrecord[1][i] = temp_data[i];
	                                                                	                            }*/
	                                                       texttoHTML(data1[j],date);
	                                             /*          ans = 1;
	                                                       while(ans != 11)
	                                                       {
	                                                       ans = keypad();
	                                                       Timer0_Wait(250);
	                                                       }
*/

	                                                        lcd_clr();
	                                                        lcd_cursor();


	                                                        lcd_string("New Student?");

	                                                        for (i = 0; i < 2; i++)
	                                                                	                                     {
	                                                                	                                         temp_id[i] = keypad();
	                                                                	                                         Timer0_Wait(250);

	                                                                	                                         if(temp_id[i] == 10)
	                                                                	                                         {
	                                                                	                                        	 i = i - 2;
	                                                                	                                        	 lcd_back();
	                                                                	                                         }
	                                                                	                                         else if(i != 1)
	                                                                	                                         {
	                                                                	                                    	 sprintf(buffer, "%d", temp_id[i]);
	                                                                	                                    	 lcd_string(buffer);
	                                                                	                                    	 Timer0_Wait(250);
	                                                                	                                         }
	                                                                	                                     }

	                                                                	                         while(temp_id[1] != 11)
	                                                                	                              {
	                                                                	                                temp_id[1] = keypad();
	                                                                	                                Timer0_Wait(250);
	                                                                	                              }

	                                                                	                         morestudents = temp_id[0];

	                                                        if (morestudents == 1)
	                                                        {
	                                                            j = j + 1;
	                                                        }
	                                               }

	                                               lcd_clr();
	                                               lcd_string("Which Mode?");
	                                               for (i = 0; i < 2; i++)
	                                                                                          {
	                                                                                              temp_id[i] = keypad();
	                                                                                              Timer0_Wait(250);

	                                                                                              if(temp_id[i] == 10)
	                                                                                              {
	                                                                                             	 i = i - 2;
	                                                                                             	 lcd_back();
	                                                                                              }
	                                                                                              else if(i != 1)
	                                                                                              {
	                                                                                         	 sprintf(buffer, "%d", temp_id[i]);
	                                                                                         	 lcd_string(buffer);
	                                                                                         	 Timer0_Wait(250);
	                                                                                              }
	                                                                                          }

	                                                                              while(temp_id[1] != 11)
	                                                                                   {
	                                                                                     temp_id[1] = keypad();
	                                                                                     Timer0_Wait(250);
	                                                                                   }

	                                                                             mode = temp_id[0];

	                                               j++; //increment before going to different mode


	                                               }
	                                               //end of entering data tooth by tooth


	                                               //start of view records
	                                                  while (mode == 2)
	                                                  {
	                                                  	ans = 1;
	                                                  	mode2 = 1;
	                                                  	while (ans == 1)
	                                                  	           {
	                                                  		       lcd_clr();
	                                                  	           lcd_string("Student ID");
	                                                  	        	//oled_putString(10,10,buffer,OLED_COLOR_WHITE,OLED_COLOR_BLACK);
	                                                  	        	        record_id[0] = 0;
	                                                  	        	        record_id[1] = 0;
	                                                  	        	        record_id[2] = 0;


	                                                  	        	                     for (i = 0; i < 4; i++)
	                                                  	        	                     {
	                                                  	        	                         temp_id[i] = keypad();
	                                                  	        	                         Timer0_Wait(250);

	                                                  	        	                         if(temp_id[i] == 10)
	                                                  	        	                         {
	                                                  	        	                        	 i = i - 2;
	                                                  	        	                        	 lcd_back();
	                                                  	        	                         }
	                                                  	        	                         else if(i != 3)
	                                                  	        	                         {
	                                                  	        	                    	 sprintf(buffer, "%d", temp_id[i]);
	                                                  	        	                    	 lcd_string(buffer);
	                                                  	        	                    	 Timer0_Wait(250);
	                                                  	        	                         }
	                                                  	        	                     }

	                                                  	        	                     while(temp_id[3] != 11)
	                                                  	        	                             	{
	                                                  	        	                             		temp_id[3] = keypad();
	                                                  	        	                             		Timer0_Wait(500);
	                                                  	        	                             	}



	                                                  	        	                     for(i = 0; i < 3; i++)
	                                                  	        	                     {
	                                                  	        	                    	record_id[i] = temp_id[i];
	                                                  	        	                     }
	                                                  	        	        //record_id[0] = keypad();


	                                                  	        	 found = 0;
	                                                  	        	 check = 0;
	                                                  	             k = 0;

	                                                  	    while ((k <=j) && check==0)//j is the number of records stored in array data1

	                                                  	        	 {
	                                                  	        		 found = 0;
	                                                  	        	 for (i = 0; i < 3; i++)
	                                                  	        	 {
	                                                  	        		 if (data1[k].student_id[i] == record_id[i])
	                                                  	        			 {
	                                                  	        			 found = found + 0;
	                                                  	        			 }
	                                                  	        			 else
	                                                  	        			 found++;
	                                                  	        	 }

	                                                  	        	 if (found==0)
	                                                  	        		 {lcd_2ndline();
	                                                  	        		  lcd_string("Record Found");
	                                                  	        		  check = 1;
	                                                  	        		 }
	                                                  	        	 else
	                                                  	        		 k++;

	                                                  	        	 }
	                                                  	             if (check==0)
	                                                  	             {lcd_2ndline();
	                                                  	   		      lcd_string("No Record");
	                                                  	             }
	                                                  	             else
	                                                  	             {
	                                                  	            	 lcd_clr();
	                                                  	            	 for (i = 0; i < 2; i++)
	                                                  	            	 {
	                                                  	            		 for (g = 0; g < 16; g++)
	                                                  	            		 {
	                                                  	            			 printf("%d ", data1[k].teethrecord[i][g]);
	                                                  	            			 sprintf(buffer, "%d", data1[k].teethrecord[i][g]);
	                                                  	            			 lcd_string(buffer);

	                                                  	            		 }
	                                                  	            		 lcd_2ndline();
	                                                  	            	 }
	                                                  	             }

	                                                  	             //Timer0_Wait(1000);

	                                                  	             ans = 1;
	                                                  	             while (ans != 11)
	                                                  	             {
	                                                  	            	 ans = keypad();
	                                                  	            	Timer0_Wait(250);
	                                                  	             }

	                                                  	             lcd_clr();
	                                                  	             lcd_string("More Records?");

	                                                  	             for (i = 0; i < 2; i++)
	                                                  	                                                        {
	                                                  	                                                            temp_id[i] = keypad();
	                                                  	                                                            Timer0_Wait(250);

	                                                  	                                                            if(temp_id[i] == 10)
	                                                  	                                                            {
	                                                  	                                                           	 i = i - 2;
	                                                  	                                                           	 lcd_back();
	                                                  	                                                            }
	                                                  	                                                            else if(i != 1)
	                                                  	                                                            {
	                                                  	                                                       	 sprintf(buffer, "%d", temp_id[i]);
	                                                  	                                                       	 lcd_string(buffer);
	                                                  	                                                       	 Timer0_Wait(250);
	                                                  	                                                            }
	                                                  	                                                        }

	                                                  	                                            while(temp_id[1] != 11)
	                                                  	                                                 {
	                                                  	                                                   temp_id[1] = keypad();
	                                                  	                                                   Timer0_Wait(250);
	                                                  	                                                 }

	                                                  	                                           ans = temp_id[0];
	                                                  	    }
	                                                  	lcd_clr();
	                                                  	lcd_string("Which Mode?");
	                                                  	for (i = 0; i < 2; i++)
	                                                  	                                           {
	                                                  	                                               temp_id[i] = keypad();
	                                                  	                                               Timer0_Wait(250);

	                                                  	                                               if(temp_id[i] == 10)
	                                                  	                                               {
	                                                  	                                              	 i = i - 2;
	                                                  	                                              	 lcd_back();
	                                                  	                                               }
	                                                  	                                               else if(i != 1)
	                                                  	                                               {
	                                                  	                                          	 sprintf(buffer, "%d", temp_id[i]);
	                                                  	                                          	 lcd_string(buffer);
	                                                  	                                          	 Timer0_Wait(250);
	                                                  	                                               }
	                                                  	                                           }

	                                                  	                               while(temp_id[1] != 11)
	                                                  	                                    {
	                                                  	                                      temp_id[1] = keypad();
	                                                  	                                      Timer0_Wait(250);
	                                                  	                                    }

	                                                  	                              mode = temp_id[0];
	                                                  }
	                                                  //end of view records
	                                             }//end of infinite while loop









	            /* This will call the text-to-HTML function */
	           // texttoHTML(student,date);


	        	return 0;

	}

void check_failed(uint8_t *file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while(1);
}

