#include "Pump.h"

Pump::Pump( uint8_t pin, uint8_t mls, uint8_t dose, char* desc) {
  _pin = pin;
  _mls = mls;
  _dose = dose;
  _desc = desc;
}

uint8_t Pump::getPin( void ){
  return _pin;

}

void Pump::setPin( uint8_t pin){
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

uint8_t Pump::getDose( void ){
  return _dose;

}

void Pump::setDose( uint8_t dose){
  _dose = dose;
}


void Pump::save(uint16_t ee){
  
}

void Pump::load(uint16_t ee){
  
}
