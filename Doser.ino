/*
  Doser.ino - DIY 5 channel doser
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

#include <Flash.h> //from http://arduiniana.org/libraries/flash/
#include <Wire.h>
#include <EEPROM.h> 
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <DS1302.h> //from http://www.henningkarlsen.com/electronics/library.php?id=5
#include "LCDi2c4bit.h"
#include "mcp23xx.h"
#include "EEPROMAnything.h"
#include "IRremote.h"
#include "MenuBackend.h"
#include "Pump.h"
#define PWM_BACKLIGHT_PIN      9  // pwm-controlled LED backlight
#define CONFIG_PIN             8  // pwm-controlled LED backlight
#define IR_PIN                12  // Sensor data-out pin, wired direct
#define ATO_FS1               15  // Analog 1 - float switch 1
#define ATO_FS2               16  // Analog 2 - float switch 2
#define ATO_FS3               17  // Analog 3 - float switch 3
#define ATO_RELAY              4  // Digital 4 - 5V relay for pump

// has to be the last entry in this list
#define K_KEY_SENTINEL       7  // this must always be the last in the list
#define MAX_FUNCTS                  K_KEY_SENTINEL
#define EEPROM_SAVED        500  // EEPROM location to save
#define EEPROM_SAVED_VALUE     3  // EEPROM value to check; change this to reinitialize the pumps

/*
 * IFC = internal function codes
 *
 * these are logical mappings of physical IR keypad keys to internal callable functions.
 * its the way we soft-map keys on a remote to things that happen when you press those keys.
 */
#define K_MENU               0  // enter menu mode
#define K_UP                 1  // up-arrow
#define K_DOWN               2  // down-arrow
#define K_LEFT               3  // left-arrow
#define K_RIGHT              4  // right-arrow
#define K_OK                 5  // Select/OK/Confirm btn
#define K_CANCEL             6  // Cancel/back/exit

uint8_t backlight_min      = 255; // color-independant 'intensity'
uint8_t backlight_max      = 255; // color-independant 'intensity'
uint16_t minCounter        =   0;
uint16_t tempMinHolder     =   0; // this is used for holding the temp value in menu setting
char strTime[20];                 // temporary array for time output
char tmp[20];                     // temporary array for random stuff
uint8_t psecond            =   0; // Previous second
uint8_t backupTimer        =   0; // timer that will count seconds since the ATO started. 
uint8_t backupMax          =  10; // max seconds for timer to run. If this is reached, kill the ATO. (adjust timer based on your pump output)
uint8_t sPos               =   1; // position for setting
uint8_t lcd_in_use_flag    =   0; // is the LCD in use. Used for IR reading
uint8_t ATO_FS1_STATE      =   0; // State holder for flat switch 1
uint8_t ATO_FS2_STATE      =   0; // State holder for flat switch 2
uint8_t ATO_FS3_STATE      =   0; // State holder for flat switch 3

Time currentTime;                 // hold current time

uint8_t i, global_mode,ts, tmi, th, tdw, tdm, tmo, ty;//variables for time
uint16_t key;                     // Store current hex of the IR key
uint8_t calPage            =   0; // Which page on the calibration instruction are we on
uint32_t calTime           =   0; // Store time it took to move X amount of water
float rate                 = 0.0; // rate for current pump
boolean calibrated       = false; // flag that we've gone through calibration

char * calibration_instructions[6] = {
            "Prepare 250ml water.","Prime line.         ",
            "Submerge line input.","Press OK to start.  ",
            "Press OK when all   ","water is gone       "};
    
boolean first = true;
// this is a temporary holding area that we write to, key by key; and then dump all at once when the user finishes the last one
uint16_t ir_key_bank1[MAX_FUNCTS+1];
decode_results results;

// this is used in learn-mode, to prompt the user and assign enums to internal functions
struct _ir_keys {
  long hex;
  uint8_t internal_funct_code;
  char name[16];
}

ir_keys[MAX_FUNCTS+1] = {
  { 0x00, K_MENU,          "Menu"           }
 ,{ 0x00, K_UP,            "Up Arrow"       }
 ,{ 0x00, K_DOWN,          "Down Arrow"     }
 ,{ 0x00, K_LEFT,          "Left Arrow"     }
 ,{ 0x00, K_RIGHT,         "Right Arrow"    }
 ,{ 0x00, K_OK,            "Confirm/Select" }
 ,{ 0x00, K_CANCEL,        "Back/Cancel"    }
 ,{ 0x00, K_KEY_SENTINEL,  "NULL"           }
};

//Init the Real Time Clock
DS1302 rtc(13, 7, 2);


//Init the MCP port expander for LCD
MCP23XX lcd_mcp = MCP23XX(LCD_MCP_DEV_ADDR);

//Init the LCD
LCDI2C4Bit lcd = LCDI2C4Bit(LCD_MCP_DEV_ADDR, LCD_PHYS_LINES, LCD_PHYS_ROWS, PWM_BACKLIGHT_PIN);

//Init the Pumps
//Pump(pin number, ml/s, daily dose, dsecription);
Pump * p1 = new Pump(3,1,0,"Pump 1");
Pump * p2 = new Pump(5,0,0,"Pump 2");
Pump * p3 = new Pump(6,0,0,"Pump 3");
Pump * p4 = new Pump(11,0,0,"Pump 4");
Pump * p5 = new Pump(10,0,0,"Pump 5");
Pump * currentPump;

