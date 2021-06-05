//////////////////////////////////////////////////////////////////////////////
//
//  File        : symbol.cc
//
//  Author      : Tom Mortensen - Copyright (C) 1999
//
//  Description : Symbol table (see additional info below).
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
//
//  Description :
//
//  This module implements the symbol-table, and the functions needed to
//  manipulate it. The symbol-table supports scopes, scope import/export
//  and arbitrary types.
//
//  The size of the hash vector is set using 'init', and can't be changed
//  after initialization.  The default size of the hash-table is 1024.
//  The size must be 2^N, N=1, 2, ...
//
//  To get a case in-sensitive symbol table, initialize with : init(TRUE);
//
//  'symbol.hh' defines the class interface and the structure of a
//  symbol-table entry.
//
//  Functions   :
//
//  init         - Initialize symbol-table
//  add          - Add symbol to symbol-table, no check is made to see
//                 if symbol already exist.
//  get          - Return pointer to symbol.
//  scopeDown    - Add new scope
//  scopeUp      - Delete scope
//  printTable   - Print symbol table.
//
//////////////////////////////////////////////////////////////////////////////

/// Include //////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "symbol.hh"
#include "utils.hh"

/// Macros ///////////////////////////////////////////////////////////////////

#define LOWERCOMP(c1, c2)       (tolower(c1)==tolower(c2))

/// Functions ////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////////
  //
  // Constructor
  //

