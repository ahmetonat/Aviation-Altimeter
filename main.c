
/********************************************************************
 * Copyright 2016 Ahmet Onat
 * This program is distributed under the terms of the 
 * GNU General Public License
 *
 * This file is part of Barometer34
 *
 * Barometer34 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Barometer34 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with Barometer34.  If not, see <http://www.gnu.org/licenses/>.
 *
 *********************************************************************/

#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_bkp.h>
#include <stm32f10x_pwr.h>

#include "i2c.h"  //For sensors, other hardware.
#include "ms5611.h"
#include "encoder_dc.h"
#include "board_Naze.h"

#include "u8g_arm.h"
#include "xuart.h"
#include "xprintf.h"
#include <u8g.h>


#include <math.h>

#define TRUE 1
#define FALSE 0

//For u8g_lib:
#define BUFSIZE 25
#define IDLE_WAIT_MS 100
#define INTRO_WAIT_MS 2000
#define BARO_Delay_Max 200

void Delay( uint32_t nTime );
//void init_GPIO (void);

void init_BKP(void);          // Backup QNH in battery backup memory.
void init_USART1 (void);
void disable_JTAG (void);
void Delay( uint32_t nTime ); // Used for MS5611 measurement.

static __IO uint8_t TimerEventFlag;
static __IO uint16_t BARO_Delay;
static __IO uint8_t printQNH; //Signals that QNH must be printed.
static __IO uint8_t printINFO;//Signals that information screen must be printed.

float QNH;  //Global because shared between main and SysTick ISR

//U8g variables. TODO: Make into local!
static u8g_t u8g;

//TODO: Clean up the fonts file:
//#include "u8g_font_6x10.c"
#include "u8g_font_fub11.c"  // Easy to read. But a bit large at 11 pixels.
#define NORMALFONT u8g_font_fub11
#include "u8g_font_fub25n.c"
// TODO: why are fonts in a 'C' file? -> U8glib olikarus?


void draw_Intro(void){ //Show various info about device
  u8g_SetFont(&u8g, NORMALFONT);

  u8g_DrawStr (&u8g, 14, 12, "ALTIMETER!");
  u8g_DrawStr (&u8g, 20, 30, "V34, 2016");
  u8g_DrawStr (&u8g, 10, 45, "GNU GPL V3");
  u8g_DrawStr (&u8g, 15, 64, "Ahmet Onat");
}

//It is unfortunate that QNH as argument here is different from global ONH...
// TODO: Fix it sometime...
void draw_alt(int pressure, int temperature, int altitude_m, int altitude_ft, int QNH)
{
  //Display locations:
  const int BARO_x=2, BARO_y=13;
  //const int TEMP_x=4, TEMP_y=20;
  const int ALTM_x=4, ALTM_y=30;
  const int ALTFT_x=30, ALTFT_y=47;
  const int QNH_x=2, QNH_y=62;
  char array[BUFSIZE];
  u8g_SetFont(&u8g, NORMALFONT);

  if(pressure>0){
    xsprintf(array, "P:%uhpa T:%uC", pressure,temperature);  
  }else{
  xsprintf(array, "P:-%uhpa T:%uC", -pressure,temperature);  
  }
  u8g_DrawStr(&u8g, BARO_x, BARO_y, array);
  //  xsprintf(array, "Temp : %u", temperature);
  //  u8g_DrawStr(&u8g, TEMP_x, TEMP_y, array);

  u8g_DrawStr (&u8g, 0, ALTFT_y-8, "Alt:");
  u8g_DrawStr (&u8g, ALTFT_x+76, ALTFT_y-8, "ft");
  
  xsprintf(array, "QNH: %uhpa", QNH);
  u8g_DrawStr(&u8g, QNH_x, QNH_y, array);


  u8g_SetFont(&u8g, u8g_font_fub25n);  //This also works...
  if (altitude_ft>=0){  //Workaround with %u in displaying negative values.
    xsprintf(array, "%u",altitude_ft);
  }else{
    xsprintf(array, "-%u",-altitude_ft);
  }
  u8g_DrawStr(&u8g, ALTFT_x, ALTFT_y, array);
  //  xsprintf(array, "Alt  : %u m",altitude_m);
  //u8g_DrawStr(&u8g, ALTM_x, ALTM_y, array);

  
  //u8g_DrawRFrame(&u8g, 8,6,96,38,8);// (x,y,h,l,radius)

}

