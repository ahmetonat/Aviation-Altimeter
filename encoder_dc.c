
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

#include "encoder_dc.h"

//Retains the state of the keys/contacts for the encoder.
static __IO uint8_t keystate[NumKeys];

void init_ENC (void){
  int i;
  init_ENC_GPIO();

  for (i=0; i<NumKeys;++i){
    keystate[i]=KeyAtRest;
    //    keyoldstate[i]=keystate[i];
  }
}

void init_ENC_GPIO (void){

  GPIO_InitTypeDef GPIO_InitStruct;
  
  // Enable GPIOA (ENC) Clock
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA, ENABLE ); 
  
  GPIO_StructInit (&GPIO_InitStruct);

  // Configure ENC pins
  GPIO_InitStruct.GPIO_Pin = ENC_A_GPIO_PIN| ENC_B_GPIO_PIN| ENC_C_GPIO_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU; //Input pull up.
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(ENC_GPIO_PORT, &GPIO_InitStruct);

}


//************* Functions *******************************************
void check_keypress(uint8_t key){
  // Checks the button passed in the argument, and
  //  debounces it using a digital LPF.
  // The decision is returned as the corresponding element of 
  //  of the variable keystate[] (==KeyPressed or KeyAtRest)
  
  int x;
  int tmp;
  int keyread;
  //  static __IO uint8_t key1dsp;
  static uint8_t keydsp[NumKeys]; //Initialized to zero by compiler.

  //This part depends on the hardware:
  if (key==ENC_A){
    keyread=GPIO_ReadInputDataBit (ENC_GPIO_PORT, ENC_A_GPIO_PIN);
  }else if (key==ENC_B){
    keyread=GPIO_ReadInputDataBit (ENC_GPIO_PORT, ENC_B_GPIO_PIN);
  }else if (key==ENC_C){
    keyread=GPIO_ReadInputDataBit (ENC_GPIO_PORT, ENC_C_GPIO_PIN);
  }
   if (keyread==KeyPressed){
     x=0x3F;
       }else{ //KeyAtIdle;
     x=0;
   }
   // The LPF code is:
   //y(t) = 0.25x(t)+0.75y(t)  DSP based debounce.
   //1.0=FF, 0.25=0x3F, 0.75y(t)= y(t)-(y(t)>>2)

   tmp=(keydsp[key] >>2); //=key1state/4
   keydsp[key]= x+keydsp[key]-tmp; //keydsp*3/4

   //hysteresis:
   if ((keydsp[key]>0xF0) && (keystate[key]==KeyAtRest)){
       keystate[key]=KeyPressed;
     } //else 
     if ((keydsp[key] <0x0F) && (keystate[key]==KeyPressed)){
       keystate[key]=KeyAtRest;
     }
}


// This function must be called every millisecond to check user input.
//////////////////////////////////////////////////////////////////////
// The function checks for 
//   1. Simple press and turn, 
//   2. Double press and 
//   3. Long press of the encoder pushbutton.
//
// It returns the events:
// UP/DOWN if the encoder pushbutton is pressed and turned
// DOUBLECLICK_EVENT if the pushbutton is pressed twice in succession
// LONGPRESS_EVENT if the pushbutton is pressed and held.
//
// The durations for events are set from the #define statements in encoder.h
//////////////////////////////////////////////////////////////////////


