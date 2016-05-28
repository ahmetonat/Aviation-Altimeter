
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

#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>

#define LED_PORT GPIOB
#define LED_PIN GPIO_Pin_4

void LED_ON(void); //Turn on the LED
void LED_OFF(void); //Turn off
void init_LED_GPIO (void); //Initialize the LED ports.
