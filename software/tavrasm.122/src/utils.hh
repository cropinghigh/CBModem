//////////////////////////////////////////////////////////////////////////////
//
//  File        : utils.hh
//
//  Author      : Tom Mortensen - Copyright (C) 1999
//
//  Description : Utility header
//
//  History
//  ========================================================================
//
//  980902      : Tom - File created.
//  990124      : Tom - Added GPL notice.
//  001101      : Brian Rhodefer - Added "fopenInIncpath()" 
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

#ifndef _UTILS_HH_
#define _UTILS_HH_

/// Include //////////////////////////////////////////////////////////////////

#include "symbol.hh"
#include <stdio.h>

/// Prototypes ///////////////////////////////////////////////////////////////

void insertInst(int v, int s);
void checkSupported(int opcode);
symbolTableEntry* getsym(char *name);
void insertcode(int val);
void insertdbdw(int val);
void insertdata(int val);
FILE * fopenInIncpath(const char *name, const char *mode);

  /// Error handling ///////////////////////////////////////////////////////

int   error(int errorno, ...);
int   errorin(int errorno, ...);
int   warning(int warnno, ...);
int   warningin(int warnno, ...);
void  errorexit(int errorno, ...);
void  internalerror(char const *fmt, ...);
int   message(int msgno, ...);

  /// Strip comments ///////////////////////////////////////////////////////

void  tolog(void);
int   getfilename(char *name, char *newname);
int   striprem(char *str);
int   exptabs(char *str);
int   checkfilename(char *name);

#endif /* _UTILS_HH_ */

/// END OF FILE //////////////////////////////////////////////////////////////