//this controls the menu backend and the event generation
MenuBackend menu = MenuBackend(menuUseEvent,menuChangeEvent);
//beneath is list of menu items needed to build the menu
MenuItem settings = MenuItem("Settings");
MenuItem mi_clock = MenuItem("Clock");
MenuItem mi_sleep = MenuItem("Display Sleep");
MenuItem mi_pump1 = MenuItem("Pump 1");
MenuItem mi_pump1_review = MenuItem("Review");
MenuItem mi_pump1_set = MenuItem("Set");
MenuItem mi_pump1_calibrate = MenuItem("Calibrate");
MenuItem mi_pump2 = MenuItem("Pump 2");
MenuItem mi_pump2_review = MenuItem("Review");
MenuItem mi_pump2_set = MenuItem("Set");
MenuItem mi_pump2_calibrate = MenuItem("Calibrate");
MenuItem mi_pump3 = MenuItem("Pump 3");
MenuItem mi_pump3_review = MenuItem("Review");
MenuItem mi_pump3_set = MenuItem("Set");
MenuItem mi_pump3_calibrate = MenuItem("Calibrate");
MenuItem mi_pump4 = MenuItem("Pump 4");
MenuItem mi_pump4_review = MenuItem("Review");
MenuItem mi_pump4_set = MenuItem("Set");
MenuItem mi_pump4_calibrate = MenuItem("Calibrate");
MenuItem mi_pump5 = MenuItem("Pump 5");
MenuItem mi_pump5_review = MenuItem("Review");
MenuItem mi_pump5_set = MenuItem("Set");
MenuItem mi_pump5_calibrate = MenuItem("Calibrate");
MenuItem mi_ATO = MenuItem("ATO");
MenuItem mi_ATO_set = MenuItem("Set timeot");


IRrecv irrecv(IR_PIN);

void setup()
{
#ifdef DEBUG
  Serial.begin(9600);
#endif
//  Serial.println("Hello");
  Wire.begin();
  // Enable config pin's pullup
  pinMode(CONFIG_PIN,INPUT);
  digitalWrite(CONFIG_PIN,HIGH);
  pinMode(ATO_FS1, INPUT);
  // Enable Float switch 1 pin's pullup
  digitalWrite(ATO_FS1,HIGH);
  pinMode(ATO_FS2, INPUT);
  // Enable Float switch 2 pin's pullup
  digitalWrite(ATO_FS2,HIGH);
  pinMode(ATO_FS3, INPUT);
  // Enable Float switch 3 pin's pullup
  digitalWrite(ATO_FS3,HIGH);
  pinMode(ATO_RELAY, OUTPUT);
  p1->setEE(100);
  p2->setEE(150);
  p3->setEE(200);
  p4->setEE(250);
  p5->setEE(300);
  
  if (EEPROM.read(EEPROM_SAVED) != EEPROM_SAVED_VALUE) {
    p1->save();
    p2->save();
    p3->save();
    p4->save();
    p5->save();
    EEPROM.write(EEPROM_SAVED,EEPROM_SAVED_VALUE);
  }else{
    p1->load();
    p2->load();
    p3->load();
    p4->load();
    p5->load();
  }
  //start IR sensor
  irrecv.enableIRIn();
#ifdef DEBUG_PUMP
  Serial.print(F("Pump 1 - pin:"));
  Serial.print(p1->getPin());
  Serial.print(F("desc:"));
  Serial.println(p1->getDescription());
  Serial.print(F("Pump 2 - pin:"));
  Serial.print(p2->getPin());
  Serial.print(F("desc:"));
  Serial.println(p2->getDescription());
  Serial.print(F("Pump 3 - pin:"));
  Serial.print(p3->getPin());
  Serial.print(F("desc:"));
  Serial.println(p3->getDescription());
  Serial.print(F("Pump 4 - pin:"));
  Serial.print(p4->getPin());
  Serial.print(F("desc:"));
  Serial.println(p4->getDescription());
  Serial.print(F("Pump 5 - pin:"));
  Serial.print(p5->getPin());
  Serial.print(F("desc:"));
  Serial.println(p5->getDescription());
#endif
  // Setup Serial connection
  //start LCD
  lcd.init();
  lcd.SetInputKeysMask(LCD_MCP_INPUT_PINS_MASK);
  lcd.backLight(255);


// // Set the clock to run-mode, and disable the write protection
//    rtc.halt(false);
//    rtc.writeProtect(false);
//  // The following lines can be commented out to use the values already stored in the DS1302
//    rtc.setDOW(WEDNESDAY);        // Set Day-of-Week to FRIDAY
//    rtc.setTime(22, 58, 00);     // Set the time to 12:00:00 (24hr format)
//    rtc.setDate(11, 01, 2012);   // Set the date to August 6th, 2010
  rtc.setTCR(TCR_D1R2K);
  if (digitalRead(CONFIG_PIN) == LOW){
    enter_setup_mode();
  }

  menuSetup();
 //read remote keys from EEPROM  
  for (i=0; i<=MAX_FUNCTS; i++) {
    EEPROM_readAnything(40 + i*sizeof(key), key);
    ir_keys[i].hex = key;
  }
}

void loop()
{
  // Get data from the DS1302
  currentTime = rtc.getTime();
  uint8_t second = currentTime.sec;
  if (global_mode == 0) {     //  home screen
    onKeyPress();
  }//global_mode == 0
  else if (global_mode == 1){ //we're in menu
    if (first){
      while (menu.getCurrent().getLeft() != 0){
        menu.moveLeft();
      }
      lcd.clear_L1();
      lcd.clear_L2();
      lcd.clear_L3();
      lcd.cursorTo(0,0);
      lcd.print((char*)menu.getCurrent().getName());
      if (menu.getCurrent().getRight() != 0){
        lcd.cursorTo(1,3);
        lcd.print(F(">"));
      }else{
        lcd.cursorTo(1,3);
        lcd.print(F(" "));
      }
      first=false;
    }
    show_menu();
  } //global_mode == 1
  
  else if (global_mode == 2){
    cal_pump();
  }//global_mode == 2
  
  else if (global_mode == 3){
    set_pump();
  }//global_mode == 3
  
  else if (global_mode == 4){
    set_time();
  }//global_mode == 4
  
  else if (global_mode == 5){
    handle_review_pump();
  }//global_mode == 5
  
  else{//we're somewhere else?
    
  }
  
  if (psecond != second){
      psecond = second;
      run_sec();
  }
  delay(50);
}


