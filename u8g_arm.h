
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

#ifndef _U8G_ARM_H
#define _U8G_ARM_H

#include <u8g.h>
#include <stm32f10x_conf.h>
//AO!:
//#define CS_ON()   GPIO_SetBits(GPIOA, GPIO_Pin_6)
//#define CS_OFF()  GPIO_ResetBits(GPIOA, GPIO_Pin_6)

//*************************************************************************
void Delay_mss (void);
void init_SPI1(void);
void SPI_Out_LCD12864_Ap (uint8_t Data_LCD12864);
void LCD_OUT (uint8_t Data_LCD, uint8_t Np_LCD);

uint8_t u8g_com_hw_i2c_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr);
#endif


