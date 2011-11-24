/*
  Doser.h - workaround to latest arduino oro bug.
*/

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
 
int memoryTest() {
  int byteCounter = 0; // initialize a counter
  byte *byteArray; // create a pointer to a byte array
  // More on pointers here: http://en.wikipedia.org/wiki/Pointer#C_pointers

  // use the malloc function to repeatedly attempt allocating a certain number of bytes to memory
  // More on malloc here: http://en.wikipedia.org/wiki/Malloc
  while ( (byteArray = (byte*) malloc (byteCounter * sizeof(byte))) != NULL ) {
    byteCounter++; // if allocation was successful, then up the count for the next try
    free(byteArray); // free memory after allocating it
  }
  
  free(byteArray); // also free memory after the function finishes
  return byteCounter; // send back the highest number of bytes successfully allocated
}

#endif
