//////////////////////////////////////////////////////////////////////////////
//
//  File        : avrasm.cc
//
//  Author      : Tom Mortensen - Copyright (C) 1999
//
//  Description : AVR assembler entry point (See additional info below).
//
//  History
//  ========================================================================
//
//  980902      : Tom - File created.
//  990117      : Tom - Fixed alignment of .db in CSEG with forward refs.
//  990124      : Tom - Added GPL notice.
//  990127      : Tom - Added support for German.
//  990303      : JED - Modified to support intel hex as the default. Now
//                      automatically determines the output file name.
//                      Exits with a count of the number of errors (for make).
//  990329      : Tom - Fixed problem with tabs after include filename
//  990329      : Tom - Added support for hex values like 1AB without $ or 0x
//  990512      : Tom - Fixed detection on binary numbers.
//                Tom - Fixed problem with memset of context stack
//  990516      : Tom - Added support for wrapping in branch/rcall/rjmp
//  990519      : Tom - Added '-g' switch to generate generic hex.
//  990522      : Tom - Added support for PC/DC/EC.   
//  991209	: Tom - Fixed problem with BRBC/BRBS (they did not work :(
//  991211      : Tom - Added the new instructions: 'eijmp', 'eicall', 'espm',
//                      'mulsu', 'fmul', 'fmuls', 'fmulsu', 'muls', 'movw', 
//                      'elpm'. And changed behaviour of 'lpm' according to
//                      Atmel specs.
//  991211      : Tom - Added support for Spanish
//  991211      : Tom - Fixed .endm problem (it had to be lower case)
//  991212	: Tom - Now the Alpha problem is fixed (I forgot to move
//                      fix from test to release source).
//  991212      : Tom - Fixed promlem with detecting supported instructions
//                      (Introduced in 1.10)
//  991213	: Tom - Added 'spm' instruction
//  991222      : Kurt- Added command line option -x for local labels
//  001031      : Tom - Added command line option -f to allow forward .orgs
//  001031      : Tom - Fixed problem with saving rom/erom files when forward
//                      orgs were used.
//  001101	: Brian - Add support for multiple include search paths
//  010112	: Tom - Increased max id length to 64
//  010319      : Timothy Lee - Added byte1 function (sames as low())
//  010403      : Tom - Fixed path delimiter problem in include path
//  020418      : Tom - Fixed error message for undefined references.
//  030217	: Patrick - Changes to allow compilation with gcc 3.2
//  040109	: John Romein (john@cs.vu.nl) - Added .align support
//  041204      : Tom - Fixed problem with the .align directive
//  041204      : Tom - Added ATmega 128 device support
//  041212      : Tom - Fixed problem with .db 00
//  050106      : Tom - Fixed  symtab = (new (symbolTableEntry*))[size] :-)
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
// Kurt = Kurt Stege
// E-Mail : kurt-stege@gmx.de
//
// Patrick = Patrick Dreker
// E-Mail : patrick@dreker.de
// 
//////////////////////////////////////////////////////////////////////////////
//
//  BUGS - What did I do wrong (Always learn from your mistakes)
// =========================================================================
//
// - Alginemnt of .db in CSEG with forward references (fixed in 1.01)
//
//   I had a function that was called after evry line that checked the
//   alignment of the 'program counter' and another function that
//   updated the logfile, I had merged the two functions (at a late
//   night code clean-up). Problem : The logfile function returns
//   immediately during the first pass (the logfile is not created
//   until the second pass), so the code segment alignment check
//   wasn't performed until the second pass, so if you had a forward
//   reference to a un-aligned code segment address, this was not
//   aligned :-(
//
//   Thanks to Rob Penny for reporting this bug.
// 
// - Problem with tabs after include filename (fixed in 1.04)
//
//   I only stripped trailing spaces, not tabs. This was fixed in
//   striprem.
//
//   Thanks to Andreas Bogk for reporting this bug.
//
// - Fixed problem with detection of binary numbers like '0b010101'.
//   This was bug was introduced by the new hex detection feature in
//   V. 1.04 :( 
//	       
//   Thanks to Randy Ott for reporting this bug.
//
// - Fixed problem with memset of context statck that caused the winDOS
//   version to crash on some systems (strange that Linux/NT did not detect
//   this obvious memory violation).
//
//   Thanks to my friend Henning for finding this.
//
// - Fixed problem with SBR
//
//   This is the same as ORI reg, 2^bitno, but the SBR version had a bug
//   (one of those that generates faulty code), bit 0 was always set. 
//   This is the second "wrong code generated" bug, I guess no software is
//   perfect! 
//
//   Thanks to Dean Huxley for reporting this (and supplying a patch).
//
// - BRBC/BRBS generated incorrect (invalid) opcodes.
//
//   Ooops the third bug that generated incorrect opcodes.
//   
//   The problem was that I used the destination operand (the status register
//   bit number) as boths source and destination (they are named 'values' and
//   'valued' :)
//
//   I guess nobody has found this until now, because we normally prefer
//   to specify 'BRCC xxx' (BRanc If Carry Cleared) instead of 'BRBC 0, xxx'.
//
//   Hmmm, poor excuse!
//
//   Thanks to Todd M. Squires for reporting this.
//
// - Fixed problem when generating HEX output on Alpha's. Actually this was
//   not an Alpha problem. It happend because I made a incorrect asumption
//   on how compilers generate code, that just happend to be correct for gcc.
//
//   Thanks to Todd M. Squires for reporting this (and supplying a patch).
//
//////////////////////////////////////////////////////////////////////////////
//
//  The following functions are defined (move to documentation):
//
//  - LOW(expr)   returns the low byte of an expression
//  - HIGH(expr)  returns the second byte of an expression
//  - BYTE1(expr) returns the low byte of an expression (same as LOW)
//  - BYTE2(expr) returns the second byte of an expression (same as HIGH)
//  - BYTE3(expr) returns the third byte of an expression
//  - BYTE4(expr) returns the fourth byte of an expression
//  - LWRD(expr)  returns bits 0-15 of an expression
//  - HWRD(expr)  returns bits 16-31 of an expression
//  - PAGE(expr)  returns bits 16-21 of an expression
//  - EXP2(expr)  returns 2^expression
//  - LOG2(expr)  returns the integer part of log2(expression)
//
//////////////////////////////////////////////////////////////////////////////
//
// Source code notice:
//
// - Global varaibles
//
//   This program uses a lot of program wide global variables. They are
//   all defined in 'avrasm.hh'. Names of system wide globals start
//   with 'yy' (stupid, has to be changed - Tom). When writing event
//   driven software like a compiler, there is no point in trying to
//   avoid them, but the implementation could be more elegant than the
//   one I have chosen. Maybe I will get around to fixing this in the
//   future (the names will be changed).
//
// - Use of {}
//
//   When writing this program i used the GNU preferred style for {} i.e.
//
//   void foo(void)
//   {
//
//   }
//
//   But I have since been persuaded to change to the style preferred
//   by many programmers:
//
//   void food(void) {
//
//   }
//
//   It is not a high priority to change this, but if anyone would like
//   to add/fix something, they should use the later style.
//
// - History
//
//   Any changes after version 1.00 should be noted in the header of the
//   modified file, as well as in the header of 'avrasm.cc' (this file).
//
//////////////////////////////////////////////////////////////////////////////
//
// Odd things found in the ATMEL DOS assembler V. 1.21 (items 1-21 send to
// ATMEL October 1998), if you find any other, please report to them.
// I don't know if they fix the bugs reported to them, but they did send
// me a nice e-mail back when i reported the list of bugs below, and
// in addition to this they added a reference to tavrasm on their web site :-)
//
//  1: This generates code that is illegal (0xFFFF is generated) :
//
//     bld     r16,   -1       ; Bit Load from T to Register bit
//
//     Other instructions have similar problems
//
//  2: If backwards rjump/rcall is 2049 it will generate a jump forward 2047
//
//  3: For relative jmp/call the 2k boundary is incorrect, it will only
//     allow 0x7FE in relative distance forward
//
//  4: For all branch instructions the -64/+63 offset is incorrect, it
//     will only allow +62 in relative distance forward
//
//  5: Illegal opcodes in macro is not detected, and and 2 words of
//     code is generated, but is reported unused.
//
//  6: If file is included in macro, remaining part of macro is not
//     assembled
//
//  7: If a label is defined in an include file and the file is
//     included both within a macro and in the outer scope, any reference
//     to that label is not calculated correctly. If file is included
//     before macro is used, label within macro refers to label defined in
//     first include.  If Include in outer scope is done after macro, the
//     reference is "random".
//
//  8: If a label is specified in DSEG/ESEG in a macro, references to that
//     label are incorrect.
//
//  9: If last line of input file is not empty, that line is logged
//     incorrectly
//
// 10: If .device is specified the "Address extension record" in Intel hex
//     format is omitted.
//
// 11: If macro error (like no name specified), or if .include file can
//     not be found, error in the remaining of the file is not reported.
//
// 12: No error is reported for lines like ,foo or ,include "file.inc"
//
// 13: If empty file is assembled the "Address extension record" in Intel
//     hex format is omitted.
//
// 14: Multiple .device can be specified, but only last occurrence has
//     any effect, and it also effects previous instructions.
//
// 15: Constants are 'wrapped' (might be ok, but should generate a warning).
//
// 16: If only .byte is specified in .eseg no eeprom file is generated
//     (might be ok).
//
// 17: .dw and .db in CSEG is logged with high and low byte swapped
//
// 18: If a file is included twice that contains a macro with a label, a
//     duplicate label is reported.
//
// 19: If a macro is defined twice, no duplicate name error is generated,
//     and last definition is used with no warning.
//
// 20: Escape sequences are not supported in strings and characters (why
//     not ?)
//
// 21: If file is include in macro, line number is 0 for that file in .obj,
//     but "in macro" indicator is not set.
//
//////////////////////////////////////////////////////////////////////////////

