//////////////////////////////////////////////////////////////////////////////
//
//  File        : semantic.hh
//
//  Author      : Tom Mortensen - Copyright (C) 1999
//
//  Description : Code generation header
//
//  History
//  ========================================================================
//
//  980902      : Tom - File created.
//  990124      : Tom - Added GPL notice.
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

#ifndef _SEMANTIC_HH_
#define _SEMANTIC_HH_

/// Include //////////////////////////////////////////////////////////////////

#include "opcodes.hh"
#include "avrasm.hh"
#include "symbol.hh"
#include "messages.hh"

/// Defines //////////////////////////////////////////////////////////////////

#define OP_AND          1
#define OP_STAR         2
#define OP_PLUS         3
#define OP_MINUS        4
#define OP_WAVE         5
#define OP_NOT          6

#define OP_DIV          7
#define OP_MOD          8
#define OP_LS           9
#define OP_RS          10
#define OP_LESS        11
#define OP_GREAT       12
#define OP_LE          13
#define OP_GE          14
#define OP_EQ          15
#define OP_NE          16
#define OP_XOR         17
#define OP_OR          18
#define OP_ANDAND      19
#define OP_OROR        20

#define OP_LOW         21
#define OP_HIGH        22
#define OP_BYTE2       23
#define OP_BYTE3       24
#define OP_BYTE4       25
#define OP_LWRD        26
#define OP_HWRD        27
#define OP_PAGE        28
#define OP_EXP2        29
#define OP_LOG2        30

/// Types ////////////////////////////////////////////////////////////////////

  /// Attribute structs (also typedef'ed below) ////////////////////////////

struct  instructionstruct
        {
          int           value;
          int           size;
          int           opcode;
        };

struct  valuestruct
        {
          int           value;
          int           valid;
        };

struct  indirectstruct
        {
          int           regno;
          int           plus;
          int           disp;
          valuestruct   offset;
        };

  /// Synthesized attributes ///////////////////////////////////////////////

typedef int                     regSA;
typedef instructionstruct       instSA;
typedef int                     opcodeSA;
typedef valuestruct             valueSA;
typedef char                    nameSA[MAX_ID_LENGTH+1];
typedef symbolTableEntry*       symbolSA;
typedef indirectstruct          indirectSA;
typedef int                     functionSA;
typedef char*                   stringSA;

/// Prototypes ///////////////////////////////////////////////////////////////

  /// Generate functions ///////////////////////////////////////////////////

void  genRegReg(int opcode, regSA dest, regSA src);
void  genRegImm(int opcode, regSA reg, valueSA *value);
void  genImmReg(int opcode, valueSA *value, regSA reg);
void  genImmImm(int opcode, valueSA *valued, valueSA *values);
void  genImmedi(int opcode, valueSA *value);
void  genNoargs(int opcode);
void  genRegist(int opcode, regSA reg);
void  genIndirc(int opcode, indirectSA *indi, regSA reg, int right = FALSE);
void  genLpm(int opcode, regSA reg, indirectSA *indi, int useregs);

  /// Handle directives ////////////////////////////////////////////////////

void  doDef(symbolSA symb, regSA reg);          // Handle '.def'
void  doDef(symbolSA symbl, symbolSA symbr);    // Handle '.def'
void  doEqu(symbolSA symb, valueSA *value);     // Handle '.eq'
void  doAdb(void);                              // Init '.db'
void  doAdb(valueSA *value);                    // Handle '.db'
void  doAdb(stringSA str);                      // Handle '.db'
void  doAdw(void);                              // Init '.dw'
void  doAdw(valueSA *value);                    // Handle '.dw'
void  doOrg(valueSA *value);                    // Handle '.org'
void  doAlign(valueSA *value);                  // Handle '.align'
void  doByt(valueSA *value);                    // Handle '.byte'
void  doSet(symbolSA symb, valueSA *value);     // Handle '.set'
void  doDev(symbolSA devicename);               // Handle '.device'

  /// Add label ////////////////////////////////////////////////////////////

void  doLab(symbolSA symb);                     // Add a label

  /// Operators ////////////////////////////////////////////////////////////

void  oprUna(int opr, valueSA *value, valueSA *res);
void  oprBin(valueSA *vl, int opr, valueSA *vr, valueSA *res);
void  genFun(functionSA func, valueSA *value, valueSA *res);

#endif /* _SEMANTIC_HH_ */

/// END OF FILE //////////////////////////////////////////////////////////////

