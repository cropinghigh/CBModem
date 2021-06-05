//////////////////////////////////////////////////////////////////////////////
//
//  File        : symbol.hh
//
//  Author      : Tom Mortensen - Copyright (C) 1999
//
//  Description : Symbol table header
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

#ifndef _SYMBOL_HH_
#define _SYMBOL_HH_

/// Include //////////////////////////////////////////////////////////////////

#include "avrasm.hh"

/// Defines //////////////////////////////////////////////////////////////////

#define HASH_DEFAULT_SIZE       1024  // Could be any 2^N
#define HASH_MAX_SIZE        0x10000  // Could be any 2^N

/// Type/Class definitions ///////////////////////////////////////////////////

class symbolTable;

  ////////////////////////////////////////////////////////////////////////////
  //
  // Symbol-table entry
  //

struct symbolTableEntry
{
  // Identifier

  char                   name[MAX_ID_LENGTH+1];

  // House keeping

  int                    scope;
  symbolTableEntry       *prev;
  symbolTableEntry       *next;
  symbolTableEntry       *up;
  symbolTableEntry       *down;

  // Symbol defines a macro

  int                    macro;
  char                   macfilename[MAX_FILENAME_LENGTH+1];
  int                    macdefline;
  int                    macsize;
  char                   *macstr;
  int                    macparms;
  char                   macparmlist[10];
  symbolTable            *macsym;
  int                    macrodone;

  // Symbol defines a variable

  int                    isvar;

  // Symbol defines a label

  int                    islabel;
  int                    offset;
  int                    macrolabel;

  // Symbol defines a value

  int                    valid;
  int                    valdefline;
  int                    value;
  char                   valfilename[MAX_FILENAME_LENGTH+1];
  int                    segment;
  int                    mask;

  // Symbol defines a register

  int                    isdefine;
  int                    regdefline;
  int                    reg;
  char                   regfilename[MAX_FILENAME_LENGTH+1];
};

  ////////////////////////////////////////////////////////////////////////////
  //
  // Symbol-table class
  //

class symbolTable
{

public:

  // Constructor/destructor

  symbolTable();
  ~symbolTable();

  // Public member functions

  int                init(int cs=0, unsigned int hashsize=HASH_DEFAULT_SIZE);
  symbolTableEntry*  add(char *name);
  symbolTableEntry*  get(char *name, int inscope = -1);
  int                scopeDown(void);
  int                scopeUp(void);
  int                printTable(int fromscope=-1,int toscope=-1,int format=0);

private:

  // Private member functions

  inline int         hash(char *name);

  // Private data members

  //  Scope stack entry

  struct symbolScopeStack
  {
    symbolScopeStack       *up;
    symbolScopeStack       *down;
    symbolTableEntry       *first;
    symbolTableEntry       *last;
    int                    level;
  };

  int                hashSize;
  int                scope;
  symbolTableEntry   **symtab;
  symbolScopeStack   *scopeTop;
  symbolScopeStack   *currentScope;
  symbolTableEntry   *currentSymbol;
  int                casesensitive;

};

#endif /* _SYMBOL_HH_ */

/// END OF FILE //////////////////////////////////////////////////////////////