/// Include //////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "avrasm.hh"
#include "symbol.hh"
#include "semantic.hh"
#include "utils.hh"
#include "devices.hh"

/// Globals //////////////////////////////////////////////////////////////////

GLOBALS(;);

/// Externals ////////////////////////////////////////////////////////////////

extern int      yyparse(void);
extern FILE     *yyin;
extern char     messages[MESSAGE_COUNT][MESSAGES_LEN];

/// Functions ////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////////
  //
  // Main
  //

int main(int argc, char **args)
{
  int   parseresult;
  int   c;
  int   unusedcode = 0;
  int   unusederom = 0;

  /// Process arguments //////////////////////////////////////////////////////

  yyIncPathCount = 0;

  if(!getargs(argc, args))
    return 0;

  /// Print header ///////////////////////////////////////////////////////////

  if(yycfg -> info)
    printf(INFOSTR);

  /// Open input file ////////////////////////////////////////////////////////

  if(! (yyin= fopen(yyfilename,"r")) )
    errorexit(X_UNABLE_TO_OPEN_FILE, yyfilename);
  if(! (yyin2= fopen(yyfilename,"r")) )
    errorexit(X_UNABLE_TO_OPEN_FILE, yyfilename);

  if(yylogfilename[0])
  {
    if(! (yylogfile = fopen(yylogfilename,"w")) )
      errorexit(X_UNABLE_TO_OPEN_LOGFILE, yylogfilename);
  }
  else
    yylogfile = NULL;

  /// Setup global symbols ///////////////////////////////////////////////////

  INITEXTERN;
  yyparseno      = 0;
  yyfilecount    = 1;

  if( !(yyfilelist[0] = new char[MAX_FILENAME_LENGTH]) )
    errorexit(X_OUT_OF_MEMORY);

  strcpy(yyfilelist[0], yyfilename);

  /// Allocate buffers ///////////////////////////////////////////////////////

  if( !(symtab = new symbolTable) )
    errorexit(X_OUT_OF_MEMORY);
  if( !(yycodebuf     = new unsigned char[MAX_CODE_SIZE]) )
    errorexit(X_OUT_OF_MEMORY);
  if( !(yyerombuf     = new unsigned char[MAX_EROM_SIZE]) )
    errorexit(X_OUT_OF_MEMORY);
  if( !(yycodeusage   = new unsigned char[MAX_CODE_SIZE]) )
    errorexit(X_OUT_OF_MEMORY);
  if( !(yyeromusage   = new unsigned char[MAX_EROM_SIZE]) )
    errorexit(X_OUT_OF_MEMORY);
  if( !(yycodeline    = new unsigned short[MAX_CODE_SIZE]) )
    errorexit(X_OUT_OF_MEMORY);
  if( !(yylinetxt     = new char[MAX_LINE_LENGTH+1]) )
    errorexit(X_OUT_OF_MEMORY);
  if( !(yyinline      = new char[MAX_LINE_LENGTH+1]) )
    errorexit(X_OUT_OF_MEMORY);
  if( !(yyinlinenew   = new char[MAX_LINE_LENGTH+1]) )
    errorexit(X_OUT_OF_MEMORY);
  if( !(yyinlineold   = new char[MAX_LINE_LENGTH+1]) )
    errorexit(X_OUT_OF_MEMORY);

  for(c=0; c<MAX_CONTEXT_DEPTH; c++)
  {
    if( !(yycontextstack[c]= new context) )
      errorexit(X_OUT_OF_MEMORY);
    else
      memset(yycontextstack[c],0,sizeof(context));
  }

  memset(yycodebuf,     0, sizeof(unsigned char)*MAX_CODE_SIZE);
  memset(yyerombuf,     0, sizeof(unsigned char)*MAX_EROM_SIZE);
  memset(yycodeusage,   0, sizeof(unsigned char)*MAX_CODE_SIZE);
  memset(yyeromusage,   0, sizeof(unsigned char)*MAX_EROM_SIZE);
  memset(yysymbolstack, 0, sizeof(symbolTable*)*MAX_CONTEXT_DEPTH);
  memset(yycodeline,    0, sizeof(unsigned short)*MAX_CODE_SIZE);

  /// Setup context stack ////////////////////////////////////////////////////

  strcpy(yycontextstack[0]->filename, yyfilename);

  /// Setup device info //////////////////////////////////////////////////////

  for(c=0; c<MAX_DEVICES; c++) {
	  yydevices[c] = devices[c];
  }

  /// Fill line read-ahead buffer ////////////////////////////////////////////

  if(!fgets(yyinline, MAX_LINE_LENGTH, yyin2))
    strcpy(yyinline,"\n");
  if(!fgets(yyinlinenew, MAX_LINE_LENGTH, yyin2))
    strcpy(yyinlinenew,"\n");

  STRIPNR(yyinlinenew);
  STRIPNR(yyinline);

  /// Setup symbol table /////////////////////////////////////////////////////

  symtab -> init(yycfg->casesensitive);
  yysymbolstack[0] = symtab;

  /// Start assembler ////////////////////////////////////////////////////////

  if(yylogfile)
  {
    fputs(INFOSTR, yylogfile);
    fputs("\n+------------------+--------------------", yylogfile);
    fputs("---------------------------------------+\n", yylogfile);
    fputs("| Value            | Text               ",   yylogfile);
    fputs("                                       |\n", yylogfile);
    fputs("|------------------|--------------------",   yylogfile);
    fputs("---------------------------------------|\n", yylogfile);
  }

  parseresult = yyparse();

  yycodepos = (yycodepos>yycodeposhigh) ? yycodepos : yycodeposhigh;
  yydatapos = (yydatapos>yydataposhigh) ? yydatapos : yydataposhigh;
  yyerompos = (yyerompos>yyeromposhigh) ? yyerompos : yyeromposhigh;

  for(c=0;c<yycodepos;c+=2)
    if(!yycodeusage[c])
      unusedcode++;
  for(c=0;c<yyerompos;c++)
    if(!yyeromusage[c])
      unusederom++;

  if(yylogfile)
  {
    fputs("+------------------+--------------------",  yylogfile);
    fputs("---------------------------------------+\n", yylogfile);
    fprintf(yylogfile, "\n");
    fprintf(yylogfile, MSG(M_ERRORS     ), yyerrorcount);
    fprintf(yylogfile, "\n");
    fprintf(yylogfile, MSG(M_WARNINGS   ), yywarningcount);
    fprintf(yylogfile, "\n");
    fprintf(yylogfile, MSG(M_CODE       ), yycodepos/2);
    fprintf(yylogfile, "\n");
    fprintf(yylogfile, MSG(M_ROM        ), yyerompos);
    fprintf(yylogfile, "\n");
    fprintf(yylogfile, MSG(M_DATA       ), yydatapos-DEVINF.datastart);
    fprintf(yylogfile, "\n");
    fprintf(yylogfile, MSG(M_UNUSED_CODE), unusedcode);
    fprintf(yylogfile, "\n");
    fprintf(yylogfile, MSG(M_UNUSED_ROM ), unusederom);
    fprintf(yylogfile, "\n");
    fclose(yylogfile);
  }

  if(yycfg->info)
  {
    printf("\n");
    message(M_ERRORS      , yyerrorcount);
    message(M_WARNINGS    , yywarningcount);
    message(M_CODE        , yycodepos/2);
    message(M_ROM         , yyerompos);
    message(M_DATA        , yydatapos-DEVINF.datastart);
    message(M_UNUSED_CODE , unusedcode);
    message(M_UNUSED_ROM  , unusederom);
  }

  /// Save results ///////////////////////////////////////////////////////////

  if(!yyerrorcount)
  {
    if(yycfg -> intel)
      saveIHF();
    else if(yycfg -> motorola)
      saveMotorola();
    else if(yycfg -> obj)
      saveObj();
    else if(yycfg -> bin)
      saveBin();
    else
      saveGeneric();
  }

  exit(yyerrorcount);

}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Get commandline arguments.
  //
  ////////////////////////////////////////////////////////////////////////////
  //
  //  -o = output file
  //  -r = rom file
  //  -e = list file
  //  -v = verbose
  //  -w = no warnings
  //  -c = case sensitive labels/defines
  //  -l = limit log width to 80 characters
  //  -x = allow local labels
  //  -a = do not align .db in CSEG
  //  -m = output Motorola S-record format
  //  -i = output Intel HEX format
  //  -j = output ATMEL .obj format
  //  -g = output ATMEL generic format
  //