void draw_qnh(int pressure, int temperature, int altitude_m, int altitude_ft, int QNH)
{
  //Display locations:
  const int BARO_x=2, BARO_y=13;
  //  const int TEMP_x=2, TEMP_y=20;
  //  const int ALTM_x=2, ALTM_y=30;
  const int ALTFT_x=2, ALTFT_y=22; //was 28
  const int QNH_x=45, QNH_y=62;
  char array[BUFSIZE];

  u8g_SetFont(&u8g, NORMALFONT);

  //  xsprintf(array, "P:%uhpa, T:%uC", pressure,temperature);  
  //  u8g_DrawStr(&u8g, BARO_x, BARO_y, array);
  //  xsprintf(array, "Temp : %u", temperature);
  //  u8g_DrawStr(&u8g, TEMP_x, TEMP_y, array);

  if (altitude_ft>0){
    xsprintf(array, "Alt: %uft, %um",altitude_ft, altitude_m);
  }else{
    xsprintf(array, "Alt: -%uft, -%um",-altitude_ft, -altitude_m);
  }
  u8g_DrawStr(&u8g, ALTFT_x, ALTFT_y, array);
  //  xsprintf(array, "Alt  : %u m",altitude_m);
  //u8g_DrawStr(&u8g, ALTM_x, ALTM_y, array);

  u8g_DrawStr (&u8g, QNH_x-45, QNH_y-15, "QNH");
  u8g_DrawStr (&u8g, QNH_x-45, QNH_y-2, "hpa");
  u8g_SetFont(&u8g, u8g_font_fub25n);  //This also works...

  xsprintf(array, "%u", QNH);
  u8g_DrawStr(&u8g, QNH_x, QNH_y, array);
  
  //u8g_DrawRFrame(&u8g, 8,6,96,38,8);// (x,y,h,l,radius)

}



////////////////////  MAIN  /////////////////////////////
int main (void) {

  int i;
  float temperature, pressure;
  float QNHPA;
  const int QNH_Calib=2;  //AO: TODO Make this settable at startup and 
                          //write to nvRAM.
  float altitude_m,altitude_ft;
  struct ms5611_vars baro;  

  //Coordinates for writing values on screen:
  uint8_t ALTFT_x=190, ALTFT_y=100;
  uint8_t QNH_x=90,QNH_y=120;       //Location on screen to print QNH
  uint8_t ALTM_x=QNH_x, ALTM_y=140;
  uint8_t BARO_x=QNH_x,BARO_y=160;  //Location on screen to print BARO
  uint8_t TEMP_x=QNH_x,TEMP_y=180;  //Location on screen to print TEMP


  //TODO: Remove obsolete code.
  //TODO: Baro_Delay_Max becomes a #define.
  //  int GPIO_Delay_Max =250; //Led's will stay on for this many ms.
  //  int BARO_Delay_Max =200; //Baro refresh rate.
  
  //Display related:
  int a;

  // Both single speed and double speed access works. 
  // Single speed is slower, needs less memory, vice versa for double speed.
  //u8g_InitComFn(&u8g, &u8g_dev_ssd1306_128x64_i2c, u8g_com_hw_i2c_fn);  
  u8g_InitComFn(&u8g, &u8g_dev_ssd1306_128x64_2x_i2c, u8g_com_hw_i2c_fn); 

  // Initialize hardware:
  disable_JTAG(); //So that some pins (notably LED) are freed up for use.
  init_BKP();     //Battery backup/RTC module init.
  init_ENC();     //Initialize ports connected to encoder.
  init_LED_GPIO();  //Initialize ports connected to LED.

  /* Init Chan's Embedded String Functions (xprintf etc) */
  xdev_out(uart_putc);
  xdev_in(uart_getc);
  init_USART1();

  // Initialize USART1:
  uart_open (USART1, 115200, 0); //USART2 is not supported.
  
  if (SysTick_Config(SystemCoreClock/1000))
    while (1);
  // Every 1 msec, the timer will trigger a call to the SysTick_Handler.   

  xprintf ("STM32F103 Naze32/Flip32.\n\r");
  xprintf("System core clock rate is %d Hz\n\r",SystemCoreClock);

  xprintf("QNH calibration value set to: %d. \n\r",QNH_Calib);
  
  //Turn off LED
  LED_OFF();
  
  QNH = BKP_ReadBackupRegister (BKP_DR1);
  QNH=1013;  //TODO: Naze32 has no provision for backup battery connection.
   // The pin V_bat is connected to supply...
   // The function works correctly. Needs a board with V_bat wired correctly.

  if (QNH<950)  //Quick sanity check.
    QNH=950;
  else if (QNH>1050)
    QNH=1050;
  
  ms5611_init(&baro);
  ms5611_measure(&baro);
  ms5611_calculate(&baro);
  
  QNHPA = (QNH+QNH_Calib)*100;  //Convert to Pa. baro.pressure is also in pa.
  //Note: Uncomment following line for AGL measurement.
  //QNHPA= baro.pressure;   
  
  BARO_Delay = BARO_Delay_Max;      //Counts the number of timer ticks so far.
  
  //Display an introductory info message and wait 3s before starting:
  u8g_FirstPage(&u8g);
  do {
    draw_Intro();
  } while ( u8g_NextPage(&u8g) );
  
  i=0;
  while(i < INTRO_WAIT_MS){  //Keep info screen up for some time.
    if (TimerEventFlag==TRUE){
      TimerEventFlag=FALSE;
      ++i;
    }
  }  
  
  while (1) {
    
    if (TimerEventFlag==TRUE){
      // QNH adjustment used to live here but because of the long delay
      //  in calculating the altitude, it was moved into the ISR.
      TimerEventFlag=FALSE;
    }
    
    //Time to update barometer?:
    if (BARO_Delay >=BARO_Delay_Max){
      BARO_Delay=0;
      
      //LED_ON(); //To check utilization.
      ms5611_measure(&baro);
      ms5611_calculate(&baro);
      
      //Note: Comment following line for AGL measurement.
      QNHPA = (QNH+QNH_Calib)*100;//Convert to Pa. baro.pressure is also in pa.
      altitude_m = 44330*(1- powf((baro.pressure/QNHPA),(0.19029495))); //m
      altitude_ft =altitude_m *3.28084; //ft.
    } //END: if (BARO_Delay >=BARO_Delay_Max)

    if (printQNH==TRUE){  //Time to print QNH:
      printQNH=FALSE;
      u8g_FirstPage(&u8g);
      do {
	draw_qnh((int)baro.pressure/100,(int)baro.temperature/100,(int)altitude_m,(int)altitude_ft,(int)QNH);
      } while ( u8g_NextPage(&u8g) );
      
      BKP_WriteBackupRegister (BKP_DR1, QNH);
    } else if (printINFO==TRUE){
      u8g_FirstPage(&u8g);
      do {
	draw_Intro();
      } while ( u8g_NextPage(&u8g) );
    }else{
      u8g_FirstPage(&u8g);
      do {
	draw_alt((int)baro.pressure/100,(int)baro.temperature/100,(int)altitude_m,(int)altitude_ft,(int)QNH);
      } while ( u8g_NextPage(&u8g) );
    }
    //LED_OFF(); //To check utilization.
      
  } //END: while(1)
} //END: main