/*********************************/
/****** RUN ONCE PER SECOND ******/
/*********************************/
void run_sec( void ){
  
//  // It is midnight. Might be useful for something in the future :)
//  if (currentTime.hour == 0 && currentTime.min == 0 && currentTime.sec == 0){
//  }
      
  do_ATO();
  if (global_mode != 2){ // we don't want dosing to occur while we use the pumps for calibration
    do_DOSING(p1);
    do_DOSING(p2);
    do_DOSING(p3);
    do_DOSING(p4);
    do_DOSING(p5);
  }
  update_clock(3,0);
  
  if (!first && !calibrated){
    calTime++;
  }
#ifdef DEBUG
  switch (global_mode){
    case 0:
      Serial.println(F("Global mode"));
      break;
    case 1:
      Serial.println(F("Menu"));
      break;
    case 2:
      Serial.println(F("Calibration"));
      break;
    case 3:
      Serial.println(F("Set Dose"));
      break;
    case 4:
      Serial.println(F("Set Time"));
      break;
    case 5:
      Serial.println(F("Review"));
      break;
  }
#endif
  Serial.print("Mem: "); Serial.println(availableMemory());
}

/****************************/
/****** PERFORM DOSING ******/
/****************************/
void do_DOSING(Pump *pump){
  if (pump->isOn()){
    uint8_t repeat = 1440/pump->getDose();
    if ((currentTime.hour*60+currentTime.min)%repeat == 0){
      pump->startDosing();
    }else{
      pump->stopDosing();
    }
  }

}

/*******************************/
/****** PRINT TIME AT X,Y ******/
/*******************************/
void update_clock(uint8_t x, uint8_t y){
  if (global_mode == 0 || global_mode == 1 || global_mode == 5){
    lcd.cursorTo(x,y);
    lcd.print(rtc.getTimeStr());
    lcd.print(F("  "));
    lcd.print(rtc.getDateStr());
  }
}


/*******************************/
/****** PRINT TEMP AT X,Y ******/
/*******************************/
void update_temp(uint8_t x, uint8_t y){
  lcd.cursorTo(x,y);
  lcd.print(strTime);
}

/********************************/
/****** INITIAL SETUP MODE ******/
/********************************/
void enter_setup_mode( void )  {

  uint8_t setup_finished = 0;
  uint8_t idx = 0, i = 0;
  uint8_t blink_toggle = 0;
  uint8_t blink_count = 0;
  float ratio;
  uint8_t eeprom_index = 0;
  uint8_t key_pressed = 0;


  lcd.clear();
  lcd.cursorTo(0,0);
  lcd.print(F("Remote Learning"));

  idx = 0;
  while (!setup_finished) {
    if (!strcmp(ir_keys[idx].name, "NULL")) {
      setup_finished = 1;   // signal we're done with the whole list  
      goto done_learn_mode;
    }  

    // we embed the index inside our array of structs, so that even if the user comments-out blocks
    // of it, we still have the same index # for the same row of content
    eeprom_index = ir_keys[idx].internal_funct_code;

    // prompt the user for which key to press
    lcd.send_string(ir_keys[idx].name, LCD_CURS_POS_L2_HOME+1);
    delay(300);

    blink_toggle = 1;
    blink_count = 0;

    /*
     * non-blocking poll for a keypress
     */

    while ( (key = get_input_key()) == 0 ) {

      if (blink_toggle == 1) {
        blink_toggle = 0;
        lcd.clear_L2();  // clear the string
        delay(300);  // debounce
      } 
      else {
        blink_toggle = 1;
        ++blink_count;
        lcd.send_string(ir_keys[idx].name, LCD_CURS_POS_L2_HOME+1);  // redraw the string
        delay(600);  // debounce
      }


      // check if we should exit (user got into this mode but had 2nd thoughts ;)
      if ( blink_count >= 30 ) {    // change the value of '30' if you need more time to find your keys ;)
        setup_finished = 1;
        global_mode = 0;           // back to main 'everyday use' mode

        lcd.clear();
        lcd.cursorTo(0,0);
        lcd.print(F("Abandon SETUP"));


        /*
         * read LAST GOOD soft-set IR-key mappings from EEPROM
         */

        for (i=0; i<=MAX_FUNCTS; i++) {
          EEPROM_readAnything(40 + i*sizeof(key), key);
          ir_keys[i].hex = key;
        }

        delay(1000);

        lcd.clear();
        return;

      } // if blink count was over the limit (ie, a user timeout)

    } // while


    // if we got here, a non-blank IR keypress was detected!
    lcd.send_string("*", LCD_CURS_POS_L2_HOME);
    lcd.send_string(ir_keys[idx].name, LCD_CURS_POS_L2_HOME+1);  // redraw the string

    delay(1000);  // debounce a little more


    // search the list of known keys to make sure this isn't a dupe or mistake
    // [tbd]


    // accept this keypress and save it in the array entry that matches this internal function call
    ir_key_bank1[eeprom_index] = key;

    idx++;  // point to the next one

    irrecv.resume(); // we just consumed one key; 'start' to receive the next value
    delay(300);

  } // while



done_learn_mode:
  global_mode = 0;           // back to main 'everyday use' mode
  lcd.clear();
//  lcd.send_string("Learning Done", LCD_CURS_POS_L1_HOME);
//  delay(500);
//  lcd.send_string("Saving Key Codes", LCD_CURS_POS_L2_HOME);

  // copy (submit) all keys to the REAL working slots
  for (i=0; i<=MAX_FUNCTS; i++) {
    ir_keys[i].hex = ir_key_bank1[i];
    EEPROM_writeAnything(40 + i*sizeof(ir_key_bank1[i]), ir_key_bank1[i]);    // blocks of 4 bytes each (first 40 are reserved, though)
    ratio = (float)i / (float)idx;

    delay(50);
  }
  first = true;
  delay(1000);

  lcd.clear();
}

