/*
 *  mcp23xx.h: part of the "VoluMaster(tm)" system
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

#ifndef MCP23XX_H
#define MCP23XX_H

#include <inttypes.h>


// register codes
#define MCP_REG_CONF   0x05
#define MCP_REG_IO     0x00
#define MCP_REG_ODATA  0x0a


// general i2c register codes
#define MCP_REG_IODIR     0x00
#define MCP_REG_IPOL      0x01
#define MCP_REG_GPINTEN   0x02
#define MCP_REG_DEFVAL    0x03
#define MCP_REG_INTCON    0x04
#define MCP_REG_IOCON     0x05
#define MCP_REG_GPPU      0x06
#define MCP_REG_INTF      0x07
#define MCP_REG_INTCAP    0x08
#define MCP_REG_GPIO      0x09
#define MCP_REG_OLAT      0x0A




class MCP23XX {

 public:

  MCP23XX( uint8_t devI2CAddress );
  MCP23XX();
  void init( void );
  void set( uint8_t reg, uint8_t val );
  uint8_t get( uint8_t reg );

 private:
  uint8_t my_dev_addr;

};

#endif  // MCP23XX_H


