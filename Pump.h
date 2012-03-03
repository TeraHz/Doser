#ifndef PUMP_H
#define PUMP_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class Pump {
public:
  Pump( uint8_t pin, uint8_t mls, uint8_t dose, char * desc);
  ~Pump();
  uint8_t getPin( void );
  void setPin( uint8_t );
  uint8_t getMls( void );
  void setMls( uint8_t );
  char* getDescription( void );
  void setDescription( char*);
  uint8_t getDose( void );
  void setDose( uint8_t );
  void save(uint16_t ee);
  void load(uint16_t ee);

private:
  uint8_t _pin; // which digital pin controls the pump
  uint8_t _mls; // how many ml/s does the pump move
  uint8_t _dose; // daily dose for the pump
  char * _desc;
};
#endif

