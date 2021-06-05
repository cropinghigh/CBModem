//////////////////////////////////////////////////////////////////////////////
//
//  File        : utils.cc
//
//  Author      : Tom Mortensen - Copyright (C) 1999
//
//  Description : Implements functions for logging, error reporting etc.
//
//  History
//  =======================================================================
//
//  980902      : Tom - File created.
//  990117      : Tom - Fixed alignment of .db in CSEG with forward refs.
//  990124      : Tom - Added GPL notice.
//  990329      : Tom - Fixed problem with tabs after include filename
//  991212      : Tom - Fixed promlem with detecting supported instructions
//                      (Introduced in 1.10)
//  001101	: Brian Rhodefer - Added "fopenInIncpath()" 
//  010403      : Tom - Fixed path delimiter problem in include path
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

#define _UTILS_CC_

/// Include //////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include "messages.hh"
#include "avrasm.hh"
#include "utils.hh"
#include "symbol.hh"
#include "semantic.hh"

/// Externals ////////////////////////////////////////////////////////////////

GLOBALS(extern);

/// Defines //////////////////////////////////////////////////////////////////

#define SEP     yyinmacro?'>':'|'
#define CUT     if( yycfg -> cutlog )        \
                {                            \
                  strcat(yyinlineold, sp );  \
                  yyinlineold[58] = '|';     \
                  yyinlineold[59] = '\0';    \
                }

/// Functions ////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////////
  //
  // Get symbol
  //

