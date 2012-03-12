#include <Flash.h>
#include "LCDi2c4bit.h"
#include <Wire.h>
#include <string.h> //needed for strlen()
#include <inttypes.h>
#include <stdio.h>
#include "mcp23xx.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WConstants.h"
#endif

extern uint8_t lcd_in_use_flag;
extern MCP23XX lcd_mcp;

LCDI2C4Bit::LCDI2C4Bit( uint8_t devI2CAddress, uint8_t num_lines, uint8_t lcdwidth, uint8_t backlightPin ) 
{
  myNumLines = num_lines;
  myWidth = lcdwidth;
  myBacklightPin = backlightPin;
  myInputKeysMask = 0x00;  // default
  myBacklight_max = DEFAULT_BL_LEVEL;
  myBacklight_min = MIN_BL_LEVEL;

  backlight_currently_on = 1;               // boolean


  one_second_counter_ts = 0;
  seconds = 0;
  lcd_inactivity_timeout = 4;               // in seconds: how long do we keep the 'bright' setting before we fade
  backlight_admin = 0;    // administratively set (enable auto timeout; normal mode)

  /*
   * setup and define our pins.  usually a good idea toward the top of each program.
   */

  pinMode(myBacklightPin, OUTPUT);      // pwm backlight
}

void
LCDI2C4Bit::SetInputKeysMask( uint8_t input_keys_mask )
{
  myInputKeysMask = input_keys_mask;
}



uint8_t
LCDI2C4Bit::ReadInputKeys( void )
{
  uint8_t data;

  data = lcd_mcp.get(MCP_REG_GPIO);

  return data;
}



void 
LCDI2C4Bit::SendToLCD( uint8_t data ) 
{
  data |= dataPlusMask;
  lcd_mcp.set(MCP_REG_OLAT, data);
  delayMicroseconds(5);

  data ^= 0x80; // Enable bit ON on LCD
  lcd_mcp.set(MCP_REG_OLAT, data);
  delayMicroseconds(5);

  data ^= 0x80; // Enable bit OFF on LCD
  lcd_mcp.set(MCP_REG_OLAT, data);
  delayMicroseconds(5);
}



void 
LCDI2C4Bit::WriteLCDByte( uint8_t bdata ) 
{
  lcd_in_use_flag = 1;    // set a mutex so that the IR isr won't do anything while we are doing lcd i/o

  SendToLCD( bdata >> 4 );
  SendToLCD( bdata & 0x0F );

  lcd_in_use_flag = 0;    // clear that mutex (so that the IR isr CAN now do things)
}



void 
LCDI2C4Bit::init( void ) 
{
  lcd_mcp.init();
  lcd_in_use_flag = 1;    // set a mutex so that the IR isr won't do anything while we are doing lcd i/o
  dataPlusMask = 0;
  lcd_mcp.set(MCP_REG_IOCON, 0x0C);
  delay(50);
  lcd_mcp.set(MCP_REG_IODIR, myInputKeysMask);
  delay(50);
  lcd_mcp.set(MCP_REG_GPPU, myInputKeysMask);
  delay(50);

  SendToLCD(0x03); 
  delay(5);

  SendToLCD(0x03);
  delayMicroseconds(100);

  SendToLCD(0x03);
  delay(5);

  SendToLCD(0x02);
  WriteLCDByte(0x28);
  WriteLCDByte(0x08);
  WriteLCDByte(0x0C); // turn on, cursor off, no blinking
  delayMicroseconds(60);

  WriteLCDByte(0x01); // clear display
  delay(5);  

  lcd_in_use_flag = 0;    // clear that mutex (so that the IR isr CAN now do things)
}



void 
LCDI2C4Bit::backLight( uint8_t value ) 
{
  analogWrite(myBacklightPin, value);  // 0..255 gets us 'dark' to 'very bright'
}



void 
LCDI2C4Bit::write( uint8_t value ) 
{
  dataPlusMask |= 0x10; // RS
  WriteLCDByte(value);
  dataPlusMask ^= 0x10; // RS
}



void 
LCDI2C4Bit::print( char value[] ) 
{
  for ( char *p = value; *p != 0; p++ ) {
    write(*p);
  }
}

void 
LCDI2C4Bit::printL( char value[], uint8_t chars ) 
{
   uint8_t l = strlen(value);
  for ( char *p = value; *p != 0; p++ ) {
    write(*p);
  }
  for (l; l < chars; l++){
    write(' ');
  }
}



void 
LCDI2C4Bit::clear( void ) 
{
  command(CMD_CLR);
}



void 
LCDI2C4Bit::cursorTo( uint8_t row, uint8_t col ) 
{
  int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };

  command(0x80 | (col + row_offsets[row]));
}


void 
LCDI2C4Bit::command( uint8_t command ) 
{
  // RS - leave low
  WriteLCDByte(command);
  delayMicroseconds(500);
}