symbolTable::symbolTable(void)
{

  hashSize      = 0;
  symtab        = NULL;
  scope         = 0;
  scopeTop      = NULL;
  currentScope  = NULL;
  currentSymbol = NULL;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Destructor
  //

symbolTable::~symbolTable(void)
{
  if(symtab)
    delete symtab;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Allocate and initialize a new symbol table.
  //

int symbolTable::init(int cs, unsigned int hashsize)
{
  unsigned int      size = 1;
  symbolScopeStack  *newscope;

  // Return if already initialized

  if(symtab)
    return 0;;

  // Align hashsize to 2^N

  while(hashsize>>=1)
    size<<=1;

  if(size>HASH_MAX_SIZE)
    size = HASH_MAX_SIZE;

  // Check case sensitive

  if(cs)
    casesensitive = TRUE;
  else
    casesensitive = FALSE;

  // Allocate mew symbol-table

  if(size && (symtab = new symbolTableEntry*[size]))
  {
    // Clear hash-table

    memset(symtab,0,size*sizeof(symbolTableEntry*));
    hashSize = size - 1;

    // Allocate first element on scope stack

    if(! (newscope = new symbolScopeStack))
    {
      delete symtab;
      symtab = NULL;
      return 0;
    }

    currentScope          = newscope;
    scopeTop              = newscope;

    currentScope -> down  = NULL;
    currentScope -> up    = NULL;
    currentScope -> last  = NULL;
    currentScope -> first = NULL;

    // scope is set to 1

    currentScope -> level = ++scope;

    return size;
  }

  return 0;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // The hash function does three things to create the hash value :
  //
  // 1 : Adding the ASCII value of all characters
  //
  //     This works fine for small hash tables, but hash values tends to pile
  //     up in large tables, since ASCII values for alpha numeric characters
  //     are "small", ie. adding 1 to 8 values in the interval 48 to 123
  //     result in a hash value between 48 and 984, so there would be no point
  //     in using a hash table larger than 1024 (since we assume that the
  //     table is 2^N, where N = 1, 2, ...
  //
  // 2 : Shifting value right
  //
  //     To improve the hash functions we shift the previous hash value right
  //     4 times, this creates much larger values, thus solving the 'add'
  //     problem.
  //
  // 3 : XOR'ing value
  //
  //     The solution to the 'add' problem creates another problem when using
  //     small hash tables, since the shifting has the effect that the hash
  //     value tends to forget the previous character values, because they are
  //     shifted to the right, and the eventually lost when the hash value
  //     is AND'ed with the size of the hash table. This problem is fixed
  //     by XOR'ing the previous value with the shifted value.
  //
  // An alternate solution that does not change the name pointer :
  //
  //   for(h=i=0; name[i]; h^= ( h << 4 ) + name[i++]);
  //   h&=hashsize;
  //

inline int symbolTable::hash(char *name)
{
  int h = 0;

  if(!casesensitive)
    while(*name)
      h^= ( h << 4 ) + tolower(*name++);
  else
    while(*name)
      h^= ( h << 4 ) + *name++;

  return  h &= hashSize;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Get a pointer to a symbol.
  //

symbolTableEntry* symbolTable::get(char *name, int inscope)
{
  int notfound;
  int p;

  // Get a pointer to the first element in the hash chain

  symbolTableEntry *next = symtab[hash(name)];

  // If 'next' == NULL then the symbol is not in the table and we return.

  if(!next)
    return NULL;

  // Set scope

  if(inscope<0)
    inscope = scope;

  notfound = true;

  // If 'next' != NULL the chain is not empty, and we have have to
  // scan all symbols at the current scope level. Since new symbols are
  // added at the front of the chain, we can end the scan, when the first
  // identifier with a higher scope level (lower 'scope') is found.

  if(casesensitive)
  {
    while(next && (inscope<=next->scope)&&(notfound=strcmp(name, next->name)))
      next = next -> next;
  }
  else
  {
    while(next && (inscope<=next->scope) && notfound)
    {
      p = 0;
      while( name[p] && next->name[p] && LOWERCOMP(name[p],next->name[p]) )
        p++;
      if( name[p] || next->name[p] )
        next = next -> next;
      else
        notfound = FALSE;
    }
  }

  if(!notfound)
    return next;

  return NULL;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Add a symbol to symbol-table.
  //

symbolTableEntry* symbolTable::add(char *name)
{
  int h;

  // Get a pointer to the first element in the hash chain

  symbolTableEntry *next = symtab[h=hash(name)];

  // Allocate new symbol

  symtab[h] = new symbolTableEntry;

  if(!(symtab[h]))
    return NULL;

  // Clear symbol

  memset(symtab[h], 0, sizeof(symbolTableEntry));

  // Link with next symbol in chain.

  symtab[h] -> next = next;

  if(next)
    next -> prev = symtab[h];

  // Add to scope chain

  if(currentSymbol)
  {
    currentSymbol -> down = symtab[h];
    symtab[h] -> up = currentSymbol;
  }
  else
    currentScope -> first = symtab[h];

  currentSymbol = symtab[h];

  currentScope -> last = symtab[h];

  // Setup new symbol

  if(strlen(name) > MAX_ID_LENGTH)
    internalerror("ID TOO LONG");

  strcpy(symtab[h]->name, name);

  symtab[h] -> scope = scope;

  next = symtab[h];

  // return pointer to new symbol

  return symtab[h];
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Add new scope
  //

int symbolTable::scopeDown(void)
{
  symbolScopeStack *newscope;

  // Allocate new element on top of the scope stack.

  if(! (newscope = new symbolScopeStack))
    return -1;

  // Setup new element

  currentScope -> down  = newscope;
  newscope     -> up    = currentScope;
  currentScope          = newscope;
  currentScope -> down  = NULL;
  currentScope -> first = NULL;
  currentScope -> last  = NULL;
  currentScope -> level = ++scope;
  currentSymbol         = NULL;

  return scope;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Delete scope
  //

int symbolTable::scopeUp(void)
{
  symbolTableEntry *next, *current;
  int h;

  current = currentScope -> first;

  // Kill current scope

  while(current)
  {
    if(current->prev)
      current -> prev -> next = current -> next;
    else
      symtab[h=hash(current -> name)]= current -> next;

    if(current->next)
      current -> next -> prev = current -> prev;

    next = current -> down;
    delete current;
    current = next;
  }

  // Decrement scope

  if(currentScope -> up)
  {
    currentScope = currentScope -> up;
    delete currentScope -> down;
    currentScope -> down = NULL;
    currentSymbol = currentScope -> last;
  }
  else
  {
    currentScope -> up    = NULL;
    currentScope -> down  = NULL;
    currentScope -> first = NULL;
    currentScope -> last  = NULL;
  }

  return --scope;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Print symbol-table to stdout.
  //

int symbolTable::printTable(int fromscope, int toscope, int format)
{
  symbolScopeStack *thescope = scopeTop;
  symbolTableEntry *next;
  int              count =0;

  if(toscope == -1)
    toscope = scope;
  if(fromscope == -1)
    fromscope = scope;

  if(!thescope)
    printf("Symbol table is empty\n");
  else
  {
    while(thescope)
    {
      if( (thescope -> level >= fromscope ) && (thescope -> level <= toscope ) )
      {
        printf("\n+---------------------------------------------------+\n");
        printf("| S:%5i  I V S M                                  |\n",thescope -> level);
        printf("|---------------------------------------------------|\n");
        next = thescope -> first;
        while(next)
        {
          // Print detailed information

          if(format)
          {
          }

          // Print just the symbol names
          else
            printf("| %08X %i %i %X %i %-32s |\n",next->value,next->isvar,next->valid,next->segment,next->macro,next->name);

          next = next -> down;
          count++;
        }
        printf("+---------------------------------------------------+\n");

      }
      thescope = thescope -> down;
    }
  }

  return count;

}

/// END OF FILE //////////////////////////////////////////////////////////////

