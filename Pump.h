/*
  Pump.h - DIY 5 channel doser
  Copyright (C) 2001-2012 Georgi Todorov
  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  https://github.com/TeraHz/Doser
  
*/
#ifndef PUMP_H
#define PUMP_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class Pump {
public:
  Pump( uint8_t pin, uint8_t mls, uint16_t dose, char * desc);
  ~Pump();
  uint8_t getPin( void );
  void setPin( uint8_t );
  float getMlm( void );
  void setMlm( float );
  char* getDescription( void );
  void setDescription( char*);
  uint16_t getDose( void );
  void setDose( uint16_t );
  void setEE(uint16_t ee);
  void startDosing();
  void stopDosing();
  void save();
  void load(); // make sure you call setEE before you load
  boolean isOn();
  
private:
  uint8_t _counter; // count 1 minute of dosing
  uint8_t _pin; // which digital pin controls the pump
  float _mlm; // how many ml/s does the pump move
  uint16_t _dose; // daily dose for the pump in ml
  char * _desc; // some descriptin
  uint16_t _ee; // location in EEPROM
  boolean _isOn; // 0 dose = off
};
#endif