/*******************************/
/****** GET INFRARED KEY ******/
/******************************/
long get_input_key( void ) {
  long my_result;
  long last_value = results.value;   // save the last one in case the new one is a 'repeat code'

  if (irrecv.decode(&results)) {

    // fix repeat codes (make them look like truly repeated keys)
    if (results.value == 0xffffffff) {
      if (last_value != 0xffffffff) {  
        results.value = last_value;
      } 
      else {
        results.value = 0;
      }

    }

    if (results.value != 0xffffffff) {
      my_result = results.value;
    } 
    else {
      my_result = last_value;  // 0;
    }

    irrecv.resume();    // we just consumed one key; 'start' to receive the next value
    return results.value; //my_result;
  }
  else {
    return 0;   // no key pressed
  }
}


/**************************************/
/****** SET PUMP pump TO PWM val ******/
/**************************************/
void update_pump(uint8_t pump, uint8_t val){
  //      pinMode(pumps[pump], val);
  //      char tmpStr[5];
  //      sprintf(tmpStr,"%d:%03d ",pump+1, val);
//  if (pump*7 < 20){
//    lcd.cursorTo(0,pump*7);
//  }
//  else{
//    lcd.cursorTo(1,(pump-3)*7+4);
//  }
//  lcd.print(pump+1);
//  lcd.print(":");
//  lcd.print(val);
//  lcd.print("  ");
}

/********************************************/
/****** SETUP LCD FOR pump CALIBRATION ******/
/********************************************/
void pump_menu_cal(Pump *pump){
  currentPump = pump;
#ifdef DEBUG_MENU
  Serial.print(F("Setting curentTemp to "));
  Serial.print(pump->getDescription());
  Serial.print("->");
  Serial.println(currentPump->getDescription());
#endif
  lcd.clear();
  lcd.cursorTo(0,0);  
  lcd.printL(pump->getDescription(), 8);
  lcd.print(F("Calibration"));
  global_mode = 2;
  first = true;
  calPage = 0;
  update_cal_screen();
  calTime = 0;
  cal_pump();
}

/********************************************/
/****** PRINT CURRENT PAGE OF CAL INST ******/
/********************************************/
void update_cal_screen(){
  lcd.clear_L4();   
  if (calPage !=3){
    lcd.cursorTo(1,0);
    lcd.print(calibration_instructions[calPage*2]);  
    lcd.cursorTo(2,0);
    lcd.print(calibration_instructions[calPage*2+1]);
    if (calPage != 0 && first){
      lcd.cursorTo(3,0);
      lcd.print(F("<Previous"));
    }
    if (calPage != 2){
      lcd.cursorTo(3,15);
      lcd.print(F("Next>"));
    }
  }else{
    lcd.cursorTo(1,0);
    lcd.print(F("Stats:              "));
    lcd.cursorTo(2,0);

    rate = 250.0*60.0/calTime;
    uint8_t frate = (uint8_t)rate; // compute the whole part of the float
    uint16_t prate = (rate - frate)*100; // compute up to 2 decimal digits accuracy
    sprintf(tmp,"t:%04ds ",calTime);
    lcd.print(tmp);
    sprintf(tmp,"r:%03u.",frate);
    lcd.print(tmp);
    sprintf(tmp,"%02uml/m", prate);
    lcd.print(tmp);
    lcd.cursorTo(3,0);
    lcd.print(F("Press OK to store   "));
  }
}


/**************************************/
/****** SETUP LCD FOR pump SETUP ******/
/**************************************/
void pump_menu_set(Pump *pump){
  global_mode = 3;
  tempMinHolder = pump->getDose();
  currentPump = pump;
#ifdef DEBUG_MENU
  Serial.print(F("Setting curentTemp to "));
  Serial.print(pump->getDescription());
  Serial.print("->");
  Serial.println(currentPump->getDescription());
#endif
  lcd.clear(); 
  lcd.cursorTo(0,0);  
  lcd.printL(pump->getDescription(), 8);
  lcd.print(F("Setup"));
  lcd.cursorTo(2,0);
  lcd.print(F("Enter daily dose:"));  
  lcd.cursorTo(3,0);
  sprintf(tmp,"   %05u ml",pump->getDose()*pump->getMlm());
  lcd.print(tmp);
  set_pump();
}