int EncRead_Guarded(void){
  int retval;
  static int keyoldstate[NumKeys]; //Static to make it fast...
  static int C_state = C_IDLE;
  static int C_timer;

      // Check the states of the keys:
      keyoldstate[ENC_A]=keystate[ENC_A]; 
      check_keypress(ENC_A);
      keyoldstate[ENC_B]=keystate[ENC_B]; 
      check_keypress(ENC_B);
      keyoldstate[ENC_C]=keystate[ENC_C]; 
      check_keypress(ENC_C);

      ///////////////Above is key status structure//////
      //////////////////////CUT HERE/////////////////
      ///////////////Below is key interpretation.///////
      //
      // Possibilities:
      // 1. Press & turn.
      // 2. Long press C.
      // 3. Double click C.
      //
      //////////////////////////////////////////////////

      retval=NOCHANGE;

      if (keyoldstate[ENC_C]==KeyAtRest&&keystate[ENC_C]==KeyPressed){
	//xprintf("Click.\n");
	//This is a click of key C.
	if (C_state == C_IDLE){ // C is being pressed for the first time.
	  C_state= C_CLICKED1;  //Set the current state
	  C_timer=0;            // Start the timer.
	  //xprintf("Clicked 1st.\n");
	}else if (C_state == C_UNCLICKED1){
	  //xprintf("Clicked 2nd.\n");
	  //The second press of C.
	  //If the designated amount of time has passed, a double click
	  // can be declared.
	  if (C_timer>C_DOUBLECLICK_MINLIM&&C_timer<C_DOUBLECLICK_MAXLIM){
	    //The normal duration of for a double click.
	    // Wait until key is released
	    // Transition into:
	    //xprintf("Double Click. Wait release..\n");
	    C_state= C_CLICKED2;
	    C_timer=0;
	  }else{ //Unsuitable amount of time elapsed. We can return to idle.
	    //xprintf("Unsuitable time. C_timer = %d\n", C_timer);
	    C_state=C_IDLE;
	    C_timer=0;
	  }
	}else{//This must be an error. Reset C_state, reset timer.
	  xprintf("Error 1.\n\n");
	  C_state=C_IDLE;
	  C_timer=0;
	}
      }else if(keyoldstate[ENC_C]==KeyPressed&&keystate[ENC_C]==KeyPressed){
	//Key is being depressed
	if(C_state== C_CLICKED1){
	  //First click. This can be:
	  // 1. Normal press and turn. It will be handled at the bottom.
	  // 2. Double click
	  // 3. Long click.
	  // Last option will be implemented later.
	  ++C_timer;      //Timer was started, so we keep incrementing it.

	  if (C_timer > C_LONGPRESS_DURATION){
	    // Long press event detected.
	    //xprintf("Long press event.\n\n");
	    retval = LONGPRESS_EVENT;
	    C_state= C_IDLE;
	    C_timer=0;
	  }
	}else if (C_state== C_CLICKED2){
	  // Double click event detected.
	  // Wait until key is released. Do nothing until then.
	} else if (C_state==C_ROTATING){
	  //The encoder is used for setting QNH. Do not do anything here.
	  // it will be handled at the end of the function.
	  // This is only a placeholder so that no error is thrown.
	}else {//This must be an error. Reset C_state, reset timer.
	  C_state=C_IDLE;
	  C_timer=0;
	}
      }else if(keyoldstate[ENC_C]==KeyPressed&&keystate[ENC_C]==KeyAtRest){
	//Key release event
	//xprintf("Release.\n");
	if (C_state== C_CLICKED1){ 
	  //xprintf("Released1.\n");
	  //If we are here, then it means a long press was not reached.
	  // If the timing is OK, we can start the timer and 
	  //  transition into waiting for the next click.
	  C_state = C_UNCLICKED1;
	  C_timer=0;
	}else if (C_state== C_CLICKED2){
	  //Declare double click event. Reset state.	
	  //xprintf("Released2.\n");
	  retval = DOUBLECLICK_EVENT;
	  C_state=C_IDLE;
	  C_timer=0;
	} else if (C_state== C_IDLE){
	  //Do nothing. This will be reached if a long press has occurred.
	  // It may be necessary in the menu to wait until the press event
	  // on the encored pushbutton.
	}else if (C_state==C_ROTATING){
	  C_state= C_IDLE; //If the encoder was being rotated, get out of that.
	}else {
	//This must be an error. Reset C_state, reset timer.
	  xprintf("Error 2.\n\n");
	  C_state=C_IDLE;
	  C_timer=0;
	}
      }else if(keyoldstate[ENC_C]==KeyAtRest&&keystate[ENC_C]==KeyAtRest){
	//Key is released.
	if (C_state==C_IDLE){
	  // Nothing to do here. No press.
	}else if (C_state==C_UNCLICKED1){
	  ++C_timer;//Increment timer.
	  if(C_timer> C_DOUBLECLICK_MAXLIM){ //Waited too long. Reset the state.
	    xprintf("Waited too long for double click.\n");
	    C_state= C_IDLE;
	    C_timer=0;
	  }
      }else {
	  //This must be an error. Reset C_state, reset timer.
	  xprintf("Error 3.\n\n");
	  C_state=C_IDLE;
	  C_timer=0;
	}
      }else{
	//This must be an error. Reset C_state, reset timer.
	xprintf("Error 4.\n\n");
	C_state=C_IDLE;
	C_timer=0;
      }

      // Now check UP/DOWN movement. This will override other clicks:

      //Check for the press and turn event. 
      // This will override the rest of the sequence: 
      //if(keyoldstate[ENC_C]!=keystate[ENC_C]){
	if (keystate[ENC_C]==KeyPressed){
	  if (C_state==C_ROTATING){
	  retval = CLICK;  //Signal caller that encoder was pressed.
	  }
	  if(keyoldstate[ENC_A]!=keystate[ENC_A]){ //Edge detected on signal A.
	    C_state=C_ROTATING;	    //Set C_state and C_timer...
	    C_timer=0;
	    if (keystate[ENC_A]==KeyPressed){ //Rising edge on A
	      //xprintf ("A+ ");
	      if(keystate[ENC_B]==KeyPressed){
		//xprintf ("DN ");
		retval = DOWN;
	      }else{
		//xprintf ("UP ");
		retval=UP;
	      }
	    }else if(keystate[ENC_A]==KeyAtRest) {//Falling edge on A
	      //xprintf ("A- ");
              if(keystate[ENC_B]==KeyAtRest){
                //xprintf ("DN ");
                retval = DOWN;
              }else{
                //xprintf ("UP ");
                retval=UP;
              }
	    }   // END: else if(keystate[ENC_A]==KeyAtRest)
	  }	// END: if(keyoldstate[ENC_A]!=keystate[ENC_A])
	}       // END: if (keystate[ENC_C]==KeyPressed)


        /*  //The code below might be useful for debug.
      // If ENC_B is pressed:
      if(keyoldstate[ENC_B]!=keystate[ENC_B]){
	if (keystate[ENC_B]==KeyPressed){
	  //xprintf ("B+ ");
	}else{
	  //xprintf ("B- ");
	}
      }	//END: if ENC_B

      // If ENC_C is pressed:
      if(keyoldstate[ENC_C]!=keystate[ENC_C]){
	if (keystate[ENC_C]==KeyPressed){
	  //LED_ON();
	  //xprintf ("C+ ");
	}else{
	  //LED_OFF();
	  //xprintf ("C- ");
	}
      }	//END: if ENC_C
	*/

      return (retval);
}
