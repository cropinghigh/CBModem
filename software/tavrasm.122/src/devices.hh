//////////////////////////////////////////////////////////////////////////////
//
//  File        : devices.hh
//
//  Author      : Tom Mortensen - Copyright (C) 1999
//
//  Description : AVR Controller descriptions
//
//  History
//  ========================================================================
//
//  980902      : Tom - File created.
//  990124      : Tom - Added GPL notice.
//  990721	: Tom - Added AT90S4433
//  021218	: Andreas Schwarz <andreas-s@web.de> added AT90S2333, ATmegaXX
//  040306      : Dariusz Kowalewski <darekk@automex.pl> fixes/updates for ATmega
//
//////////////////////////////////////////////////////// Tom did this ////////
//
// Copyright notice:
//
// tavrasm - A GNU/Linux assembler for the Atmel AVR series
// of microcontrollers. Copyright (C) 1999 Tom Mortensen
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// 
// Tom Mortensen
//
// E-mail : tom@tavrasm.org
// WWW    : http://www.tavrasm.org
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _DEVICES_HH_
#define _DEVICES_HH_

/// Include //////////////////////////////////////////////////////////////////

#include "avrasm.hh"

/// Device descriptions //////////////////////////////////////////////////////
//
// struct deviceinfo {
//         char    name[MAX_DEVICE_NAME_LENGTH+1]; // Device name
//         int     datastart;                      // Registers + I/O ports
//         int     ramsize;                        // Size of ram
//         int     eepromsize;                     // Size of EEPROM
//         int     flashsize;                      // Size of flash (in words)
//         int     supported;                      // Unsupported instructions
// };
// 

deviceinfo devices[MAX_DEVICES] = {

  // -----------------------------------------------------------------------
  // Name       |Reg| Ram  | EEPROM |Flash Size | Unsupported instuctions  |
  // -----------------------------------------------------------------------
  //            |   |      |        |           |                          |
  { "GENERIC"   , 96, 65536, 0x10000, 0x1000000 , 0                       },
  { "ATMega48"  ,256,   512,     256,      2048 , 0                       },
  { "ATMega328P" ,256,  2048,     1024,      32768 , 0                       },
  //            |   |      |        |           |                          |
  // -----------------------------------------------------------------------

  // Test device

  { ""          , 96, 2    , 5      ,        10 , 0                       },

  // NULL - terminating device.

  { ""          , 0 ,   0, 0x000, 0x1000000 , 0                           }

};

#endif /* _DEVICES_HH_ */

/// END OF FILE //////////////////////////////////////////////////////////////