/**********************/
/****** SET PUMP ******/
/**********************/
void set_pump(){
  float mlm = currentPump->getMlm();
  key = get_input_key();
  if (key == 0) {
    return;
  }
  delay(50);
  // key = OK
  if (key == ir_keys[K_OK].hex ) {
    currentPump->setDose(tempMinHolder);
    currentPump->save();
    global_mode = 0;
    lcd.clear();
    first=true;
  }

  // key = Up
  else if (key == ir_keys[K_UP].hex){
    if (tempMinHolder < 1440){
      lcd.clear_L2();
      tempMinHolder++;
    } 
    else{
      tempMinHolder = 1440;
      lcd.cursorTo(1,0);
      sprintf(tmp,"Pump max: %luml",(long)(mlm*1440));
      lcd.print(tmp);
      delay(700);
      lcd.clear_L2();
    }
    lcd.cursorTo(3,0);
    sprintf(tmp,"   %05lu ml",(long)(tempMinHolder*mlm));
    lcd.print(tmp);
  }

  // key = Down
  else if (key == ir_keys[K_DOWN].hex){

    if (tempMinHolder > 0){
      tempMinHolder--;
    }
    else{
      tempMinHolder = 0;
      lcd.cursorTo(1,0);
      lcd.print(F("Turning pump OFF    "));
    }
    lcd.cursorTo(3,0);
    sprintf(tmp,"   %05lu ml",(long)(tempMinHolder*mlm));
    lcd.print(tmp);
  }

  // key = Left
  else if (key == ir_keys[K_LEFT].hex){
    if (tempMinHolder > 10){
      tempMinHolder-=10;
    }
    else{
      tempMinHolder = 0;
      lcd.cursorTo(1,0);
      lcd.print(F("Turning pump OFF    "));
    }
    lcd.cursorTo(3,0);
    sprintf(tmp,"   %05lu ml",(long)(tempMinHolder*mlm));
    lcd.print(tmp);
  }

  // key = Right
  else if (key == ir_keys[K_RIGHT].hex){ 
   if (tempMinHolder <= 1430){
      tempMinHolder+=10;
    } 
    else{
      tempMinHolder = 1440;
      lcd.cursorTo(1,0);
      sprintf(tmp,"Pump max: %luml",(long)(mlm*1440));
      lcd.print(tmp);
      delay(700);
      lcd.clear_L2();
    }

    lcd.cursorTo(3,0);
    sprintf(tmp,"   %05lu ml",(long)(tempMinHolder*mlm));
    lcd.print(tmp);
  }

  // key = Cancel
  else if (key == ir_keys[K_CANCEL].hex){
    lcd.clear();
    global_mode = 0;
    delay (100);
    first = true;
  }else{
#ifdef DEBUG
    Serial << F("unknown key") << "\n\r";
#endif
  }

  delay(100);
}


/****************************/
/****** CALIBRATE PUMP ******/
/****************************/
void cal_pump(){
    key = get_input_key();
  if (key == 0) {
    return;
  }
  delay(50);
  // key = OK
  if (key == ir_keys[K_OK].hex ) {
    if (first){
      if (calibrated){
        currentPump->setMlm(rate);
        currentPump->save();
        calibrated=false;
        calPage = 0;
        global_mode = 0;
        lcd.clear();
      }else{
        first = false;
        calibrated = false;
        currentPump->startDosing();
        calPage = 2;
        update_cal_screen();
      }
    }else{
      first = true;
      calibrated  = true;
      currentPump->stopDosing();
      calPage = 3;
      update_cal_screen();
    }
  }

  // key = Left
  else if (key == ir_keys[K_LEFT].hex){
   if (calPage >0 && first){
     calPage--;
     update_cal_screen();
   }
  }

  // key = Right
  else if (key == ir_keys[K_RIGHT].hex){ 
    if (calPage < 2){
      calPage++;
      update_cal_screen();
    }
  }

  // key = Cancel
  else if (key == ir_keys[K_CANCEL].hex){
    currentPump->stopDosing();
    lcd.clear();
    global_mode = 0;
    first = true;
    calibrated  = true;
  }else{
#ifdef DEBUG
    Serial << F("unknown key") << "\n\r";
#endif
  }

  delay(100);
  
}

/*********************************/
/****** PREPARE REVIEW PUMP ******/
/*********************************/
void prep_review_pump(Pump *pump){
  global_mode = 5;
  lcd.clear_L2();
  lcd.clear_L3();
  tempMinHolder = pump->getDose();
  lcd.cursorTo(0,8);
  lcd.print(F("Review"));
  lcd.cursorTo(1,0);
  lcd.print(F("ml per minute:"));
  lcd.print(pump->getMlm());  
  lcd.cursorTo(2,0);
  lcd.print(F("daily dose: "));
  lcd.print((int)(pump->getDose()*pump->getMlm()));
  lcd.print("ml");
  handle_review_pump();
}
/*************************/
/****** REVIEW PUMP ******/
/*************************/
void handle_review_pump(){
  key = get_input_key();
  if (key == 0) {
    return;
  }
  delay(50);
  // key = Cancel or OK 
  if (key == ir_keys[K_CANCEL].hex || key == ir_keys[K_OK].hex){
    lcd.clear_L1();
    lcd.clear_L2();
    lcd.clear_L3();
    global_mode = 0;
    delay (100);
    first = true;
  }else{
#ifdef DEBUG
    Serial.print("if (key[");
    Serial.print(key);
    Serial.print("] == ir_keys[K_CANCEL].hex[");
    Serial.print(ir_keys[K_CANCEL].hex);
    Serial.println("])");
#endif
  }

  delay(100);
    
  
}

/**************************/
/****** AUTO TOP-OFF ******/
/**************************/
void do_ATO(){

  ATO_FS1_STATE = digitalRead(ATO_FS1);
  ATO_FS2_STATE = digitalRead(ATO_FS2);
  ATO_FS3_STATE = digitalRead(ATO_FS3);

#ifdef DEBUG
  Serial << F("ATO_FS1_STATE: ") << ATO_FS1_STATE << "\r\n";
  Serial << F("ATO_FS2_STATE: ") << ATO_FS2_STATE << "\r\n";
  Serial << F("ATO_FS3_STATE: ") << ATO_FS3_STATE << "\r\n";
#endif

  if ( (backupTimer < backupMax) && (ATO_FS1_STATE == LOW) && (ATO_FS2_STATE == LOW) && (ATO_FS3_STATE == LOW)){ // LOW because we are pulling down the pins when switches activate
    //all is good. turn ATO on
//    digitalWrite(redLED,HIGH);
    digitalWrite(ATO_RELAY, HIGH);
    backupTimer++;
  }
  else if ((ATO_FS1_STATE == HIGH) && (ATO_FS2_STATE == LOW)){
    // water level is good. reset timer
    digitalWrite(ATO_RELAY, LOW);
//    digitalWrite(redLED,LOW);
    backupTimer = 0;
  }
  else if (ATO_FS3_STATE == HIGH){
    // backup float switch is on, something is wrong.
    digitalWrite(ATO_RELAY, LOW);
  }
  else if (backupTimer >= backupMax){
    // Pump has been running for too long, something is wrong.
    digitalWrite(ATO_RELAY, LOW);
//    digitalWrite(redLED,HIGH);
  }
  else{
    // RO/DI water level too low
    digitalWrite(ATO_RELAY, LOW);
//    digitalWrite(redLED,HIGH);
  }
}