void 
LCDI2C4Bit::send_string( const char *str, const uint8_t addr )
{
  //  Send string at addr, if addr <> 0, or cursor position  if addr == 0
  if (addr != 0) {
    command(addr);
  }

  uint8_t i = 0;

  while (str[i] != 0) {
    write(str[i++]);
  }
} 


void 
LCDI2C4Bit::clear_L1( void ) 
{
  // Clear LCD line 1
  clear_line(LCD_CURS_POS_L1_HOME);
}



void 
LCDI2C4Bit::clear_L2( void ) 
{
  //  Clear LCD line 2
  clear_line(LCD_CURS_POS_L2_HOME);
}



void 
LCDI2C4Bit::clear_L3( void ) 
{
  //  Clear LCD line 2
  clear_line(LCD_CURS_POS_L3_HOME);
}

void 
LCDI2C4Bit::clear_L4( void ) 
{
  //  Clear LCD line 2
  clear_line(LCD_CURS_POS_L4_HOME);
}


void
LCDI2C4Bit::clear_line( uint8_t line_num )
{
  command(line_num);

  for (uint8_t i=0; i<LCD_PHYS_ROWS; i++) {
    write(' ');
  }
}

void
LCDI2C4Bit::LCD_turn_display_on( void )
{
  analogWrite(myBacklightPin, myBacklight_max);
}


void
LCDI2C4Bit::LCD_turn_display_off( void )
{
  analogWrite(myBacklightPin, 0);   // zero brightness
}



void 
LCDI2C4Bit::set_backlight_levels( uint8_t my_min, uint8_t my_max )
{
  myBacklight_max = my_max;
  myBacklight_min = my_min;
}


void
LCDI2C4Bit::lcd_fade_backlight_on( void )
{
  for (int bl=myBacklight_min; bl<myBacklight_max; bl++) {
    backLight(bl);    // restore to normal brightness again
    delay(2);
  }

  backlight_currently_on = 1;   // flag it as now on

  // reset things so we start the count-down all over
  seconds = 0;
  one_second_counter_ts = millis();
}


void
LCDI2C4Bit::restore_backlight( void )
{
  /*
   *  if we got a valid IR remote command and the display was off, we better turn it on again!
   */

  if (backlight_currently_on == 0) {
    backLight(myBacklight_max);    // restore to normal brightness again
    delay(5);

    backlight_currently_on = 1;
  }

  // reset things so we start the count-down all over
  seconds = 0;
  one_second_counter_ts = millis();
}


void
LCDI2C4Bit::lcd_fade_backlight_off( void )
{
  for (int bl=myBacklight_max; bl>myBacklight_min; bl--) {
    backLight(bl);   // temporarily turn backlight entirely off (or go to its lowest 'low' setting)
    delay(2);
  }

  backlight_currently_on = 0;   // flag it as now off
}


void
LCDI2C4Bit::lcd_fade_backlight_complete_off( void )
{
  for (int bl=myBacklight_max; bl>0; bl--) {
    backLight(bl);   // temporarily turn backlight entirely off (or go to its lowest 'low' setting)
    delay(2);
  }

  backlight_currently_on = 0;   // flag it as now off
}

void 
LCDI2C4Bit::commandWrite( int command ) {
  // RS - leave low
  WriteLCDByte(command);
  delay(2);
}

void 
LCDI2C4Bit::print( float f){
char tmp[20];
  sprintf(tmp,"%1d",(int)f);
  print(tmp);
  print(".");
  int temp = (f - (int)f) * 100;
  sprintf(tmp,"%02d",abs(temp) );
  print(tmp);
  
}

void 
LCDI2C4Bit::print( int i){
  char tmp[20];
  sprintf(tmp,"%1d",(int)i);
  print(tmp);
}

void
LCDI2C4Bit::print(const __FlashStringHelper *ifsh){
  const prog_char *p = (const prog_char *)ifsh;
  while (1) {
    unsigned char c = pgm_read_byte(p++);
    if (c == 0) return;
      write(c);
  }
}

void 
LCDI2C4Bit::setCustomCharacter(uint8_t slot, uint8_t bitmask[8])
{
	//64 = write CGRAM address
	//slot<<3 = 3 bit (8 possible) cust. char. address
	commandWrite(64 | (slot << 3));

	//write the character to CGRAM
	dataPlusMask |= 0x10;
	commandWrite(bitmask[0]);
	commandWrite(bitmask[1]);
	commandWrite(bitmask[2]);
	commandWrite(bitmask[3]);
	commandWrite(bitmask[4]);
	commandWrite(bitmask[5]);
	commandWrite(bitmask[6]);
	commandWrite(bitmask[7]);
	dataPlusMask ^= 0x10;

	//set DDRAM address to make sure next write command is to DDRAM instead of CGRAM
	//this will trash your cursor position, but prevent you from accidentally overwriting custom characters
	commandWrite(128);
}