symbolTableEntry* getsym(char *name)
{
  int                   c;
  symbolTableEntry      *symb = NULL;

  // Is it a macro def

  for(c=yycontext; c>=0 ; c--)
    if( yysymbolstack[c] && (symb=yysymbolstack[c]->get(name)) && symb->macro)
      return symb;

  // No it was not

  c    = yycontext;
  symb = NULL;

  if(!yyparseno)
  {
    while(!yysymbolstack[c])
      c--;

    symb = yysymbolstack[c]->get(name);
    if(!symb)
      symb = yysymbolstack[c]->add(name);
  }
  else
  {
    while( (c>=0) && !symb )
    {
      if( yysymbolstack[c--] )
      {
        symb = yysymbolstack[c+1]->get(name);
        if(! (symb && ISUSED(symb)) )
          symb = NULL;
      }
    }
    if(!symb)
    {
      c = yycontext;
      while( (c>=0) && !symb )
      {
        if( yysymbolstack[c--] )
          symb = yysymbolstack[c+1]->get(name);
      }
    }
  }

  if(!symb)
    internalerror("GS %s",name);

  return symb;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Insert code i CSEG
  //

void insertcode(int val)
{
  if(yycodepos>=MAX_CODE_SIZE)
    errorexit(X_TOO_MUCH_CODE);

  yycodebuf[yycodepos]     = val;
  yycodeusage[yycodepos]   = yyfileno|0x80*(ISMACRO!=false);
  yycodeline[yycodepos++]  = yyline-1+(ISMACRO!=0);
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Insert .db or .dw in CSEG
  //

void insertdbdw(int val)
{
  if(yycodepos>=MAX_CODE_SIZE)
    errorexit(X_TOO_MUCH_CODE);

  yycodebuf[yycodepos]     = val;
  yycodeusage[yycodepos]   = yyfileno|0x80*(ISMACRO!=false);
  yycodeline[yycodepos++]  = yyline;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Insert data in ESEG
  //

void insertdata(int val)
{
  if(yyerompos>=MAX_EROM_SIZE)
    errorexit(X_TOO_MUCH_DATA);

  yyerombuf[yyerompos]     = val;
  yyeromusage[yyerompos++] = yyfileno;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Check if instruction is supported with the current device
  //

#define NOSUP(_inst_) (DEVINF.supported & (_inst_))

void checkSupported(int opcode)
{
  switch(opcode)
  {
    case O_ADIW   : if(NOSUP(S_ADIW  )) warning(W_UNSUP,"adiw");   break;
    case O_ICALL  : if(NOSUP(S_ICALL )) warning(W_UNSUP,"icall");  break;
    case O_CALL   : if(NOSUP(S_CALL  )) warning(W_UNSUP,"call");   break;
    case O_IJMP   : if(NOSUP(S_IJMP  )) warning(W_UNSUP,"ijmp");   break;
    case O_JMP    : if(NOSUP(S_JMP   )) warning(W_UNSUP,"jmp");    break;
    case O_LD     : if(NOSUP(S_LD    )) warning(W_UNSUP,"ld");     break;
    case O_LDD    : if(NOSUP(S_LDD   )) warning(W_UNSUP,"ldd");    break;
    case O_LDS    : if(NOSUP(S_LDS   )) warning(W_UNSUP,"lds");    break;
    case O_LPM    : if(NOSUP(S_LPM   )) warning(W_UNSUP,"lpm");    break;
    case O_MUL    : if(NOSUP(S_MUL   )) warning(W_UNSUP,"mul");    break;
    case O_PUSH   : if(NOSUP(S_PUSH  )) warning(W_UNSUP,"push");   break;
    case O_POP    : if(NOSUP(S_POP   )) warning(W_UNSUP,"pop");    break;
    case O_SBIW   : if(NOSUP(S_SBIW  )) warning(W_UNSUP,"sbiw");   break;
    case O_ST     : if(NOSUP(S_ST    )) warning(W_UNSUP,"st");     break;
    case O_STD    : if(NOSUP(S_STD   )) warning(W_UNSUP,"std");    break;
    case O_STS    : if(NOSUP(S_STS   )) warning(W_UNSUP,"sts");    break;
    case O_EIJMP  : if(NOSUP(S_EIJMP )) warning(W_UNSUP,"eijmp");  break;
    case O_EICALL : if(NOSUP(S_EICALL)) warning(W_UNSUP,"eicall"); break;
    case O_ESPM   : if(NOSUP(S_ESPM  )) warning(W_UNSUP,"espm");   break;
    case O_MULSU  : if(NOSUP(S_MULSU )) warning(W_UNSUP,"mulsu");  break;
    case O_FMUL   : if(NOSUP(S_FMUL  )) warning(W_UNSUP,"fmul");   break;
    case O_FMULS  : if(NOSUP(S_FMULS )) warning(W_UNSUP,"fmuls");  break;
    case O_FMULSU : if(NOSUP(S_FMULSU)) warning(W_UNSUP,"fmulsu"); break;
    case O_MULS   : if(NOSUP(S_MULS  )) warning(W_UNSUP,"muls");   break;
    case O_MOVW   : if(NOSUP(S_MOVW  )) warning(W_UNSUP,"movw");   break;
    case O_ELPM   : if(NOSUP(S_ELPM  )) warning(W_UNSUP,"elpm");   break;
    case O_SPM    : if(NOSUP(S_SPM   )) warning(W_UNSUP,"spm");    break;
  }
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Insert instruction into code segment
  //

void insertInst(int v, int s)
{
  if(yysegment==SEGMENT_DATA)
    error(E_PROGRAM_IN_DSEG);
  else if(yysegment==SEGMENT_EEPROM)
    error(E_PROGRAM_IN_DSEG);
  else
  {
    if(s==1)
    {
      insertcode( (v & 0x000000FF) >>  0);
    }
    else if(s==2)
    {
      insertcode( (v & 0x000000FF) >>  0);
      insertcode( (v & 0x0000FF00) >>  8);
    }
    else if(s==4)
    {
      insertcode( (v & 0x000000FF) >>  0);
      insertcode( (v & 0x0000FF00) >>  8);
      insertcode( (v & 0x00FF0000) >> 16);
      insertcode( (v & 0xFF000000) >> 24);
    }
    else
      internalerror("GG %08X %08X",v,s);

    if(s==1)
      sprintf(yylinetxt,"?????? %02X%02X     ",
              yycodebuf[yycodepos-1], yycodebuf[yycodepos-2]);
    else if(s==2)
      sprintf(yylinetxt,"%06X %02X%02X     ", (yycodepos-2)/2,
              yycodebuf[yycodepos-1], yycodebuf[yycodepos-2]);
    else if(s==4)
      sprintf(yylinetxt,"%06X %02X%02X %02X%02X", (yycodepos-4)/2,
      yycodebuf[yycodepos-3], yycodebuf[yycodepos-4],
      yycodebuf[yycodepos-1], yycodebuf[yycodepos-2]);
    else
      internalerror("PH %08X %08X",v,s);
  }
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Report warning at end of line
  //

int warning(int warnno, ...)
{
  va_list  args;
  char     errtext[MAX_LINE_LENGTH+1];

  if(!yyparseno || !warnno)
    return false;

  yywarningcount++;

  if(!yycfg -> warnings)
    return false;

  if( (yywarningline==yyline-!ISMACRO) && !ISMACRO )
    return false;

  yywarningline = yyline-!ISMACRO;

  if( (warnno<1) || (warnno>MESSAGES_WARNING) )
    internalerror("WW %04X", warnno);

  va_start(args, warnno);
  vsprintf(errtext, messages[warnno], args);
  va_end(args);

  if(yyinmacro)
    printf("%s:%i: Warning in macro : %s\n",yyfilename,
                                            yyline-!ISMACRO,errtext);
  else
    printf("%s:%i: Warning : %s\n",yyfilename,yyline-!ISMACRO,errtext);

  if(yylogfile)
  {
    if(yyinmacro)
      fprintf(yylogfile, "%s:%i: Warning in macro : %s\n",yyfilename,
                                              yyline-!ISMACRO,errtext);
    else
      fprintf(yylogfile, "%s:%i: Warning : %s\n",yyfilename,
              yyline-!ISMACRO,errtext);
  }

  STRIPNR(yyinlineold);

  printf("%s:%i: \'%s\'\n",yyfilename,yyline-!ISMACRO,yyinlineold);

  return true;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Report warning inline
  //

int warningin(int warnno, ...)
{
  va_list  args;
  char     errtext[MAX_LINE_LENGTH+1];

  if(!yyparseno || !warnno)
    return false;

  yywarningcount++;

  if(!yycfg -> warnings)
    return false;

  if( (yywarningline==yyline) && !ISMACRO )
    return false;

  yywarningline = yyline;

  if( (warnno<1) || (warnno>MESSAGES_WARNING) )
    internalerror("WI %04X", warnno);

  va_start(args, warnno);
  vsprintf(errtext, messages[warnno], args);
  va_end(args);

  if(yyinmacro)
    printf("%s:%i: Warning in macro : %s\n",yyfilename,yyline,errtext);
  else
    printf("%s:%i: Warning : %s\n",yyfilename,yyline,errtext);

  if(yylogfile)
  {
    if(yyinmacro)
      fprintf(yylogfile, "%s:%i: Warning in macro : %s\n",yyfilename,
                                                          yyline,errtext);
    else
      fprintf(yylogfile, "%s:%i: Warning : %s\n",yyfilename,yyline,errtext);
  }

  STRIPNR(yyinline);

  printf("%s:%i: \'%s\'\n",yyfilename,yyline,yyinline);

  return true;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Report error at end of line
  //

int error(int errorno, ...)
{
  va_list  args;
  char     errtext[MAX_LINE_LENGTH+1];

  if(!yyparseno)
    return false;

  if( (yyerrorline == yyline- !ISMACRO) && !ISMACRO )
    return false;

  yyerrorline = yyline - !ISMACRO;

  yyerrorcount++;

  if( (errorno<1) || (errorno>MESSAGES_ERROR) )
    internalerror("EE %04X", errorno);

  errorno += MESSAGES_WARNING;

  va_start(args, errorno);
  vsprintf(errtext, messages[errorno], args);
  va_end(args);

  if(yyinmacro)
    printf("%s:%i: Error in macro : %s\n",yyfilename,yyline-!ISMACRO,errtext);
  else
    printf("%s:%i: Error : %s\n",yyfilename,yyline-!ISMACRO,errtext);

  if(yylogfile)
  {
    if(yyinmacro)
      fprintf(yylogfile, "%s:%i: Error in macro : %s\n",yyfilename,
              yyline-!ISMACRO,errtext);
    else
      fprintf(yylogfile, "%s:%i: Error : %s\n",yyfilename,
              yyline-!ISMACRO,errtext);
  }

  STRIPNR(yyinlineold);

  printf("%s:%i: \'%s\'\n",yyfilename,yyline-!ISMACRO,yyinlineold);

  return true;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Report error inline
  //

int errorin(int errorno, ...)
{
  va_list  args;
  char     errtext[MAX_LINE_LENGTH+1];

  if(!yyparseno)
    return false;

  if( (yyerrorline == yyline) && !ISMACRO )
    return false;

  yyerrorline = yyline;

  yyerrorcount++;

  if( (errorno<1) || (errorno>MESSAGES_ERROR) )
    internalerror("EI %04X", errorno);

  errorno += MESSAGES_WARNING;

  va_start(args, errorno);
  vsprintf(errtext, messages[errorno], args);
  va_end(args);

  if(yyinmacro)
    printf("%s:%i: Error in macro : %s\n",yyfilename,yyline,errtext);
  else
    printf("%s:%i: Error : %s\n",yyfilename,yyline,errtext);

  STRIPNR(yyinline);

  printf("%s:%i: \'%s\'\n",yyfilename,yyline,yyinline);

  if(yylogfile)
  {
    if(yyinmacro)
      fprintf(yylogfile, "%s:%i: Error in macro : %s\n",yyfilename,yyline,errtext);
    else
      fprintf(yylogfile, "%s:%i: Error : %s\n",yyfilename,yyline,errtext);
  }

  return true;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Print a message
  //

int message(int msgno, ...)
{
  va_list  args;
  char     msgtext[MAX_LINE_LENGTH+1];

  msgno += MESSAGES_WARNING + MESSAGES_ERROR + MESSAGES_FATAL + 2;

  va_start(args, msgno);
  vsprintf(msgtext, messages[msgno], args);
  va_end(args);

  printf("%s\n",msgtext);

  return true;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Report fatal error and exit(1)
  //

void errorexit(int errorno, ...)
{
  va_list  args;
  char     errtext[MAX_LINE_LENGTH+1];

  if( (errorno<1) || (errorno>MESSAGES_FATAL) )
    internalerror("EX %04X", errorno);

  errorno += MESSAGES_WARNING + MESSAGES_ERROR + 1;

  va_start(args, errorno);
  vsprintf(errtext, messages[errorno], args);
  va_end(args);

  printf("Program terminated - %s\n",errtext);
  exit(1);
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Report internal error and exit(2)
  //

void internalerror(char const *fmt, ...)
{
  va_list  args;
  char     errtext[MAX_LINE_LENGTH+1];

  va_start(args, fmt);
  vsprintf(errtext, fmt, args);
  va_end(args);

  sprintf(errtext+strlen(errtext)," %s",VERSIONSTR);

  while(strlen(errtext)<57)
    strcat(errtext," ");

  printf("\n");
  printf("   ************************************************************\n");
  printf("   *                                                          *\n");
  printf("   *                Internal compiler error !!!               *\n");
  printf("   *                                                          *\n");
  printf("   * Please note the error text and report to Tom :           *\n");
  printf("   *                                                          *\n");
  printf("   * %s*\n",errtext);
  printf("   *                                                          *\n");
  printf("   ************************************************************\n");
  exit(2);
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Add line to log file
  //

void tolog(void)
{
  int  c;
  char temp[MAX_LINE_LENGTH+100];
  char sp[]="                                                           |\n";
  static int rompos  = 0;
  static int codepos = 0;
  static int rampos  = 0;

  // Check alignment /////////////////////////////////////////////////////////

  if( (yysegment==SEGMENT_CODE) && (yycodepos&1) && (yycfg->aligndb) ) {
    yyline-=(ISMACRO==0);
    insertdbdw(0);
    yyline+=(ISMACRO==0);
  }

  // Return if first parse ///////////////////////////////////////////////////

  if(!yyparseno)
    return;

  // Check if eeprom/flash size has been exceeded ////////////////////////////

  if( (yycodepos > DEVINF.flashsize*2) && (yycodepos != codepos) ) {
    warning(W_FLASH_EXCEEDED,(yycodepos-DEVINF.flashsize*2)/2);
    codepos = yycodepos;
  }

  if( (yyerompos > DEVINF.eepromsize) && (yyerompos!=rompos) ) {
    warning(W_EROM_EXCEED,yyerompos-DEVINF.eepromsize);
    rompos = yyerompos;
  }

  if( (yydatapos > DEVINF.datastart+DEVINF.ramsize) && (yydatapos!=rampos)) {
    warning(W_SRAM_EXCEED,yydatapos-(DEVINF.datastart+DEVINF.ramsize));
    rampos = yydatapos;
  }

  // Clean-up log line ///////////////////////////////////////////////////////

  STRIPNR(yyinlineold);
  exptabs(yyinlineold);

  CUT;

  /// Check if this line was a '.db' or '.dw' ////////////////////////////////

  if(yydefinestart!=-1)
  {
    if(yysegment == SEGMENT_CODE)
    {
      if( (yycodepos & 1) && ( yycfg -> aligndb ) )
      {
        yyline-=(ISMACRO==0);
        insertdbdw(0);
        yyline+=(ISMACRO==0);
      }

      if(!ISMACRO && (yycodeline[yycodepos-2]==yyline) )
        yycodeline[yycodepos-2]--;

      if(yylist && yylogfile && !(yyinmacro && !yylistmacro))
      {
        sprintf(temp,"%06X ", yydefinestart/2);

        for(c=0;c<(yycodepos-yydefinestart)&3;c++)
        {
          sprintf(temp+strlen(temp),"%02X",yycodebuf[yydefinestart+c]);
          if(c&1)
            strcat(temp," ");
        }

        strcat(temp,"                      ");
        temp[16] = '\0';

        CUT;

        fprintf(yylogfile,"| %s %c %s\n",temp,SEP,yyinlineold);

        if(yycodepos-yydefinestart>3)
        {
          for(c=0;c< (yycodepos-yydefinestart)-4;c++)
          {
            if(c%4 == 0)
              sprintf(temp,"%06X ",(c+4+yydefinestart)/2);

            sprintf(temp+strlen(temp),"%02X",yycodebuf[yydefinestart+c+4]);
            if(c&1)
              strcat(temp," ");
            if( (c%4 == 3) && c)
            {
              strcat(temp,"                      ");
              temp[16] = '\0';

              fprintf(yylogfile,"| %s %c",temp,SEP);
              if( yycfg -> cutlog)
                fprintf(yylogfile, sp);
              else
                fprintf(yylogfile,"\n");
            }
          }
        }
        if( (yycodepos-yydefinestart>4) && (c%4 != 0) )
        {
          strcat(temp,"                      ");
          temp[16] = '\0';
          fprintf(yylogfile,"| %s %c",temp,SEP);
          if( yycfg -> cutlog)
            fprintf(yylogfile, sp);
          else
            fprintf(yylogfile,"\n");
        }
      }
    }
    else if(yysegment == SEGMENT_EEPROM)
    {
      if(yylist && yylogfile && !(yyinmacro && !yylistmacro))
      {
        if(yydefinetype!=SEGMENT_DEFINE_BYTE)
        {
          c = yydefinestart;
          CUT;
          fprintf(yylogfile,"| %06X %02X EEPROM %c %s\n", c,
                    yyerombuf[c],SEP,yyinlineold);
          for(c++;c<yyerompos;c++)
          {
            fprintf(yylogfile,"| %06X %02X EEPROM %c", c, yyerombuf[c], SEP);
            if( yycfg -> cutlog)
              fprintf(yylogfile, sp);
            else
              fprintf(yylogfile,"\n");
          }
        }
        else
        {
          CUT;
          fprintf(yylogfile, "| %06X    EEPROM %c %s\n",yydefinestart, 
		  SEP,yyinlineold);
          yydefinetype = 0;
        }
      }
    }
    else if(yysegment == SEGMENT_DATA)
    {
      if(yylist && yylogfile && !(yyinmacro && !yylistmacro))
      {
        c = yydefinestart;
        CUT;
        fprintf(yylogfile, "| %06X    -DATA- %c %s\n", c, SEP,yyinlineold);
      }
    }
    else
      internalerror("LI %04X",yysegment);
  }

  /// No it is a regular instruction /////////////////////////////////////////

  else
  {
    if(yyinmacro)
    {
      if((yylistmacro||yyfirstmacroline) && yylist && yylogfile)
      {
        if( strlen(yylinetxt) )
        {
          CUT;
          fprintf(yylogfile,"| %s > %s\n",yylinetxt,yyinlineold);
        }
        else if(yyfirstmacroline)
        {
          CUT;
          fprintf(yylogfile,"| %06X           > %s\n",yycodepos/2,yyinlineold);
        }
        else
        {
          CUT;
          fprintf(yylogfile,"|                  > %s\n",yyinlineold);
        }
      }
    }
    else
    {
      if(yylist && yylogfile)
      {
        if( strlen(yylinetxt) )
        {
          CUT;
          fprintf(yylogfile, "| %s | %s\n",yylinetxt,yyinlineold);
        }
        else if(!(yyinmacrodef && !yylistmacro))
        {
          CUT;
          fprintf(yylogfile, "|                  | %s\n",yyinlineold);
        }
      }
    }
  }

  yylinetxt[0]     = 0;
  yydefinestart    = -1;
  yyfirstmacroline = false;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Strip comment from line
  //

int striprem(char *str)
{
  int   l   = 0;
  int   rem = false;
  char  *pos;

  if( (pos=strpbrk(str,"\r\n")) )
    *pos = '\0';

  while(str[l])
  {
    if( (str[l]=='"') || (str[l]=='\'') )
      rem ? rem = false : rem = true;
    else if( str[l]=='\\' )
    {
      if(str[l+1])
        l++;
    }
    else if ( (str[l]==';') && !rem)
      str[l--] = '\0';
    l++;
  }

  l = strlen(str);

  if(l)
    while(--l, ((str[l]==' ')||(str[l]=='\t')))
      str[l] = '\0';

  return l;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Expand tabs in string
  //

int exptabs(char *str)
{
  int   l   = strlen(str);
  int   c   = 0;
  char  *temp;

  /// Expand tabs ///////////////////////////////////////////////////////////

  if(l)
  {
    if(!(temp = new char[strlen(str)+1]))
      errorexit(X_OUT_OF_MEMORY);

    strcpy(temp, str);
    c = l = 0;
    while(temp[l] && (c<(MAX_LINE_LENGTH-10)))
    {
      if(temp[l]=='\t')
      {
        if(temp[l]=='\t')
          str[c++]=' ';
        while((c%8)!=0)
          str[c++]=' ';
        l++;
        if(temp[l]=='\t')
          str[c++]=' ';
      }
      else
        str[c++]=temp[l++];
    }
    str[c] = '\0';
    delete temp;
  }
  return c;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Get file.name from "file.name" or <file.name>
  //

int getfilename(char *name, char *newname)
{

  if( (name[0]!='"') || (name[strlen(name)-1]!='"') )
  {
    if( (name[0]!='<') || (name[strlen(name)-1]!='>') )
      return false;
    else
    {
      memmove(name,name+1,strlen(name)-1);
      name[strlen(name)-2] = 0;

      // Search include tree here - XXX

      strcpy(newname, name);
    }

    if(!strlen(newname))
      return false;
  }
  else
  {
    memcpy(newname,name+1,strlen(name)-1);
    newname[strlen(name)-2] = 0;
  }

  if(strlen(newname)>MAX_FILENAME_LENGTH)
  {
    errorin(E_FILENAME_TOO_LONG);
    return false;
  }

  return true;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Check that filename is valid
  //

int   checkfilename(char *name)
{
  return 1;

  int c         = strlen(name);
  int valid     = true;

  while(name[c])
    if( ! isalnum(name[c++]) )
      valid = false;

  return valid;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Open a file somewhere in the "include path".  Try in the current dir first.
  //

FILE * fopenInIncpath(const char *name, const char *mode)
{
  FILE *fp = fopen(name, mode);

  if(fp)
    return fp;

  if (MAX_FILENAME_LENGTH <= strlen(name)) 
    errorin(E_FILENAME_TOO_LONG);
  else
  {
    int c;

    for(c=0; !fp && (c<yyIncPathCount); c++) {
      char *path  = yyIncludePaths[c];
      if(strlen(path)>MAX_FILENAME_LENGTH)
        continue;
      char *slash = strrchr(path, '/');
      char file[2 * MAX_FILENAME_LENGTH + 3];
      sprintf(file, "%s%s%s", path, (slash && !slash[1]) ? "" : "/", name);
      fp = fopen(file, mode);
    }
  }

  return fp;
}

/// END OF FILE //////////////////////////////////////////////////////////////