/**************************/
/****** SET THE TIME ******/
/**************************/
void set_time( void ){
  key = get_input_key();
  if (key == 0) {
    return;
  }
  delay(50);
  // key = OK
  if (key == ir_keys[K_OK].hex ) {
    // Set the clock to run-mode, and disable the write protection
    rtc.halt(false);
    rtc.writeProtect(false);
    rtc.setDOW(tdw);        
    rtc.setTime(th, tmi, ts);     
    rtc.setDate(tdm, tmo, 2000+ty); 
    global_mode = 0;
    lcd.clear();
    first=true;
  }

  // key = Up
  else if (key == ir_keys[K_UP].hex){
    if (sPos == 1){
      if (th < 23) {
        th++;
      } 
      else {
        th = 0;  // wrap around
      }
    }
    else if (sPos == 2){
      if (tmi < 59) {
        tmi++;
      } 
      else {
        tmi = 0;  // wrap around
      }
    }
    else if (sPos == 3){
      if (ts < 59) {
        ts++;
      } 
      else {
        ts = 0;  // wrap around
      }
    }
    else if (sPos == 4){
      if (tdm < 31) {
        tdm++;
      } 
      else {
        tdm = 1;  // wrap around
      }
    }
    else if (sPos == 5){
      if (tmo < 12) {
        tmo++;
      } 
      else {
        tmo = 1;  // wrap around
      }
    }
    else if (sPos == 6){
      if (ty < 99) {
        ty++;
      } 
      else {
        ty = 0;  // wrap around
      }
    }
    else if (sPos == 7){
      if (tdw < 7) {
        tdw++;
      } 
      else {
        tdw = 1;  // wrap around
      }
    }
    delay (100);
    sprintf(strTime,"%02d:%02d:%02d %02d/%02d/%02d %d",th, tmi, ts, tdm, tmo, ty, tdw);
    update_temp(2,0);   
  }


  // key = Down
  else if (key == ir_keys[K_DOWN].hex){ 
    if (sPos == 1){
      if (th > 0) {
        th--;
      } 
      else {
        th = 23;  // wrap around
      }
    }
    else if (sPos == 2){
      if (tmi > 0) {
        tmi--;
      } 
      else {
        tmi = 59;  // wrap around
      }
    }
    else if (sPos == 3){
      if (ts > 0) {
        ts--;
      } 
      else {
        ts = 59;  // wrap around
      }
    }
    else if (sPos == 4){
      if (tdm > 1) {
        tdm--;
      } 
      else {
        tdm = 31;  // wrap around
      }
    }
    else if (sPos == 5){
      if (tmo > 1) {
        tmo--;
      } 
      else {
        tmo = 12;  // wrap around
      }
    }
    else if (sPos == 6){
      if (ty > 1) {
        ty--;
      } 
      else {
        ty = 99;  // wrap around
      }
    }
    else if (sPos == 7){
      if (tdw > 1) {
        tdw--;
      } 
      else {
        tdw = 7;  // wrap around
      }   
    }
    delay (100);
    sprintf(strTime,"%02d:%02d:%02d %02d/%02d/%02d %d",th, tmi, ts, tdm, tmo, ty, tdw);
    update_temp(2,0);   
  }


  // key = Left
  else if (key == ir_keys[K_LEFT].hex){
    if (sPos > 1) {
      sPos--;
    } 
    else {
      sPos = 7;  // wrap around
    }
    delay (100);
  }


  // key = Right
  else if (key == ir_keys[K_RIGHT].hex){
    if (sPos < 7) {
      sPos++;
    } 
    else {
      sPos = 1;  // wrap around
    }
    delay (100);
  }

  // key = Cancel
  else if (key == ir_keys[K_CANCEL].hex){
    lcd.clear();
    global_mode = 0;
    delay (100);
    first = true;
  }else{
#ifdef DEBUG
    Serial.print(key,HEX);
    Serial.print(":");
    Serial.print(ir_keys[K_UP].hex,HEX);
    Serial.print(" ");
    Serial.print(ir_keys[K_DOWN].hex,HEX);
    Serial.print(" ");
    Serial.print(ir_keys[K_LEFT].hex,HEX);
    Serial.print(" ");
    Serial.print(ir_keys[K_RIGHT].hex,HEX);
    Serial.print(" ");
    Serial.print(ir_keys[K_OK].hex,HEX);
    Serial.print(" ");
    Serial.print(ir_keys[K_CANCEL].hex,HEX);
    Serial.println();
#endif
  }
  delay(100);

}
/*********************************/
/****** NORMAL MODE HANDLER ******/
/*********************************/
void onKeyPress( void )
{

  key = get_input_key();
#ifdef DEBUG
  Serial.print(key,HEX);
#endif
  if (key == 0) {
    return;   // try again to sync up on an IR start-pulse
  }
  delay(50);
  // key = MENU
  if (key == ir_keys[K_MENU].hex) {
    global_mode = 1;
  }

  // key = UP
  else if (key == ir_keys[K_UP].hex) {
  }

  // key = DOWN
  else if (key == ir_keys[K_DOWN].hex) {
  }

  // key = LEFT
  else if (key == ir_keys[K_LEFT].hex) {
  }

  // key = RIGHT
  else if (key == ir_keys[K_RIGHT].hex) {
  }

  // key = OK
  else if (key == ir_keys[K_OK].hex) {
    //do something
  }

  // key = Cancel
  else if (key == ir_keys[K_CANCEL].hex) {
    //do something
  }

  else{
#ifdef DEBUG
    Serial.println("unsupported");
#endif
  }

  delay(100);
}

