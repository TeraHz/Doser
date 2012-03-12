/*
 * IRremote
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 */

#ifndef IRremote_h
#define IRremote_h

// The following are compile-time library options.
// If you change them, recompile the library.
// If DEBUG is defined, a lot of debugging output will be printed during decoding.
// TEST must be defined for the IRtest unittests to work.  It will make some
// methods virtual, which will be slightly slower, which is why it is optional.
// #define DEBUG
// #define TEST

#include <inttypes.h>


// Results returned from the decoder
class decode_results {
public:
  uint8_t decode_type; // NEC, SONY, RC5, UNKNOWN
  uint32_t value; // Decoded value
  uint8_t bits; // Number of bits in decoded value
  volatile uint16_t *rawbuf; // Raw intervals in .5 us ticks
  uint8_t rawlen; // Number of records in rawbuf.
};

// Values for decode_type
#define NEC 1
#define SONY 2
#define RC5 3
#define RC6 4
#define UNKNOWN -1

// Decoded value for NEC when a repeat code is received
#define REPEAT 0xffffffff

// main class for receiving IR
class IRrecv
{
public:
  IRrecv(uint8_t recvpin);
  void blink13(uint8_t blinkflag);
  uint8_t decode(decode_results *results);
  void enableIRIn();
  void resume();
private:
  // These are called by decode
  uint8_t getRClevel(decode_results *results, uint8_t *offset, uint8_t *used, uint8_t t1);
  uint16_t decodeNEC(decode_results *results);
  uint16_t decodeSony(decode_results *results);
  uint16_t decodeRC5(decode_results *results);
  uint16_t decodeRC6(decode_results *results);
} 
;

// Only used for testing; can remove virtual for shorter code
#ifdef TEST
#define VIRTUAL virtual
#else
#define VIRTUAL
#endif

class IRsend
{
public:
  IRsend() {}
  void sendNEC(uint16_t data, uint8_t nbits);
  void sendSony(uint32_t data, uint8_t nbits);
  void sendRaw(uint16_t buf[], uint8_t len, uint8_t hz);
  void sendRC5(uint32_t data, uint8_t nbits);
  void sendRC6(uint32_t data, uint8_t nbits);
  // private:
  void enableIROut(uint8_t khz);
  VIRTUAL void mark(uint16_t usec);
  VIRTUAL void space(uint16_t usec);
}
;

#define USECPERTICK 50  // microseconds per clock interrupt tick

#endif
