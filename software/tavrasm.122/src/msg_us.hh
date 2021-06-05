//////////////////////////////////////////////////////////////////////////////
//
//  File        : msg_us.hh
//
//  Author      : Tom Mortensen - Copyright (C) 1999
//
//  Description : US assembler messages
//
//  History
//  ========================================================================
//
//  980127      : Tom - File created - Messages moved from 'message.hh'.
//  991222      : Kurt- New command line option -x
//  040306      : Dariusz Kowalewski <darekk@automex.pl> added messages for
//                      W_DEF_REDEF, E_DEV_REDEF and E_DEV_AFTER_DSEG_DATA
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

#ifndef _MSG_US_HH_
#define _MSG_US_HH_

// Command line info /////////////////////////////////////////////////////////

#define HELPSTR                                                 \
"usage: tavrasm [-vwclxmihjgbafd] <infile> \n"                  \
"               [-I <includepath>]... [-o <outfile>] [-r <rom>] [-e <listfile>]\n" \
"       -v verbose\n"                                           \
"       -w no warnings\n"                                       \
"       -c case sensitive labels/defines\n"                     \
"       -l limit log width to 80 characters\n"                  \
"       -x allow local labels\n"                                \
"       -m output Motorola S-record format\n"                   \
"       -i output Intel HEX format (default)\n"                 \
"       -h omit address extension record from Intel HEX files\n"\
"       -j output .obj format\n"                                \
"       -g output generic hex\n"                                \
"       -b output binary format\n"                              \
"       -a wrap relative jumps\n"                               \
"       -f allow forward org's\n"                               \
"       -d list supported devices\n"                           

// Message strings ///////////////////////////////////////////////////////////

#ifdef _UTILS_CC_