/***********************/
/****** MAIN MENU ******/
/***********************/
void show_menu( void ) {
  key = get_input_key();
  if (key == 0) {
    return;
  }
  delay(50);
  // key = OK
  if (key == ir_keys[K_OK].hex ) {
    menu.use();
    
  }
  else if (key == ir_keys[K_DOWN].hex){
    menu.moveUp();
    delay (100);
  }
  else if (key == ir_keys[K_UP].hex){ 
    menu.moveDown();
    delay (100);
  }
  else if (key == ir_keys[K_LEFT].hex){
    menu.moveLeft();
    delay (100);
  }
  else if (key == ir_keys[K_RIGHT].hex){ 
    menu.moveRight();
    delay (100);
  }
  else if (key == ir_keys[K_CANCEL].hex){
    if (menu.getCurrent().getLeft() == 0){
      lcd.clear_L1();
      lcd.clear_L2();
      lcd.clear_L3();
      global_mode = 0;
    }else{
      menu.moveLeft();
    }
    delay (100);
  } 

  delay(100);
}




/***********************************/
/****** HANDLE MENU SELECTION ******/
/***********************************/
void menuUseEvent(MenuUseEvent used)
{
  MenuItem * left = used.item.getLeft();
#ifdef DEBUG_MENU
    Serial.print(F("Current is"));
    Serial.print(menu.getCurrent().getLeft()->getName());
    Serial.print(F("->"));
    Serial.print(menu.getCurrent().getName());
    Serial.print(F(" used:"));
    Serial.print(used.item.getLeft()->getName());
    Serial.print(F("->"));
    Serial.println(used.item.getName());
#endif
    // clock setup
    if (used.item == mi_clock){
      global_mode=4;
      th = currentTime.hour;
      tmi = currentTime.min;
      ts = currentTime.sec;
      tdm = currentTime.date;
      tmo = currentTime.mon;
      ty = currentTime.year - 2000;
      tdw = currentTime.dow;
      lcd.clear();
      lcd.cursorTo(1,0);
      lcd.print(F("Use arrows to adjust"));
      sprintf(strTime,"%02d:%02d:%02d %02d/%02d/%02d %d",th, tmi, ts, tdm, tmo, ty, tdw);
      update_temp(2,0); 
      lcd.cursorTo(3,0);  
      lcd.print(F("HH:MM:SS DD/MM/YY DW"));
      set_time();
    }
    // pump1 setup
    else if(used.item == mi_pump1_set && *left == mi_pump1){
      pump_menu_set(p1);
    }
    // pump2 setup
    else if(used.item == mi_pump2_set && *left == mi_pump2){
      pump_menu_set(p2);
    }
    // pump3 setup
    else if(used.item == mi_pump3_set && *left == mi_pump3){
      pump_menu_set(p3);
    }
    // pump4 setup
    else if(used.item == mi_pump4_set && *left == mi_pump4){
      pump_menu_set(p4);
    }
    // pump5 setup
    else if(used.item == mi_pump5_set && *left == mi_pump5){
      pump_menu_set(p5);
    }
    // pump1 calibration
    else if(used.item == mi_pump1_calibrate && *left == mi_pump1){
      pump_menu_cal(p1);
    }
    // pump2 calibration
    else if(used.item == mi_pump2_calibrate && *left == mi_pump2){
      pump_menu_cal(p2);
    }
    // pump3 calibration
    else if(used.item == mi_pump3_calibrate && *left == mi_pump3){
      pump_menu_cal(p3);
    }
    // pump4 calibration
    else if(used.item == mi_pump4_calibrate && *left == mi_pump4){
      pump_menu_cal(p4);
    }
    // pump5 calibration
    else if(used.item == mi_pump5_calibrate && *left == mi_pump5){
      pump_menu_cal(p5);
    }
    // pump1 review
    else if(used.item == mi_pump1_review && *left == mi_pump1){
      prep_review_pump(p1);
    }
    // pump2 review
    else if(used.item == mi_pump2_review && *left == mi_pump2){
      prep_review_pump(p2);
    }
    // pump3 review
    else if(used.item == mi_pump3_review && *left == mi_pump3){
      prep_review_pump(p2);
    }
    // pump4 review
    else if(used.item == mi_pump4_review && *left == mi_pump4){
      prep_review_pump(p3);
    }
    // pump5 review
    else if(used.item == mi_pump5_review && *left == mi_pump5){
      prep_review_pump(p4);
    }
    // Auto TopOff setup
    else if(used.item == mi_ATO_set){
    }
    //Menu categories.
    else if(used.item == settings || used.item == mi_pump1 || used.item == mi_pump2 || used.item == mi_pump3 || used.item == mi_pump4 || used.item == mi_pump5){
      menu.moveRight();
    }else {
//      lcd.cursorTo(2,0);
//      lcd.print("Used ");
//      lcd.printL((char*)used.item.getName(), 15);
    }
}

/*
	This is an important function
	Here we get a notification whenever the user changes the menu
	That is, when the menu is navigated
*/

