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

void Pump::setMls( uint8_t mls){
  _mls = mls;
}

char* Pump::getDescription( void ){
  return _desc;
}

void Pump::setDescription( char* desc){
  _desc = desc;
}

uint16_t Pump::getDose( void ){
  return _dose;

}

void Pump::setDose( uint16_t dose){
  _dose = dose;
}


void Pump::save(uint16_t ee){
  EEPROM_writeAnything(ee, _mls);
  EEPROM_writeAnything(ee+1, _dose);
  EEPROM_writeAnything(ee+3, _desc);
}

void Pump::load(uint16_t ee){
  EEPROM_readAnything(ee, _mls);
  EEPROM_readAnything(ee+1, _dose);
  EEPROM_readAnything(ee+3, _desc);
  
}
