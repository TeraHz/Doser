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
  uint8_t getMls( void );
  void setMls( uint8_t );
  char* getDescription( void );
  void setDescription( char*);
  uint16_t getDose( void );
  void setDose( uint16_t );
  void setEE(uint16_t ee);
  void save();
  void load(); // make sure you call setEE before you load

private:
  uint8_t _pin; // which digital pin controls the pump
  uint8_t _mls; // how many ml/s does the pump move
  uint16_t _dose; // daily dose for the pump in ml
  char * _desc;
  uint16_t _ee;
};
#endif