/*****************************/
/****** MENU NAVIGATION ******/
/*****************************/
void menuChangeEvent(MenuChangeEvent changed)
{
#ifdef DEBUG_MENU
  Serial.print(F("Menu change "));
  Serial.print(changed.from.getName());
  Serial.print("->");
  Serial.print(changed.to.getName());
  Serial.print("->");
  Serial.println(changed.to.getLeft()->getName());
  Serial.println(menu.getCurrent().getName());
#endif

  if (global_mode == 1){
    if (changed.to.getLeft() == 0){
      if (changed.to.getRight() != 0){
        lcd.cursorTo(1,0);
        lcd.print(F("   >                "));
      }else{
        lcd.cursorTo(1,3);
        lcd.print(F(" "));
      }
      lcd.cursorTo(0,0);
      lcd.printL((char*)changed.to.getName(), 20);
    }else{
      lcd.cursorTo(1,3);
      lcd.print(F("> "));
      lcd.printL((char*)changed.to.getName(), 15);
    }
  }
}



/***************************/
/****** INIT THE MENU ******/
/***************************/
void menuSetup()
{
  //add the settings menu to the menu root
  menu.getRoot().add(settings);
  //setup the settings menu item
  settings.addBefore(mi_pump1);
  settings.addAfter(mi_ATO);
  settings.addRight(mi_clock);
      mi_clock.addBefore(mi_sleep);
      mi_clock.addAfter(mi_sleep);
      mi_clock.addLeft(settings);
      mi_sleep.addAfter(mi_clock);
      mi_sleep.addLeft(settings);
  mi_pump1.addBefore(mi_pump2);
  mi_pump1.addAfter(settings);
  mi_pump1.addRight(mi_pump1_review);
    mi_pump1_review.addBefore(mi_pump1_set);
    mi_pump1_review.addAfter(mi_pump1_calibrate);
    mi_pump1_review.addLeft(mi_pump1);
    mi_pump1_set.addBefore(mi_pump1_calibrate);
    mi_pump1_set.addAfter(mi_pump1_review);
    mi_pump1_set.addLeft(mi_pump1);
    mi_pump1_calibrate.addAfter(mi_pump1_set);
    mi_pump1_calibrate.addBefore(mi_pump1_review);
    mi_pump1_calibrate.addLeft(mi_pump1);
  mi_pump2.addBefore(mi_pump3);
  mi_pump2.addAfter(mi_pump1);
  mi_pump2.addRight(mi_pump2_review);
    mi_pump2_review.addBefore(mi_pump2_set);
    mi_pump2_review.addAfter(mi_pump2_calibrate);
    mi_pump2_review.addLeft(mi_pump2);
    mi_pump2_set.addBefore(mi_pump2_calibrate);
    mi_pump2_set.addAfter(mi_pump2_review);
    mi_pump2_set.addLeft(mi_pump2);
    mi_pump2_calibrate.addAfter(mi_pump2_set);
    mi_pump2_calibrate.addBefore(mi_pump2_review);
    mi_pump2_calibrate.addLeft(mi_pump2);
  mi_pump3.addBefore(mi_pump4);
  mi_pump3.addAfter(mi_pump2);
  mi_pump3.addRight(mi_pump3_review);
    mi_pump3_review.addBefore(mi_pump3_set);
    mi_pump3_review.addAfter(mi_pump3_calibrate);
    mi_pump2_review.addLeft(mi_pump2);
    mi_pump3_set.addBefore(mi_pump3_calibrate);
    mi_pump3_set.addAfter(mi_pump3_review);
    mi_pump3_set.addLeft(mi_pump3);
    mi_pump3_calibrate.addAfter(mi_pump3_set);
    mi_pump3_calibrate.addBefore(mi_pump3_review);
    mi_pump3_calibrate.addLeft(mi_pump3);
  mi_pump4.addBefore(mi_pump5);
  mi_pump4.addAfter(mi_pump3);
  mi_pump4.addRight(mi_pump4_review);
    mi_pump4_review.addBefore(mi_pump4_set);
    mi_pump4_review.addAfter(mi_pump4_calibrate);
    mi_pump4_review.addLeft(mi_pump4);
    mi_pump4_set.addBefore(mi_pump4_calibrate);
    mi_pump4_set.addAfter(mi_pump4_review);
    mi_pump4_set.addLeft(mi_pump4);
    mi_pump4_calibrate.addAfter(mi_pump4_set);
    mi_pump4_calibrate.addBefore(mi_pump4_review);
    mi_pump4_calibrate.addLeft(mi_pump4);
  mi_pump5.addBefore(mi_ATO);
  mi_pump5.addAfter(mi_pump4);
  mi_pump5.addRight(mi_pump5_review);
    mi_pump5_review.addBefore(mi_pump5_set);
    mi_pump5_review.addAfter(mi_pump5_calibrate);
    mi_pump5_review.addLeft(mi_pump5);
    mi_pump5_set.addBefore(mi_pump5_calibrate);
    mi_pump5_set.addAfter(mi_pump5_review);
    mi_pump5_set.addLeft(mi_pump5);
    mi_pump5_calibrate.addAfter(mi_pump5_set);
    mi_pump5_calibrate.addBefore(mi_pump5_review);
    mi_pump5_calibrate.addLeft(mi_pump5);
  mi_ATO.addBefore(settings);
  mi_ATO.addAfter(mi_pump5);
  mi_ATO.addRight(mi_ATO_set);
    mi_ATO_set.addLeft(mi_ATO);
    
  menu.moveDown();
}

int availableMemory() 
{
  int size = 1024;
  byte *buf;
  while ((buf = (byte *) malloc(--size)) == NULL);
  free(buf);
  return size;
}
