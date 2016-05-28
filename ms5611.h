
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

#ifndef MS5611_H
#define MS5611_H

//#define MS5611ADDR 0x77 //Must use 7 bit address, justified left.
#define MS5611ADDR 0xEE

struct ms5611_vars{
  uint32_t ms5611_ut;  // Temp. ADC result
  uint32_t ms5611_up;  // Pressure ADC result
  int32_t pressure;
  int32_t temperature;
  uint16_t CalReg[8];
};

void ms5611_init (struct ms5611_vars * baro); 
void ms5611_measure (struct ms5611_vars * baro); 
void ms5611_calculate (struct ms5611_vars * baro); 

#endif
