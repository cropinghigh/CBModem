//////////////////////////////////////////////////////////////////////////////
//
//  File        : messages.hh
//
//  Author      : Tom Mortensen - Copyright (C) 1999
//
//  Description : AVR Macro assembler messages
//
//  History
//  ========================================================================
//
//  980902      : Tom - File created.
//  990124      : Tom - Added GPL notice.
//  990127      : Tom - Messages moved to 'msg_us.hh', multi language added
//  000111	: Brian Rhodefer - added message for too many include paths
//  040306      : Dariusz Kowalewski <darekk@automex.pl> added W_DEF_REDEF,
//                      E_DEV_REDEF and E_DEV_AFTER_DSEG_DATA
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

#ifndef _MESSAGES_HH_
#define _MESSAGES_HH_

/// Defines //////////////////////////////////////////////////////////////////

#define MESSAGES_LEN                   77

/// Warnings /////////////////////////////////////////////////////////////////

#define W_INVALID_CHAR                  1
#define W_INVALID_ASCII                 2
#define W_ARG_SPEC                      3
#define W_ARG_USED                      4
#define W_REGLABEL                      5
#define W_REGREDEF                      6
#define W_INTEGERRESULT_EXP2_IS_ZERO    7
#define W_INTEGERRESULT_LOG2_IS_ZERO    8
#define W_BIT_OUT_OF_RANGE              9
#define W_PORT_OUT_OF_RANGE_1F         10
#define W_PORT_OUT_OF_RANGE_3F         11
#define W_BITMASK_OUT_OF_RANGE         12
#define W_CONST_OUT_OF_RANGE_3F        13
#define W_CONST_OUT_OF_RANGE_FF        14
#define W_CONST_OUT_OF_RANGE_FFFF      15
#define W_ADDR_OUT_OF_RANGE_FFFF       16
#define W_DISPLACEMENT_OUT_OF_RANGE    17
#define W_ILLEGAL_DEST_ADDR            18
#define W_DB_OUT_OF_RANGE              19
#define W_DW_OUT_OF_RANGE              20
#define W_ORG_OUT_OF_RANGE             21
#define W_ORG_OUT_OF_RANGE_ESEG        22
#define W_ORG_OUT_OF_RANGE_DSEG        23
#define W_ORG_OVER_REG                 24
#define W_SRAM_EXCEED                  25
#define W_MACRO_UNUSES_PARM            26
#define W_LISTMACRO_NOT_SUP            27
#define W_LIST_NOT_SUP                 28
#define W_NOLIST_NOT_SUP               29
#define W_UNSUP                        30
#define W_UNKNOWN_DEV                  31
#define W_CONSTANT_TO_BIG              32
#define W_IDENTIFIER_TOO_LONG          33
#define W_EROM_EXCEED                  34
#define W_OCT_ESCAPE_INVALID           35
#define W_HEX_ESCAPE_INVALID           36
#define W_OCT_OUT_OF_RANGE             37
#define W_CHAR_INVALID                 38
#define W_FLASH_EXCEEDED               39
#define W_MAC_IDENTIFIER_TOO_LONG      40
#define W_DEV_REDEF                    41
#define MESSAGE_WARNING_LAST           41

/// Errors ///////////////////////////////////////////////////////////////////

#define E_UNKNOWN                       1
#define E_UNKNOWN_BIT                   2
#define E_UNKNOWN_PORT                  3
#define E_UNKNOWN_BRANCH                4
#define E_UNKNOWN_BITMASK               5
#define E_UNKNOWN_CONSTANT              6
#define E_UNKNOWN_ADDRESS               7
#define E_UNKNOWN_DISP                  8
#define E_UNKNOWN_RCALL_DEST            9
#define E_UNKNOWN_RJUMP_DEST           10
#define E_UNKNOWN_JUMP_CALL_DEST       11
#define E_UNKNOWN_DB                   12
#define E_UNKNOWN_DW                   13
#define E_UNKNOWN_ORG                  14
#define E_UNKNOWN_BYTE                 15
#define E_DIVISION_BY_ZERO             16
#define E_INVALID_REGISTER_R16_R31     17
#define E_INVALID_REGISTER             18
#define E_BRANCH_OUT_OF_RANGE_B        19
#define E_BRANCH_OUT_OF_RANGE_F        20
#define E_INVALID_REG_R24_R30          21
#define E_INVALID_SOURCE_REG_XYZ       22
#define E_INVALID_DEST_REG_XYZ         23
#define E_RCALL_OUT_OF_RANGE_B         24
#define E_RCALL_OUT_OF_RANGE_F         25
#define E_RJUMP_OUT_OF_RANGE_B         26
#define E_RJUMP_OUT_OF_RANGE_F         27
#define E_EQU_REDEF                    28
#define E_LABEL_REDEF                  29
#define E_LABEL_VAR                    30
#define E_DW_IN_DSEG                   31
#define E_DB_IN_DSEG                   32
#define E_MACRO_REDEF                  33
#define E_PROGRAM_IN_DSEG              34
#define E_PROGRAM_IN_ESEG              35
#define E_INCLUDE_DEPTH                36
#define E_INVALID_INCLUDE_NAME         37
#define E_OPEN_FILE                    38
#define E_MACRO_DEPTH                  39
#define E_EOF_IN_MACRO                 40
#define E_FILENAME_TOO_LONG            41
#define E_INVALID_FILENAME             42
#define E_REGISTER_EXPECTED            43
#define E_INVALID_REGISTER_SPEC        44
#define E_INVALID_REGIMM_SPEC          45
#define E_INVALID_IMMEDIATE_SPEC       46
#define E_NOARGS_EXPECTED_SPEC         47
#define E_UNKNOWN_OPCODE               48
#define E_EXPECTED_ID_REG              49
#define E_EXPECTED_VALLIST             50
#define E_EXPECTED_NOARGS              51
#define E_EXPECTED_VAL_LABEL           52
#define E_EXPECTED_ID_EXPR             53
#define E_EXPECTED_DEVICE              54
#define E_VAR_IN_CSEG                  55
#define E_NO_MACRO_NAME                56
#define E_UNDEF_ESCAPE                 57
#define E_NO_FILENAME                  58
#define E_DEF_USE                      59
#define E_UNDEF_ESCAPE_C               60
#define E_ENDM                         61
#define E_MACRO_IN_MACRO               62
#define E_REG_DISP                     63
#define E_DISP_REG                     64

