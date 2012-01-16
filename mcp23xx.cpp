/*
 *  mcp23xx.cpp: part of the "VoluMaster(tm)" system
 *
 *  An open-source arduino controller-based digital volume control and input/output selector
 *
 *
 *  LICENSE
 *  -------
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mcp23xx.h"

#include <Wire.h>
#include <stdio.h>       //not needed yet
#include <string.h>      //needed for strlen()
#include <inttypes.h>
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WConstants.h"
#endif


// ctor
MCP23XX::MCP23XX( uint8_t passed_i2c_addr )
{
  my_dev_addr = passed_i2c_addr;
}

void
MCP23XX::init( void )
{
  set(MCP_REG_IOCON, 0x0C);  // CONFREG (0x05)
  delay(5);

  set(MCP_REG_IODIR, 0x00);  // IOREG (0x00 = 'all are output ports'
  delay(5);
}





void
MCP23XX::set( uint8_t reg, uint8_t val )
{
  
  Wire.beginTransmission(my_dev_addr);
#if defined(ARDUINO) && ARDUINO >= 100
  Wire.write(reg);
  Wire.write(val);
#else
  Wire.send(reg);
  Wire.send(val);
#endif

  Wire.endTransmission();
}




uint8_t
MCP23XX::get( uint8_t reg ) 
{
  uint8_t val = 0;


  Wire.beginTransmission(my_dev_addr);
#if defined(ARDUINO) && ARDUINO >= 100
  Wire.write(reg);
#else
  Wire.send(reg);
#endif  

  Wire.endTransmission();


  val = Wire.requestFrom((uint8_t)my_dev_addr, (uint8_t)1);  // read 1 byte
  if (Wire.available()) {
#if defined(ARDUINO) && ARDUINO >= 100
    val = Wire.read();
#else
    val = Wire.receive();
#endif
    
  }

  return val;
}