char messages[MESSAGE_COUNT][MESSAGES_LEN]=
{
  "No warning",
  "Invalid character \'%c\' in source",
  "Invalid character 0x%02X in source",
  "Argument @%i specified, but not used in macro",
  "Argument @%i not specified, but used in macro",
  "\'%s\' already defined as label at line %i in \'%s\'",
  "\'%s\' already defined at line %i in \'%s\'",
  "Integer result of EXP2(%i) is set to zero",
  "Integer result of LOG2(%i) is set to zero",
  "Bit number '\%i\' out of range (0-7)",
  "I/O Port 0x%X out of range (0x00-0x1F)",
  "I/O Port 0x%X out of range (0x00-0x3F)",
  "Bit mask 0x%X out of range (0x00-0xFF)",
  "Constant 0x%X out of range (0x00-0x3F)",
  "Constant 0x%X out of range (0x00-0xFF)",
  "Constant 0x%X out of range (0x00-0xFFFF)",
  "Address 0x%X out of range (0x0-0xFFFF)",
  "Displacement \'%X\' is out of range (0x00 - 0x3F)",
  "Destination address \'0x%X\' out of range (0x0-0x3FFFFF)",
  "Value %i out of range in .db (-128 to 255)",
  "Value %i out of range in .dw (-32768 to 65535)",
  "Value %i moves PC back over code (by %i words)",
  "Value %i overwrites EEPROM data (by %i byte(s))",
  "Value %i overwrites data (by %i byte(s))",
  "Value %i overwrites Register or I/O area (by %i byte(s))",
  "SRAM size exceeded by %i byte(s)",
  "Unused parameter @%i in macro definition",
  "The directive '.listmac' is not supported (ignored)",
  "The directive '.list' is not supported (ignored)",
  "The directive '.nolist' is not supported (ignored)",
  "\'%s\' not supported on this device",
  "Unknown device \'%s\'",
  "Constant too big",
  "Identifier too long",
  "EEPROM size exceeded by %i byte(s)",
  "Illegal octal escape value",
  "Illegal hex escape value",
  "Octal escape value out of range",
  "Illegal character constant",
  "Flash size exceeded by %i word(s)",
  "Macro name too long",
  "Consecutive '.device' directive (\'%s\', ignored)",

  /// Error messages /////////////////////////////////////////////////////////

  "Undefined variable referenced",
  "Undefined variable referenced in bit number",
  "Undefined variable referenced in I/O port address",
  "Undefined variable referenced in branch",
  "Undefined variable referenced in bit mask",
  "Undefined variable referenced in constant",
  "Undefined variable referenced in address",
  "Undefined variable referenced in displacement",
  "Undefined variable referenced in relative call",
  "Undefined variable referenced in relative jump",
  "Undefined variable referenced in jump/call",
  "Undefined variable referenced in constant in .db",
  "Undefined variable referenced in constant in .dw",
  "Undefined variable referenced in offset",
  "Undefined variable referenced in constant in .byte",
  "Division by zero",
  "Invalid register (r16-r31)",
  "Invalid register %s",
  "Branch out of range by %i words (backwards)",
  "Branch out of range by %i words (forward)",
  "Invalid register (r24/r26/r28/r30)",
  "Invalid source register r%i (X/Y/Z)",
  "Invalid destination register r%i (X/Y/Z)",
  "Call out of range by %i words (backwards)",
  "Call out of range by %i words (forward)",
  "Jump out of range by %i words (backwards)",
  "Jump out of range by %i words (forward)",
  "Label \'%s\' already defined at line %i in \'%s\'",
  "Label \'%s\' already defined at line %i in \'%s\'",
  "Variable \'%s\' already defined as label at line %i in \'%s\'",
  ".dw in DSEG (only allowed in CSEG and ESEG)",
  ".db in DSEG (only allowed in CSEG and ESEG)",
  "Macro \'%s\' already defined (or duplicates label)",
  "Program code in DSEG (only allowed in CSEG)",
  "Program code in ESEG (only allowed in CSEG)",
  "Unable to include file \'%s\' - include depth exceeded",
  "Invalid include filename specified",
  "Unable to open file \'%s\'",
  "Unable to expand macro - context depth exceeded",
  "End of file reached in macro definition",
  "Filename too long at line %i in \'%s\'",
  "Invalid filename",
  "Register expected",
  "Invalid register specified",
  "Invalid register or immediate",
  "Invalid immediate constant",
  "No argument expected",
  "Unknown opcode",
  "Expected ID = register",
  "Expected value (,value ,...) ",
  "No arguments should be specified",
  "Expected value/label",
  "Expected ID = expression",
  "Expected device name",
  "Variable declaration in CSEG",
  "Macro with no name",
  "Undefined escape sequence %s",
  "No filename specified",
  "Macro used before it is defined",
  "Undefined escape sequence \'\\%c\'",
  "Unexpected .endmacro directive",
  "Macro definition within macro not allowed",
  "Expected register, displacement",
  "Expected displacement, register",
  "No displacement can be specified for X (use Y/Z)",
  "Invalid register r%d (expected r23-r31)",
  "Displacement can not be specified here",
  "Invalid register specified, expected 'Z'",
  "Pre-increment is invalid here (use 'Z' or 'Z+')",
  "Invalid register r%d (expected r%d-r%d)",
  "Invalid register r%d (expected r0, r2, ...)",
  "Undefined variable referenced in constant in .align",
  ".align value not a power of two",
  "Unexpected '.device' directive after DSEG data (\'%s\')",
  "Multiple '.device' directives (\'%s\' and \'%s\')",

  /// Fatal errors ///////////////////////////////////////////////////////////

  "No fatal error",
  "Out of memory",
  "Line too long in \'%s\'",
  "Macro line too long at line %i in \'%s\'",
  "Unable to open input file \'%s\'",
  "Unable to open list file \'%s\'",
  "Unable to open output file \'%s\'",
  "Unable to open rom file \'%s\'",
  "Too many files included",
  "Code size exceeded",
  "EEPROM size exceeded",
  "Max macro depth exceeded",

  /// Messages ///////////////////////////////////////////////////////////////

  "No message",
  "Empty rom file",
  "Can not save rom file with .obj format",
  "No input filename specified",
  "Unknown option '%s'",
  "More than one of m/i/j/b/h specified",
  "No rom filename specified",
  "Rom filename too long",
  "Illegal rom filename",
  "No list filename specified",
  "List filename too long",
  "Illegal list filename",
  "No output filename specified",
  "Output filename too long",
  "Illegal output filename",
  "Multiple output files specified",
  "Input filename too long",
  "Illegal input filename",
  "Multiple input files specified",
  "Errors      : %i",
  "Warnings    : %i",
  "Code        : %i",
  "Rom         : %i",
  "Data        : %i",
  "Unused Code : %i",
  "Unused Rom  : %i",
  "No code generated",
  "| Device name      | Ram start | Flash Size | Ram size | EEPROM Size |",
  "No include path found after '-I' flag",
  "Too many include paths specified",
  "Include pathname too long"
};

#endif

#endif /* _MESSAGES_HH_ */

/// END OF FILE //////////////////////////////////////////////////////////////
