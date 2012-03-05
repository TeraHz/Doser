#ifndef LCDI2C4BIT_H
#define LCDI2C4BIT_H

#include <inttypes.h>



// helpful position constants; lets us put our cursor at start of lines (or add offset, up to 16/20 chars)
#define LCD_CURS_POS_L1_HOME   0x80
#define LCD_CURS_POS_L2_HOME   0xC0
#define LCD_CURS_POS_L3_HOME   0x94
#define LCD_CURS_POS_L4_HOME   0xD4


//LCD physical dimensions
#define LCD_PHYS_ROWS          20
#define LCD_PHYS_LINES          4

#define CHAR_CELL_GRAPH_SIZE   10

#define HORIZ_PIXELS_PER_CHAR   5
#define TOTAL_HORIZ_PIXELS      (HORIZ_PIXELS_PER_CHAR * LCD_PHYS_ROWS)
#define TOTAL_BARGRAPH_PIXELS   (HORIZ_PIXELS_PER_CHAR * CHAR_CELL_GRAPH_SIZE)   // 5 'images' in each of the 10 char cells



//command bytes for LCD
#define CMD_CLR   0x01
#define CMD_RIGHT 0x1C
#define CMD_LEFT  0x18
#define CMD_HOME  0x02


// backlight
#define DEFAULT_BL_LEVEL            255
#define MIN_BL_LEVEL                100




// the LCD i2c device address
#define LCD_MCP_DEV_ADDR      0xA7


// i2c user input pins
#define LCD_MCP_INPUT_PINS_MASK   B01100000     // TODO: pick one for input switch (readable) and one for power-relay (writable)


// IMPORTANT! Wire. must have a begin() before calling init()

class LCDI2C4Bit {

public:

  LCDI2C4Bit( uint8_t devI2CAddress, uint8_t num_lines, uint8_t lcdwidth, uint8_t backlightPin );

  void init();
  void command( uint8_t );
  void write( uint8_t );
  void print( char value[] );
  void print( float );
  void print( int );
  void print(_FLASH_STRING&);
  void print(const __FlashStringHelper*);
  void printL( char value[], uint8_t);
  void clear();
  void backLight( uint8_t );
  void SendToLCD( uint8_t );
  void WriteLCDByte( uint8_t );
  void SetMCPReg( uint8_t, uint8_t );
  uint8_t GetMCPReg( uint8_t );
  void cursorTo( uint8_t, uint8_t );
  void SetInputKeysMask( uint8_t );
  uint8_t ReadInputKeys( void );
  void send_string( const char *str, const uint8_t addr );
  void LCD_cgram_load_boxed_bargraph( void );
  void LCD_cgram_load_normal_bargraph( void );
  void clear_L1( void );
  void clear_L2( void );
  void clear_L3( void );
  void clear_L4( void );
  void clear_line( uint8_t );
  void LCD_turn_display_on( void );
  void LCD_turn_display_off( void );
  void set_backlight_levels( uint8_t my_min, uint8_t my_max);
  void lcd_fade_backlight_complete_off( void );
  void lcd_fade_backlight_off( void );
  void restore_backlight( void );
  void lcd_fade_backlight_on( void );
  void commandWrite( int );
  void setCustomCharacter(uint8_t, uint8_t * );

  unsigned long one_second_counter_ts;
  int seconds;
  uint8_t lcd_inactivity_timeout;               // in seconds: how long do we keep the 'bright' setting before we fade
  uint8_t backlight_admin;    // administratively set (enable auto timeout; normal mode)
  uint8_t backlight_currently_on;
  
  

private:
  uint8_t myNumLines;
  uint8_t myWidth;
  uint8_t myBacklightPin;
  uint8_t myBacklight_max;
  uint8_t myBacklight_min;
  uint8_t dataPlusMask;
  uint8_t myInputKeysMask;
  
};

#endif

