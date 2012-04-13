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


Pump::Pump( uint8_t pin, float mlm, uint16_t dose, uint8_t dc, char * desc) {
  pinMode(pin, OUTPUT); 
  this->_pin = pin;
  this->_mlm = mlm;
  this->_dose = dose;
  this->_dc = dc;
  this->_desc = desc;
  this->_counter = 0;
}

Pump::~Pump(){}

uint8_t Pump::getPin( void ){
  return this->_pin;

}

void Pump::setPin( uint8_t pin){
  pinMode(pin, OUTPUT);      // enable the pin
  this->_pin = pin;
}

float Pump::getMlm( void ){
  return this->_mlm;
}

void Pump::setMlm( float mlm ){
  this->_mlm = mlm;
}

char* Pump::getDescription( void ){
  return this->_desc;
}

void Pump::setDescription( char* desc ){
  this->_desc = desc;
}

uint16_t Pump::getDose( void ){
  return this->_dose;

}

boolean Pump::isOn( void ){
  return this->_isOn;
}

void Pump::setDose( uint16_t dose ){
  if (dose == 0){
    this->_isOn = false;
  }else{
    this->_isOn = true;
  }
  this->_dose = dose;
}
void Pump::setEE( uint16_t e ){
  this->_ee = e;
}

uint8_t Pump::startDosing( void ){
  analogWrite(this->_pin, (uint8_t)(this->_dc*1.2+135));
  return (uint8_t)(this->_dc*1.2+135);
}

void Pump::startDosing(uint8_t val){
  analogWrite(this->_pin, val);
}

void Pump::stopDosing( void ){
  analogWrite(this->_pin, 0);
}

uint8_t Pump::getDC( void ){
  return this->_dc;
}

void Pump::setDC( uint8_t dc ){
  this->_dc = dc;
}
  
void Pump::save(){
  EEPROM_writeAnything(this->_ee, this->_mlm);
  EEPROM_writeAnything(this->_ee+5, this->_dose);
  EEPROM_writeAnything(this->_ee+10, this->_dc);
  EEPROM_writeAnything(this->_ee+15, this->_desc);
}

void Pump::load(){
  EEPROM_readAnything(this->_ee, this->_mlm);
  EEPROM_readAnything(this->_ee+5, this->_dose);
  EEPROM_readAnything(this->_ee+10, this->_dc);
  EEPROM_readAnything(this->_ee+15, this->_desc);
  
}
