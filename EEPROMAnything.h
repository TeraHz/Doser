#include <EEPROM.h>
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// ensure this library description is only included once
#ifndef DOSER_h
#define DOSER_h


template <class T> uint16_t EEPROM_writeAnything(uint16_t ee, const T &value)
{
  const byte* p = (const byte*)(const void*)&value;
  uint16_t i;
  for (i = 0; i < sizeof(value); i++)
    EEPROM.write(ee++, *p++);
  return i;
}

template <class T> uint16_t EEPROM_readAnything(uint16_t ee, T &value)
{
  byte* p = (byte*)(void*)&value;
  uint16_t i;
  for (i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(ee++);
  return i;
}
 

#endif
