
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

#include "board_Naze.h"

void LED_ON(void){
  GPIO_WriteBit(LED_PORT,LED_PIN,Bit_RESET);
  }

void LED_OFF(void){
  GPIO_WriteBit(LED_PORT,LED_PIN,Bit_SET);
    }

void init_LED_GPIO (void){

  GPIO_InitTypeDef GPIO_InitStruct;

  // Enable GPIOB (LED) Clock:
  RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOB, ENABLE ); 

  //Initilize the structure values to something sane:
  GPIO_StructInit (&GPIO_InitStruct); 
                                      
  //Initialize LED port:
  GPIO_InitStruct.GPIO_Pin = LED_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(LED_PORT, &GPIO_InitStruct);

  //  LED_ON();
}