int getargs(int argc, char **args)
{
  int c;
  int ok = TRUE;

  if(argc==1)
  {
    printf(INFOSTR);
    printf("\n");
    printf(HELPSTR);
    return FALSE;
  }

  // List devices ////////////////////////////////////////////////////////////

  else if( (argc==2) && !strcmp(args[1],"-d")) {

    int devno = 0;
    printf("+---------------------------------------------------"
	   "-----------------+\n");
    message(M_DEVICE_HEADER);
    printf("|------------------+-----------+------------+-------"
	   "---+-------------|\n");
    while(devices[devno].name[0]) {
      printf("| %-16s |  %8d | %10d | %8d | %11d |\n", devices[devno].name,
	     devices[devno].datastart,
	     devices[devno].flashsize,
	     devices[devno].ramsize,
	     devices[devno].eepromsize);
      devno++;
    }
    printf("+---------------------------------------------------"
	   "-----------------+\n");
    return false;
  }

  /// Allocate filename strings //////////////////////////////////////////////

  if(!(yyfilename     = new char[MAX_FILENAME_LENGTH+1]))
    errorexit(X_OUT_OF_MEMORY);
  if(!(yyromfilename  = new char[MAX_FILENAME_LENGTH+1]))
    errorexit(X_OUT_OF_MEMORY);
  if(!(yylogfilename  = new char[MAX_FILENAME_LENGTH+1]))
    errorexit(X_OUT_OF_MEMORY);
  if(!(yyoutfilename  = new char[MAX_FILENAME_LENGTH+1]))
    errorexit(X_OUT_OF_MEMORY);

  yyfilename[0]=yyromfilename[0]=yylogfilename[0]=yyoutfilename[0] = '\0';

  /// Setup default configuration ////////////////////////////////////////////

  if( !(yycfg = new configuration) )
    errorexit(X_OUT_OF_MEMORY);

  memset(yycfg, 0, sizeof(configuration));

  yycfg -> info           = FALSE;  // -v
  yycfg -> warnings       = TRUE;   // -w
  yycfg -> casesensitive  = FALSE;  // -c
  yycfg -> cutlog         = FALSE;  // -l
  yycfg -> aligndb        = TRUE;   // no command line option anymore
  yycfg -> motorola       = FALSE;  // -m
  yycfg -> intel          = FALSE;  // -i
  yycfg -> hex		  = FALSE;  // -g
  yycfg -> obj            = FALSE;  // -j
  yycfg -> bin            = FALSE;  // -b
  yycfg -> wrap		  = FALSE;  // -a
  yycfg -> local_labels   = FALSE;  // -x
  yycfg -> forwardorg     = FALSE;  // -f
  yycfg -> addressext	  = TRUE;   // -h

  /// Get command line options ///////////////////////////////////////////////

  for(c=1;c<argc;c++)
  {
    if(!ok)
      return FALSE;

    if(args[c][0]!='-')
    {
      ok = FALSE;

      if(strlen(args[c])>MAX_FILENAME_LENGTH)
        message(M_INPUT_FILENAME_TOO_LONG);
      else if(!checkfilename(args[c]))
        message(M_ILLEGAL_INPUT_FILENAME);
      else if(yyfilename[0])
        message(M_MULTIPLE_INPUT);
      else
      {
        ok = TRUE;
        strcpy(yyfilename, args[c]);
      }
    }
    else
    {
      if(strlen(args[c])>2)
      {
        message(M_UNKNOWN_OPTION ,args[c]);
        ok = FALSE;
      }
      else
      {
        switch(args[c][1])
        {

          /// Include path ////////////////////////////////////////////////////
	  //
	  case 'I' :
	  {
	      ok = FALSE;
	      if (c == argc-1)
		      message (M_NO_INCLUDEPATH);

              else if(strlen(args[c+1]) > MAX_INCPATH_LENGTH)
		      message(M_INCPATH_TOO_LONG);

	      else if (MAX_INCPATH_QTY == yyIncPathCount)
		      message(M_TOO_MANY_INCPATHS);

	      else
	      {
		  yyIncludePaths[yyIncPathCount++] = args[++c];
		  ok = TRUE;
	      }
	  } break;


          /// output file ////////////////////////////////////////////////////

          case 'o' :
          {
            ok = FALSE;

            if(c==argc-1)
              message(M_NO_OUTPUT_FILENAME);
            else if(strlen(args[c+1])>MAX_FILENAME_LENGTH)
              message(M_OUTPUT_FILENAME_TOO_LONG);
            else if(!checkfilename(args[c+1]))
              message(M_ILLEGAL_OUTPUT_FILENAME);
            else if(yyoutfilename[0])
              message(M_MULTIPLE_OUTPUT);
            else
            {
              strcpy(yyoutfilename, args[c+1]);
              c++;
              ok = TRUE;
            }
          } break;

          /// rom file ///////////////////////////////////////////////////////

          case 'e' :
          {
            ok = FALSE;

            if(c==argc-1)
              message(M_NO_LIST_FILENAME);
            else if(strlen(args[c+1])>MAX_FILENAME_LENGTH)
              message(M_LIST_FILENAME_TOO_LONG);
            else if(!checkfilename(args[c+1]))
              message(M_ILLEGAL_LIST_FILENAME);
            else
            {
              strcpy(yylogfilename, args[c+1]);
              c++;
              ok = TRUE;
            }
          } break;

          /// list file //////////////////////////////////////////////////////

          case 'r' :
          {
            ok = FALSE;

            if(c==argc-1)
              message(M_NO_ROM_FILENAME);
            else if(strlen(args[c+1])>MAX_FILENAME_LENGTH)
              message(M_ROM_FILENAME_TOO_LONG);
            else if(!checkfilename(args[c+1]))
              message(M_ILLEGAL_ROM_FILENAME);
            else
            {
              strcpy(yyromfilename, args[c+1]);
              c++;
              ok = TRUE;
            }
          } break;

          /// verbose ////////////////////////////////////////////////////////

          case 'v' : yycfg -> info              = TRUE;  break;

          /// no warnings ////////////////////////////////////////////////////

          case 'w' : yycfg -> warnings          = FALSE; break;

          /// case sensitive labels/defines //////////////////////////////////

          case 'c' : yycfg -> casesensitive     = TRUE;  break;

          /// limit log width to 80 characters ///////////////////////////////

          case 'l' : yycfg -> cutlog            = TRUE;  break;

          /// Wrap relative jumps/branches ///////////////////////////////////

          case 'a' : yycfg -> wrap              = TRUE;  break;

          /// allow local labels /////////////////////////////////////////////
          
          case 'x' : yycfg -> local_labels      = TRUE;  break;

          /// do not align .db in CSEG ///////////////////////////////////////

          //  case 'a' : yycfg -> aligndb           = FALSE; break;

          /// output Motorola S-record format ////////////////////////////////

          case 'm' :
          {
            if(yycfg->intel || yycfg->obj || yycfg->bin || yycfg->hex )
            {
              message(M_MORE_MIJBH);
              ok = FALSE;
            }
            else
              yycfg -> motorola = TRUE;
          }
          break;

          /// output Intel HEX format ////////////////////////////////////////

          case 'i' :
          {
            if(yycfg->motorola || yycfg->obj || yycfg->bin || yycfg->hex)
            {
              message(M_MORE_MIJBH);
              ok = FALSE;
            }
            else
              yycfg -> intel = TRUE;

            
          }
          break;

          /// output .obj format /////////////////////////////////////////////

          case 'j' :
          {
            if(yycfg->motorola || yycfg->intel || yycfg->bin || yycfg->hex)
            {
              message(M_MORE_MIJBH);
              ok = FALSE;
            }
            else
              yycfg -> obj   = TRUE;
          }
          break;

          /// output .bin format /////////////////////////////////////////////

          case 'b' :
          {
            if(yycfg->motorola || yycfg->intel || yycfg->obj || yycfg->hex)
            {
              message(M_MORE_MIJBH);
              ok = FALSE;
            }
            else
              yycfg -> bin   = TRUE;
          }
          break;

          /// Supress .org warnings //////////////////////////////////////////

          case 'f' :
          {
            yycfg -> forwardorg = TRUE;
          }
          break;

          /// output .obj format /////////////////////////////////////////////

          case 'g' :
          {
            if(yycfg->motorola || yycfg->intel || yycfg->obj || yycfg->bin)
            {
              message(M_MORE_MIJBH);
              ok = FALSE;
            }
            else
              yycfg -> hex   = TRUE;
          }
          break;

          /// No address entension record in Intel HEX output ////////////////

          case 'h' : yycfg -> addressext = FALSE; break;

          /// ignore -d flag /////////////////////////////////////////////////

          case 'd' : break;

          /// Unknown option /////////////////////////////////////////////////

          default  : message(M_UNKNOWN_OPTION , args[c]); ok=FALSE;
        }
      }
    }
  }

  if(!yyfilename[0])
  {
    ok = false;
    message(M_NO_INPUT_FILENAME);
  }

  if(!yycfg->motorola&&!yycfg->hex&&!yycfg->intel&&!yycfg->bin&&!yycfg->obj)
    yycfg->intel = TRUE;

  if(!yyoutfilename[0])
  { 
    char *type_start;

    strcpy(yyoutfilename, yyfilename);
    type_start = strrchr(yyoutfilename, '.');
    if (type_start == NULL)
      type_start = yyoutfilename + strlen(yyoutfilename);

    if (yycfg->intel)
      strcpy(type_start, ".hex");
    else if (yycfg->motorola)
      strcpy(type_start, ".s");
    else if (yycfg->bin)
      strcpy(type_start, ".bin");
    else if (yycfg->obj)
      strcpy(type_start, ".obj");
    else if (yycfg->hex)
      strcpy(type_start, ".gen");
    else
      strcpy(type_start,".");
  }

  return ok;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  //  Save Intel Hex Format (IHF)
  //
  ////////////////////////////////////////////////////////////////////////////
  //
  //  :LLAAAATTDD.......DDSS
  //  ^^ ^   ^ ^          ^
  //  12 4   8 10         end-2  (field positions)
  //
  //  FIELD 1 (COLUMN 1)
  //
  //      a : symbol, each record uses a colon as a start delimiter.
  //
  //  FIELD 2 (COLUMN 2)
  //
  //      length byte, a two digit ASCII hex representation of the number
  //      of data bytes to be transmitted in this record.
  //
  //  FIELD 3 (COLUMN 4)
  //
  //      address word, a four digit ASCII hex representation of the target
  //      load address of the first byte of the following packet. This field
  //      is always 4 digits long, hence addresses are limited to 16 bits. See
  //      note later for more information.
  //
  //  FIELD 4 (COLUMN 8)
  //
  //      record type, a two digit ASCII hex representation of an integer which
  //      in turn denotes the record type.
  //      There are three record types 0,1 and 2
  //
  //      Record Type 0 - Normal Data packet
  //      Record Type 1 - End of file record. The last record in the file has
  //                      a type of 1.
  //      Record Type 2 - Address Extension Record. A special record type used
  //                      to extend addresses beyond 16 bits.
  //
  //  FIELD 5 (COLUMN 10)
  //
  //      start of data bytes. Data bytes in two digit ASCII form. Variable
  //      length field, usually a small power of 2 e.g. 16.
  //
  //
  //  FIELD 6 (LAST 2 COLUMNS)
  //
  //      checksum. A two digit ascii representation of a special checksum for
  //      the record. The checksum is the 2's complement of the true sum of all
  //      the bytes in the record, starting with the record length and ending
  //      with the last data byte. This allows limited error detection facilites
  //      for the data packet.
  //
  //  Sample file :
  //
  //  :020000020000FC
  //  :0600220018950895F89402
  //  :1000440000000000000000000000000000000000AC
  //  :0E00540000000000000000000000000000F7A7
  //  :00000001FF
  //

int saveIHF(void)
{
  int  c, i;
  FILE *out;
  int  count;
  int  line[16];
  int  sum;
  int  last;
  bool f64k = false;

  /// Save code output file //////////////////////////////////////////////////

  if(!yyoutfilename[0])
    return TRUE;

  if(! (out= fopen(yyoutfilename,"w")) )
    errorexit(X_UNABLE_TO_OPEN_OUTFILE, yyoutfilename);

  /// Address extension record, offset 0x0000 ////////////////////////////////

  if(yycfg -> addressext)
    fprintf(out, ":020000020000FC\n");

  /// Save data //////////////////////////////////////////////////////////////

  if(!yycodepos)
    message(M_EMPTY_CODE);

  c = 0;

  while(c<yycodepos)
  {
    count = 0;
    last  = 0;

    while(!yycodeusage[c])
      c++;

    while( (count<16) && !last )
    {
      if(yycodeusage[c+count]) {
        line[count] = yycodebuf[c+count];
        count++;
      }
      else
        last++;
    }

    sum = count + (c&0x00FF) + ((c&0xFF00)>>8);

    if (c >= 0x10000 && f64k == false)
    {
      fprintf(out, ":%02X%04X00",count,c);
      f64k = true;
    }

    fprintf(out, ":%02X%04X00",count,(c&0xFFFF));

    for(i=0;i<count;i++)
    {
      fprintf(out, "%02X",line[i]);
      sum+=line[i];
    }
    fprintf(out, "%02X\n",0x100-sum&0xFF);

    c+=count;
  }

  /// End of file record /////////////////////////////////////////////////////

  fprintf(out, ":00000001FF\n");
  fclose(out);

  /// Save rom output file ///////////////////////////////////////////////////

  if(!yyromfilename[0])
    return TRUE;

  if(!yyerompos && !yyeromposhigh)
  {
    message(M_EMPTY_ROM_FILE);
    return TRUE;
  }

  if(! (out= fopen(yyromfilename,"w")) )
    errorexit(X_UNABLE_TO_OPEN_ROMFILE, yyromfilename);

  /// Address extension record, offset 0x0000 ////////////////////////////////

  if(yycfg -> addressext)
    fprintf(out, ":020000020000FC\n");

  /// Save data //////////////////////////////////////////////////////////////

  c = 0;

  while(c<yyerompos)
  {
    count = 0;
    last  = 0;

    while(!yyeromusage[c])
      c++;

    while( (count<16) && !last )
    {
      if(yyeromusage[c+count])
        line[count++] = yyerombuf[c+count];
      else
        last++;
    }

    sum = count + (c&0x00FF) + ((c&0xFF00)>>8);

    fprintf(out, ":%02X%04X00",count,c);

    for(i=0;i<count;i++)
    {
      fprintf(out, "%02X",line[i]);
      sum+=line[i];
    }
    fprintf(out, "%02X\n",0x100-sum&0xFF);

    c+=count;
  }

  /// End of file record /////////////////////////////////////////////////////

  fprintf(out, ":00000001FF\n");
  fclose(out);

  return TRUE;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  //  Save generic format
  //

int saveGeneric(void)
{
  int  c;
  FILE *out;

  /// Save code output file //////////////////////////////////////////////////

  if(!yyoutfilename[0])
    return TRUE;

  if(! (out= fopen(yyoutfilename,"w")) )
    errorexit(X_UNABLE_TO_OPEN_OUTFILE, yyoutfilename);

  /// Save data //////////////////////////////////////////////////////////////

  if(!yycodepos)
    message(M_EMPTY_CODE);

  c = 0;

  for(c=0;c<yycodepos;c+=2)
    if(yycodeusage[c])
      fprintf(out, "%06x:%04x\n",c/2,yycodebuf[c]+yycodebuf[c+1]*0x100);

  fclose(out);

  /// Save rom output file ///////////////////////////////////////////////////

  if(!yyromfilename[0])
    return TRUE;

  if(!yyerompos && !yyeromposhigh)
  {
    message(M_EMPTY_ROM_FILE);
    return TRUE;
  }

  if(! (out= fopen(yyromfilename,"w")) )
    errorexit(X_UNABLE_TO_OPEN_ROMFILE, yyromfilename);

  /// Save data //////////////////////////////////////////////////////////////

  c = 0;

  for(c=0;c<yyerompos;c++)
    if(yyeromusage[c])
      fprintf(out, "%04x:%02x\n",c,yyerombuf[c]);

  fclose(out);

  return TRUE;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  //  Save ATMEL .obj format
  //

int saveObj(void)
{
  int  c;
  FILE *out;
  int offset=0;

  /// Save code output file //////////////////////////////////////////////////

  if(!yyoutfilename[0])
    return TRUE;

  if(! (out= fopen(yyoutfilename,"wb")) )
    errorexit(X_UNABLE_TO_OPEN_OUTFILE, yyoutfilename);

  if(yyromfilename[0])
    message(M_NOT_SAVE_EROM_OBJ);

  /// Get size of obj code (offset to filenames) /////////////////////////////

  if(!yycodepos)
    message(M_EMPTY_CODE);

  for(c=0;c<yycodepos;c+=2)
    if(yycodeusage[c]&&yycodeline[c])
      offset++;

  offset *= 9;
  offset += 0x1A;

  /// Save data //////////////////////////////////////////////////////////////

  fputc( (offset & 0xFF000000)>>24, out);
  fputc( (offset & 0x00FF0000)>>16, out);
  fputc( (offset & 0x0000FF00)>> 8, out);
  fputc( (offset & 0x000000FF)>> 0, out);

  fputc( ( 0x1A  & 0xFF000000)>>24, out);
  fputc( ( 0x1A  & 0x00FF0000)>>16, out);
  fputc( ( 0x1A  & 0x0000FF00)>> 8, out);
  fputc( ( 0x1A  & 0x000000FF)>> 0, out);

  fputc( 0x09, out);
  fputc( yyfilecount, out);
  fputs( "AVR Object File", out);
  fputc(0, out);

  for(c=0;c<yycodepos;c+=2)
  {
    if(yycodeusage[c]&&yycodeline[c])
    {
      fputc( (c/2    & 0x00FF0000)>>16, out);
      fputc( (c/2    & 0x0000FF00)>> 8, out);
      fputc( (c/2    & 0x000000FF)>> 0, out);
      fputc( (yycodebuf[c+1]   & 0x00FF) >> 0, out);
      fputc( (yycodebuf[c]     & 0x00FF) >> 0, out);
      fputc( (yycodeusage[c]-1 & 0x007F) >> 0, out);
      fputc( (yycodeline[c]    & 0xFF00) >> 8, out);
      fputc( (yycodeline[c]    & 0x00FF) >> 0, out);
      fputc( (yycodeusage[c]   & 0x0080) != 0, out);
    }
  }

  for(c=0;c<yyfilecount;c++)
  {
    fputs(yyfilelist[c], out);
    fputc(0, out);
  }

  fputc(0, out);

  fclose(out);

  return TRUE;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Save Motorola S-record
  //
  ////////////////////////////////////////////////////////////////////////////
  //
  //  Motorola S-Record File Format (8-bit Systems Only)
  //
  //  The Motorola S-Record File Format is a printable ASCII format consisting
  //  of an optional header record, and one or more data records followed by
  //  an end of file record. Data records may appear in any order. Values are
  //  represented as 2 or 4 hexadecimal digit values.
  //
  //  Record Format
  //  SNLLAAAADDDD......DDDDCC
  //
  //   S    Start of record mark (letter S).
  //
  //   N    Record type field. 0 for header, 1 for data, 9 for end of record.
  //        All other record types are ignored in 8-bit systems.
  //
  //   LL   Length field. Number of bytes to follow.
  //
  //   AAAA Address field. Address of first byte.
  //
  //   DD   Data field.
  //
  //   CC   Checksum field. One's complement of the length, address
  //        and data fields modulo 256 minus 1.
  //
  //  Motorola S records
  //
  //  The record length, address, code/data, and checksum fields are
  //  hexadecimal bytes coded in ASCII: 00 is the value 0, 01 the
  //  value 1, etc. The record length is the number of data bytes
  //  (two hex digits) including the address, code/data,
  //  and checksum fields. The checksum field contains the one's
  //  complement of the sum of all the bytes from the record
  //  length field through the end of the code/data field.
  //
  //  Each record can be terminated with either a carriage return, line feed,
  //  or a nul (zero) character.
  //
  //  The record types are as follows:
  //
  //  S0  Header record for each block of S records which may contain
  //      descriptive information identifying the following
  //      block of S records. The address field is typically zeros.
  //      The code/data field of an S0 record typically contains a
  //      "hexified" ASCII string.
  //
  //  S1  Code or data record containing a 2-byte address (4 hex digit addr.).
  //
  //  S2  Code or data record containing a 3-byte address (6 hex digit addr.).
  //
  //  S3  Code or data record containing a 4-byte address (8 hex digit addr.).
  //
  //  S5  Count record containing the number of S1, S2, and S3 records
  //      transmitted in a block. The count appears in the
  //      address field, and there is no code/data field.
  //
  //  S7  Termination record for a block of S records. A 4-byte address
  //      contains the address where execution starts.
  //      There is no code/data field.
  //
  //  S8  Termination record for a block of S records. A 3-byte address
  //      contains the address where execution starts.
  //      There is no code/data field.
  //
  //  S9  Termination record for a block of S records. A 2-byte address
  //      contains the address where execution starts.
  //      There is no code or data field.
  //

int saveMotorola(void)
{
  int  c, i;
  FILE *out;
  int  count;
  int  sum;
  int  last;
  int  line[20];

  /// Save code output file //////////////////////////////////////////////////

  if(!yyoutfilename[0])
    return TRUE;

  if(! (out= fopen(yyoutfilename,"w")) )
    errorexit(X_UNABLE_TO_OPEN_OUTFILE, yyoutfilename);

  /// Insert filename in header //////////////////////////////////////////////

  fprintf(out, "S0%02X0000",(int)strlen(yyoutfilename)+3);
  sum=strlen(yyoutfilename)+3;
  for(c=0;c<(int)strlen(yyoutfilename);c++)
  {
    fprintf(out, "%02X",yyoutfilename[c]);
    sum+=yyoutfilename[c];
  }
  fprintf(out, "%02X\n",0x100 - (sum&0xFF) - 1);

  /// Save data //////////////////////////////////////////////////////////////

  if(!yycodepos)
    message(M_EMPTY_CODE);

  c = 0;

  while(c<yycodepos)
  {
    count = 0;
    last  = 0;

    while(!yycodeusage[c])
      c++;

    while( (count<16) && !last )
    {
      if(yycodeusage[c+count])
        line[count++] = yycodebuf[c+count];
      else
        last++;
    }

    sum = count + (c&0x00FF) + ((c&0xFF00)>>8) + 3;

    fprintf(out, "S1%02X%04X",count+3,c);

    for(i=0;i<count;i++)
    {
      fprintf(out, "%02X",line[i]);
      sum+=line[i];
    }
    fprintf(out, "%02X\n",0x100-(sum&0xFF)-1 );

    c+=count;
  }

  /// End of file record /////////////////////////////////////////////////////

  fprintf(out, "S9030000FC\n");
  fclose(out);

  /// Save rom output file ///////////////////////////////////////////////////

  if(!yyromfilename[0])
    return TRUE;

  if(!yyerompos && !yyeromposhigh)
  {
    message(M_EMPTY_ROM_FILE);
    return TRUE;
  }

  if(! (out= fopen(yyromfilename,"w")) )
    errorexit(X_UNABLE_TO_OPEN_ROMFILE, yyromfilename);

  /// Insert filename in header //////////////////////////////////////////////

  fprintf(out, "S0%02X0000",(int)strlen(yyromfilename)+3);
  sum=strlen(yyromfilename)+3;
  for(c=0;c<(int)strlen(yyromfilename);c++)
  {
    fprintf(out, "%02X",yyromfilename[c]);
    sum+=yyromfilename[c];
  }

  fprintf(out, "%02X\n",0x100 - (sum&0xFF) - 1);

  /// Save data //////////////////////////////////////////////////////////////

  c = 0;

  while(c<yyerompos)
  {
    count = 0;
    last  = 0;

    while(!yyeromusage[c])
      c++;

    while( (count<16) && !last )
    {
      if(yyeromusage[c+count])
        line[count++] = yyerombuf[c+count];
      else
        last++;
    }

    sum = count + (c&0x00FF) + ((c&0xFF00)>>8) + 3;

    fprintf(out, "S1%02X%04X",count+3,c);

    for(i=0;i<count;i++)
    {
      fprintf(out, "%02X",line[i]);
      sum+=line[i];
    }
    fprintf(out, "%02X\n",0x100-(sum&0xFF)-1 );

    c+=count;
  }

  /// End of file record /////////////////////////////////////////////////////

  fprintf(out, "S9030000FC\n");
  fclose(out);

  return TRUE;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  //  Save binary format
  //

int saveBin(void)
{
  int  c;
  FILE *out;

  /// Save code output file //////////////////////////////////////////////////

  if(!yyoutfilename[0])
    return TRUE;

  if(! (out= fopen(yyoutfilename,"wb")) )
    errorexit(X_UNABLE_TO_OPEN_OUTFILE, yyoutfilename);

  /// Save data //////////////////////////////////////////////////////////////

  if(!yycodepos)
    message(M_EMPTY_CODE);

  c = 0;

  for(c=0;c<yycodepos;c++)
    fputc(yycodebuf[c], out);

  fclose(out);

  /// Save rom output file ///////////////////////////////////////////////////

  if(!yyromfilename[0])
    return TRUE;

  if(!yyerompos && !yyeromposhigh)
  {
    message(M_EMPTY_ROM_FILE);
    return TRUE;
  }

  if(! (out= fopen(yyromfilename,"wb")) )
    errorexit(X_UNABLE_TO_OPEN_ROMFILE, yyromfilename);

  /// Save data //////////////////////////////////////////////////////////////

  c = 0;

  for(c=0;c<yyerompos;c++)
    fputc(yyerombuf[c], out);

  fclose(out);

  return TRUE;
}

/// END OF FILE //////////////////////////////////////////////////////////////