//************* Functions *******************************************


//This function does timing for the MS5611 baro sensor,
// based on the ststem clock tick.
static __IO uint32_t TimingDelay;

void Delay( uint32_t nTime ){  //To handle a blocking delay timer. Not precise.
  TimingDelay = nTime ;
  while ( TimingDelay != 0);   //Burn off time while the timer ISR occurs.
}

void SysTick_Handler (void){            // Timer ISR

  int i;

  // TimerEventFlag should not be set when we come here. 
  // This means, we could not handle the previous flag on-time.
  // (Actually, we can't because Barometer access and screen refresh takes too long.)
  if(TimerEventFlag == TRUE){
    // LED_ON();
  }
  
  TimerEventFlag = TRUE;
  ++BARO_Delay;

  if ( TimingDelay != 0x00)  //Added for MS5611 timing.
    TimingDelay --;

      // Check encoder:
      i=EncRead_Guarded();
      if (i==CLICK) printQNH=TRUE;
      if (i==DOWN){
        QNH=QNH-1;
	//xprintf("UP ");
        if (QNH < 900){
          QNH=900;
        }
	printQNH=TRUE;
      }
      if (i==UP){
        QNH=QNH+1;//Increment QNH value
	//xprintf("DN ");
        if (QNH>1100){
          QNH=1100;
        }
        printQNH=TRUE;
      }
      if (i==DOUBLECLICK_EVENT){
	if (printINFO==FALSE){
	  printINFO=TRUE;
	}else{
	  printINFO=FALSE;
	}
      }

}

void init_USART1 (void){
  // Start UART clock:
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_USART1, ENABLE ); 
}

void init_BKP(void){  //Initialize NVRAM registers.
  // Two steps are necessary.
  RCC_APB1PeriphClockCmd (RCC_APB1Periph_BKP|RCC_APB1Periph_PWR,ENABLE);
  PWR->CR |=PWR_CR_DBP; //Turn on write enable of register.

  //Also possible are the following:
  // *(__IO uint32_t *) CR_DBP_BB = ENABLE; //Must define the pointer.
  //PWR_BackupAccessCmd (ENABLE); //This requires pwr lib to be linked.
}


void disable_JTAG (void){
  // Turn off JTAG module to be able to use the GPIO for leds
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  //Both below seem to work OK:
  //GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//Disable only JTAG, keep SWJ on.
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE); //Disable both JTAG&SWJ
}

#ifdef USE_FULL_ASSERT
  void assert_failed ( uint8_t* file, uint32_t line)
  {
    /* Infinite loop */
    /* Use GDB to find out why we're here */
    while (1);
  }
#endif