#define E_NO_DISP                      65
#define E_INVALID_REGISTER_R23_R31     66
#define E_DISP_ILLEGAL                 67
#define E_INVALID_REG_Z_EXPECTED       68
#define E_INVALID_PREINCREMENT         69
#define E_INVALID_REG_EXPECTED         70
#define E_INVALID_REG_EXPECTED_0_2_4   71
#define E_UNKNOWN_ALIGN                72
#define E_ALIGN_NOT_POWER_OF_TWO       73
#define E_DEV_AFTER_DSEG_DATA          74
#define E_DEV_REDEF                    75
#define MESSAGE_ERROR_LAST             75

#define X_OUT_OF_MEMORY                 1
#define X_LINE_TOO_LONG                 2
#define X_MACRO_LINE_TOO_LONG           3
#define X_UNABLE_TO_OPEN_FILE           4
#define X_UNABLE_TO_OPEN_LOGFILE        5
#define X_UNABLE_TO_OPEN_OUTFILE        6
#define X_UNABLE_TO_OPEN_ROMFILE        7
#define X_TOO_MANY_FILES                8
#define X_TOO_MUCH_CODE                 9
#define X_TOO_MUCH_DATA                10
#define X_MACRO_DEPTH                  11
#define MESSAGE_FATAL_LAST             11

#define M_EMPTY_ROM_FILE                1
#define M_NOT_SAVE_EROM_OBJ             2
#define M_NO_INPUT_FILENAME             3
#define M_UNKNOWN_OPTION                4
#define M_MORE_MIJBH                    5
#define M_NO_ROM_FILENAME               6
#define M_ROM_FILENAME_TOO_LONG         7
#define M_ILLEGAL_ROM_FILENAME          8
#define M_NO_LIST_FILENAME              9
#define M_LIST_FILENAME_TOO_LONG       10
#define M_ILLEGAL_LIST_FILENAME        11
#define M_NO_OUTPUT_FILENAME           12
#define M_OUTPUT_FILENAME_TOO_LONG     13
#define M_ILLEGAL_OUTPUT_FILENAME      14
#define M_MULTIPLE_OUTPUT              15
#define M_INPUT_FILENAME_TOO_LONG      16
#define M_ILLEGAL_INPUT_FILENAME       17
#define M_MULTIPLE_INPUT               18
#define M_ERRORS                       19
#define M_WARNINGS                     20
#define M_CODE                         21
#define M_ROM                          22
#define M_DATA                         23
#define M_UNUSED_CODE                  24
#define M_UNUSED_ROM                   25
#define M_EMPTY_CODE                   26
#define M_DEVICE_HEADER                27
#define M_NO_INCLUDEPATH	       28
#define M_TOO_MANY_INCPATHS	       29
#define M_INCPATH_TOO_LONG	       30
#define MESSAGE_MESSAGES_LAST          30

/// Message count ////////////////////////////////////////////////////////////

#define MESSAGES_WARNING               MESSAGE_WARNING_LAST
#define MESSAGES_ERROR                 MESSAGE_ERROR_LAST
#define MESSAGES_FATAL                 MESSAGE_FATAL_LAST
#define MESSAGES_MESSAGES              MESSAGE_MESSAGES_LAST

#define MESSAGE_OFFSET_WARNING         0
#define MESSAGE_OFFSET_ERROR           MESSAGES_WARNING
#define MESSAGE_OFFSET_FATAL           (MESSAGES_WARNING + MESSAGES_ERROR + 1)
#define MESSAGE_OFFSET_MESSAGES        (MESSAGE_OFFSET_FATAL+MESSAGES_FATAL+1)


#define MESSAGE_COUNT                  (4+MESSAGES_WARNING+MESSAGES_ERROR+\
					  MESSAGES_FATAL+MESSAGES_MESSAGES)

/// Languages ////////////////////////////////////////////////////////////////

#define LANGUAGE_US			1  // US English
#define LANGUAGE_DE			2  // German
#define LANGUAGE_SP			3  // Spanish

/// Message strings //////////////////////////////////////////////////////////

#ifdef AVRLANG
#if   AVRLANG == LANGUAGE_DE
#include "msg_de.hh"
#elif AVRLANG == LANGUAGE_US
#include "msg_us.hh"
#elif AVRLANG == LANGUAGE_SP
#include "msg_sp.hh"
#else
#error "Invalid language selected"
#endif /* AVRLANG */
#else
#error "No language selected"
#endif

#endif /* _MESSAGES_HH_ */

/// END OF FILE //////////////////////////////////////////////////////////////

