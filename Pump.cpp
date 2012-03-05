/*
  Pump.cpp - DIY 5 channel doser 
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
#include "Pump.h"
#include "EEPROMAnything.h"


Pump::Pump( uint8_t pin, uint8_t mls, uint16_t dose, char* desc) {
  pinMode(pin, OUTPUT);
  _pin = pin;
  _mls = mls;
  _dose = dose;
  _desc = desc;
}

Pump::~Pump(){}

uint8_t Pump::getPin( void ){
  return _pin;

}

void Pump::setPin( uint8_t pin){
  pinMode(pin, OUTPUT);      // enable the pin
  _pin = pin;
}

uint8_t Pump::getMls( void ){
  return _mls;
}

void Pump::setMls( uint8_t mls ){
  _mls = mls;
}

char* Pump::getDescription( void ){
  return _desc;
}

void Pump::setDescription( char* desc ){
  _desc = desc;
}

uint16_t Pump::getDose( void ){
  return _dose;

}

void Pump::setDose( uint16_t dose ){
  _dose = dose;
}
void Pump::setEE( uint16_t e ){
  _ee = e;
}


void Pump::save(){
  EEPROM_writeAnything(_ee, _mls);
  EEPROM_writeAnything(_ee+10, _dose);
  EEPROM_writeAnything(_ee+20, _desc);
}

void Pump::load(){
  EEPROM_readAnything(_ee, _mls);
  EEPROM_readAnything(_ee+10, _dose);
  EEPROM_readAnything(_ee+20, _desc);
  
}
