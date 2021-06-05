%{ // -*- C -*-
//////////////////////////////////////////////////////////////////////////////
//
//  File        : avrlex.l
//
//  Author      : Tom Mortensen - Copyright (C) 1999
//
//  Description : Scanner file for AVR assembler
//
//  History
//  ========================================================================
//
//  980902      : Tom - File created.
//  990124      : Tom - Added GPL notice.
//  990329      : Tom - Added support for hex values like 1AB without $ or 0x
//  990512      : Tom - Fixed detection on binary numbers.
//  990522      : Tom - Added support for PC/DC/EC.   
//  991211      : Tom - Fixed .endm problem (it had to be lower case)
//  991217      : Kurt- Added # and support for local labels
//  001101	: Brian - Added support for multiple include search paths
//  010319      : Timothy Lee - Added byte1 function (sames as low())
//  041212      : Tom - Fixed problem with .db 00
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

/// Include //////////////////////////////////////////////////////////////////

#include <string.h>
#include <ctype.h>
#include "symbol.hh"
#include "semantic.hh"
#include "avrparse.hh"
#include "avrasm.hh"
#include "utils.hh"

/// Extern ///////////////////////////////////////////////////////////////////

GLOBALS(extern);

/// Global ///////////////////////////////////////////////////////////////////

symbolTableEntry       *sym;
YY_BUFFER_STATE        statestack[MAX_CONTEXT_DEPTH];
int                    restartcount = 0;
int                    ret;

/// Defines //////////////////////////////////////////////////////////////////

#define SKIPRESTART 0x02BABE  // Any value that is not a token

/// Prototypes for support function //////////////////////////////////////////

int     restart(void);
int     endofline(void);
int     identifier(void);
int     registers(void);
int     integers(void);
int     macrodef(void);
int     includefile(void);
int     string(void);
int     character(void);
int     doexit(void);
int     macro(symbolSA symb);
void    list(int listtype);
void    illegal(void);

/// Scanner definition ///////////////////////////////////////////////////////

%}

%option noyywrap
%option never-interactive
%option caseless

DECDIGIT          [0-9]
HEXDIGIT          [0-9a-fA-F]
OCTDIGIT          [0-7]
BINDIGIT          [01]
LETTER            [a-zA-Z_]
CHAR              [a-zA-Z0-9_]
HTAB              [\011]
NL                [\012]
VTAB              [\013]
FORMFEED          [\014]
CR                [\015]
SPACE             [\040]
WHITE             {HTAB}|{VTAB}|{SPACE}

%%

"*"       return STAR;
"/"       return DIV;
"%"       return MOD;
"<<"      return LS;
">>"      return RS;
"<="      return LE;
">="      return GE;
"<"       return LESS;
">"       return GREAT;
"=="      return EQ;
"!="      return NE;
"||"      return OROR;
"&&"      return ANDAND;
"|"       return OR;
"&"       return AND;
"^"       return XOR;
"("       return LPAR;
")"       return RPAR;
":"       return COLON;
","       return COMMA;
"."       return DOT;
"+"       return PLUS;
"-"       return MINUS;
"="       return EQUAL;
"~"       return WAVE;
"!"       return NOT;

".def"    return DEF;
".equ"    return EQU;
".eq"     return EQU;
".db"     return DB;
".dw"     return DW;
".org"    return ORG;
".align"  return ALIGN;
".byte"   return BYTE;
".set"    return SET;
".device" return DEVICE;

".cseg"  { yysegment = SEGMENT_CODE;   return CSEG;  }
".dseg"  { yysegment = SEGMENT_DATA;   return DSEG;  }
".eseg"  { yysegment = SEGMENT_EEPROM; return ESEG;  }

"low"    { yylval.func   = OP_LOW;   return FUNCTION;}
"high"   { yylval.func   = OP_HIGH;  return FUNCTION;}
"byte1"  { yylval.func   = OP_LOW;   return FUNCTION;}
"byte2"  { yylval.func   = OP_BYTE2; return FUNCTION;}
"byte3"  { yylval.func   = OP_BYTE3; return FUNCTION;}
"byte4"  { yylval.func   = OP_BYTE4; return FUNCTION;}
"lwrd"   { yylval.func   = OP_LWRD;  return FUNCTION;}
"hwrd"   { yylval.func   = OP_HWRD;  return FUNCTION;}
"page"   { yylval.func   = OP_PAGE;  return FUNCTION;}
"exp2"   { yylval.func   = OP_EXP2;  return FUNCTION;}
"log2"   { yylval.func   = OP_LOG2;  return FUNCTION;}
"cpc"    { yylval.opcode = O_CPC;    return IREGREG; } /* Register */
"cp"     { yylval.opcode = O_CP;     return IREGREG; }
"sbc"    { yylval.opcode = O_SBC;    return IREGREG; }
"sub"    { yylval.opcode = O_SUB;    return IREGREG; }
"add"    { yylval.opcode = O_ADD;    return IREGREG; }
"adc"    { yylval.opcode = O_ADC;    return IREGREG; }
"cpse"   { yylval.opcode = O_CPSE;   return IREGREG; }
"and"    { yylval.opcode = O_AND;    return IREGREG; }
"eor"    { yylval.opcode = O_EOR;    return IREGREG; }
"or"     { yylval.opcode = O_OR;     return IREGREG; }
"mov"    { yylval.opcode = O_MOV;    return IREGREG; }
"movw"   { yylval.opcode = O_MOVW;   return IREGREGW; }
"mul"    { yylval.opcode = O_MUL;    return IREGREG; }
"muls"   { yylval.opcode = O_MULS;   return IREGREG; }
"mulsu"  { yylval.opcode = O_MULSU;  return IREGREG; }
"fmul"   { yylval.opcode = O_FMUL;   return IREGREG; }
"fmuls"  { yylval.opcode = O_FMULS;  return IREGREG; }
"fmulsu" { yylval.opcode = O_FMULSU; return IREGREG; }
"adiw"   { yylval.opcode = O_ADIW;   return IREGIMMW; } /* Reg/Imm */
"sbiw"   { yylval.opcode = O_SBIW;   return IREGIMMW; }
"cpi"    { yylval.opcode = O_CPI;    return IREGIMM; }
"sbci"   { yylval.opcode = O_SBCI;   return IREGIMM; }
"subi"   { yylval.opcode = O_SUBI;   return IREGIMM; }
"ori"    { yylval.opcode = O_ORI;    return IREGIMM; }
"andi"   { yylval.opcode = O_ANDI;   return IREGIMM; }
"ldi"    { yylval.opcode = O_LDI;    return IREGIMM; }
"lds"    { yylval.opcode = O_LDS;    return IREGIMM; }
"sbr"    { yylval.opcode = O_SBR;    return IREGIMM; }
"brcc"   { yylval.opcode = O_BRCC;   return IIMM;    } /* Immediate */
"brcs"   { yylval.opcode = O_BRCS;   return IIMM;    }
"brne"   { yylval.opcode = O_BRNE;   return IIMM;    }
"breq"   { yylval.opcode = O_BREQ;   return IIMM;    }
"brpl"   { yylval.opcode = O_BRPL;   return IIMM;    }
"brmi"   { yylval.opcode = O_BRMI;   return IIMM;    }
"brvc"   { yylval.opcode = O_BRVC;   return IIMM;    }
"brvs"   { yylval.opcode = O_BRVS;   return IIMM;    }
"brge"   { yylval.opcode = O_BRGE;   return IIMM;    }
"brlt"   { yylval.opcode = O_BRLT;   return IIMM;    }
"brhc"   { yylval.opcode = O_BRHC;   return IIMM;    }
"brhs"   { yylval.opcode = O_BRHS;   return IIMM;    }
"brtc"   { yylval.opcode = O_BRTC;   return IIMM;    }
"brts"   { yylval.opcode = O_BRTS;   return IIMM;    }
"brid"   { yylval.opcode = O_BRID;   return IIMM;    }
"brie"   { yylval.opcode = O_BRIE;   return IIMM;    }
"brsh"   { yylval.opcode = O_BRSH;   return IIMM;    }
"brlo"   { yylval.opcode = O_BRLO;   return IIMM;    }
"neg"    { yylval.opcode = O_NEG;    return IREG;    } /* Register */
"swap"   { yylval.opcode = O_SWAP;   return IREG;    }
"inc"    { yylval.opcode = O_INC;    return IREG;    }
"asr"    { yylval.opcode = O_ASR;    return IREG;    }
"lsr"    { yylval.opcode = O_LSR;    return IREG;    }
"dec"    { yylval.opcode = O_DEC;    return IREG;    }
"pop"    { yylval.opcode = O_POP;    return IREG;    }
"push"   { yylval.opcode = O_PUSH;   return IREG;    }
"clr"    { yylval.opcode = O_CLR;    return IREG;    }
"tst"    { yylval.opcode = O_TST;    return IREG;    }
"com"    { yylval.opcode = O_COM;    return IREG;    }
"ror"    { yylval.opcode = O_ROR;    return IREG;    }
"sec"    { yylval.opcode = O_SEC;    return INOARGS; } /* No arguments */
"sez"    { yylval.opcode = O_SEZ;    return INOARGS; }
"sen"    { yylval.opcode = O_SEN;    return INOARGS; }
"sev"    { yylval.opcode = O_SEV;    return INOARGS; }
"ses"    { yylval.opcode = O_SES;    return INOARGS; }
"seh"    { yylval.opcode = O_SEH;    return INOARGS; }
"set"    { yylval.opcode = O_SET;    return INOARGS; }
"sei"    { yylval.opcode = O_SEI;    return INOARGS; }
"clc"    { yylval.opcode = O_CLC;    return INOARGS; }
"clz"    { yylval.opcode = O_CLZ;    return INOARGS; }
"cln"    { yylval.opcode = O_CLN;    return INOARGS; }
"clv"    { yylval.opcode = O_CLV;    return INOARGS; }
"cls"    { yylval.opcode = O_CLS;    return INOARGS; }
"clh"    { yylval.opcode = O_CLH;    return INOARGS; }
"clt"    { yylval.opcode = O_CLT;    return INOARGS; }
"cli"    { yylval.opcode = O_CLI;    return INOARGS; }
"nop"    { yylval.opcode = O_NOP;    return INOARGS; }
"icall"  { yylval.opcode = O_ICALL;  return INOARGS; } /* Misc. */
"ijmp"   { yylval.opcode = O_IJMP;   return INOARGS; }
"reti"   { yylval.opcode = O_RETI;   return INOARGS; }
"ret"    { yylval.opcode = O_RET;    return INOARGS; }
"sleep"  { yylval.opcode = O_SLEEP;  return INOARGS; }
"wdr"    { yylval.opcode = O_WDR;    return INOARGS; }
"eijmp"  { yylval.opcode = O_EIJMP;  return INOARGS; }
"eicall" { yylval.opcode = O_EICALL; return INOARGS; }
"spm" 	 { yylval.opcode = O_SPM;    return INOARGS; }
"espm"   { yylval.opcode = O_ESPM;   return INOARGS; }
"bclr"   { yylval.opcode = O_BCLR;   return IIMM;    }
"bset"   { yylval.opcode = O_BSET;   return IIMM;    }
"rcall"  { yylval.opcode = O_RCALL;  return IIMM;    }
"rjmp"   { yylval.opcode = O_RJMP;   return IIMM;    }
"jmp"    { yylval.opcode = O_JMP;    return IIMM;    }
"call"   { yylval.opcode = O_CALL;   return IIMM;    }
"brbc"   { yylval.opcode = O_BRBC;   return IIMMIMM; }
"brbs"   { yylval.opcode = O_BRBS;   return IIMMIMM; }
"cbi"    { yylval.opcode = O_CBI;    return IIMMIMM; }
"sbi"    { yylval.opcode = O_SBI;    return IIMMIMM; }
"sbic"   { yylval.opcode = O_SBIC;   return IIMMIMM; }
"sbis"   { yylval.opcode = O_SBIS;   return IIMMIMM; }
"out"    { yylval.opcode = O_OUT;    return IIMMREG; }
"sts"    { yylval.opcode = O_STS;    return IIMMREG; }
"rol"    { yylval.opcode = O_ROL;    return IREG;    }
"ser"    { yylval.opcode = O_SER;    return IREG;    }
"lsl"    { yylval.opcode = O_LSL;    return IREG;    }
"bld"    { yylval.opcode = O_BLD;    return IREGIMM; }
"bst"    { yylval.opcode = O_BST;    return IREGIMM; }
"cbr"    { yylval.opcode = O_CBR;    return IREGIMM; }
"in"     { yylval.opcode = O_IN;     return IREGIMM; }
"sbrc"   { yylval.opcode = O_SBRC;   return IREGIMM; }
"sbrs"   { yylval.opcode = O_SBRS;   return IREGIMM; }
"ld"     { yylval.opcode = O_LD;     return IINDIRC; }
"st"     { yylval.opcode = O_ST;     return IINDIRC; }
"ldd"    { yylval.opcode = O_LDD;    return IINDIRC; }
"std"    { yylval.opcode = O_STD;    return IINDIRC; }
"X"      { yylval.regid = 26;        return REGXYZ;  }
"Y"      { yylval.regid = 28;        return REGXYZ;  }
"Z"      { yylval.regid = 30;        return REGXYZ;  }
"lpm"    { yylval.opcode = O_LPM;    return ILPM;    }
"elpm"   { yylval.opcode = O_ELPM;   return ILPM;    }

"PC"	 { yylval.val.value=yycodepos/2;yylval.val.valid=TRUE; return COUNTER;}
"DC"	 { yylval.val.value=yydatapos;  yylval.val.valid=TRUE; return COUNTER;}
"EC"	 { yylval.val.value=yyerompos;  yylval.val.valid=TRUE; return COUNTER;}

^{WHITE}*".nolistmac"           list(LIST_NO_MACRO);
^{WHITE}*".nolist"              list(LIST_NO);
^{WHITE}*".listmac"             list(LIST_YES_MACRO);
^{WHITE}*".list"                list(LIST_YES);
^{WHITE}*".exit"                yyeol=TRUE; if((ret=doexit()   )!=SKIPRESTART) return ret;
^{WHITE}*".include"[^\n]*       if(includefile()==EOL) return EOL;
^{WHITE}*".macro"               return macrodef();
".endm"                         errorin(E_ENDM);
"\""("\\\""|[^\n\"])*"\""       return string();
[r]{DECDIGIT}+                  return registers();
{DECDIGIT}+                     return integers();
"0x"{HEXDIGIT}+                 return integers();
"0b"{BINDIGIT}+                 return integers();
"$"{HEXDIGIT}+                  return integers();
{DECDIGIT}{HEXDIGIT}*           return integers();
"'\\x"{HEXDIGIT}{1,2}"'"        return character();
"'\\x"{HEXDIGIT}{3,}"'"         warningin(W_HEX_ESCAPE_INVALID);yylval.val.valid = TRUE;yylval.val.value = 0; return INTEGER;
"'\\"{OCTDIGIT}{1,3}"'"         return character();
"'\\"{OCTDIGIT}{4,}"'"          warningin(W_OCT_ESCAPE_INVALID);yylval.val.valid = TRUE;yylval.val.value = 0; return INTEGER;
"'\\"."'"                       return character();
"'"."'"                         return character();
{LETTER}{CHAR}*                 if((ret=identifier())!=SKIPRESTART)return ret;
<<EOF>>                         yyeol=TRUE; if((ret=restart()   )!=SKIPRESTART)return ret;
[\n]                            yyeol=TRUE; return endofline();
[ \t\r]+                        ;
";".*                           ;
"//".*                          ;
"#".*                           ;
.                               illegal();

%%

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
///                                                                        ///
///   Scanner support functions                                            ///
///                                                                        ///
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////////
  //
  // Identifiers
  //

int character(void)
{
  yylval.val.value = 0;
  yylval.val.valid = FALSE;

  if(yytext[1]=='\\')
  {
    switch (yytext[2])
    {
      case 'n'  : yylval.val.value = '\n' ; yylval.val.valid = TRUE; break;
      case 't'  : yylval.val.value = '\t' ; yylval.val.valid = TRUE; break;
      case 'v'  : yylval.val.value = '\v' ; yylval.val.valid = TRUE; break;
      case 'b'  : yylval.val.value = '\b' ; yylval.val.valid = TRUE; break;
      case 'r'  : yylval.val.value = '\r' ; yylval.val.valid = TRUE; break;
      case 'f'  : yylval.val.value = '\f' ; yylval.val.valid = TRUE; break;
      case 'a'  : yylval.val.value = '\a' ; yylval.val.valid = TRUE; break;
      case '\\' : yylval.val.value = '\\' ; yylval.val.valid = TRUE; break;
      case '\'' : yylval.val.value = '\'' ; yylval.val.valid = TRUE; break;
      case '\"' : yylval.val.value = '\"' ; yylval.val.valid = TRUE; break;
      case '\?' : yylval.val.value = '\?' ; yylval.val.valid = TRUE; break;
      case 'x'  :
      case 'X'  :
      {
        if(isdigit(yytext[3]))
          yylval.val.value = yytext[3] - '0';
        else if(isxdigit(yytext[3]))
          yylval.val.value = tolower(yytext[3]) - 'a' + 10;

        if(isxdigit(yytext[3]) && isxdigit(yytext[4]))
        {
          if(isdigit(yytext[4]))
            yylval.val.value = 16*yylval.val.value + yytext[4] - '0';
          else
            yylval.val.value = 16*yylval.val.value+tolower(yytext[4])-'a'+10;
        }
        yylval.val.valid = TRUE;

      } break;

      case '0'  :
      case '1'  :
      case '2'  :
      case '3'  :
      case '4'  :
      case '5'  :
      case '6'  :
      case '7'  :
      {
        yylval.val.value = yytext[2] - '0';
        if(isdigit(yytext[3]))
        {
          yylval.val.value = 8*yylval.val.value + yytext[3] - '0';
          if(isdigit(yytext[4]))
            yylval.val.value = 8*yylval.val.value + yytext[4] - '0';
        }
        if(yylval.val.value > 0xFF)
        {
          yylval.val.value = 0;
          warningin(W_OCT_OUT_OF_RANGE);
        }
        yylval.val.valid = TRUE;

      } break;

      default   : errorin(E_UNDEF_ESCAPE,yytext);
    }
  }
  else
  {
    yylval.val.value = yytext[1];
    yylval.val.valid = TRUE;
  }
  return INTEGER;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Identifiers
  //

int identifier(void)
{
  char name[MAX_ID_LENGTH+1];

  if(yytext[0] == '_' && yycfg->local_labels)
  {
    if(strlen(yytext)+strlen(yylast_used_label)+2>MAX_ID_LENGTH)
    {
      size_t len;
      
      warningin(W_IDENTIFIER_TOO_LONG);
      name[0]='@';
      memcpy(name+1, yylast_used_label, MAX_ID_LENGTH-1);
      name[MAX_ID_LENGTH] = '\0';
      len = strlen(name);
      if(len>MAX_ID_LENGTH-9)
        len = MAX_ID_LENGTH-9;      // truncate the last_used_label to leave at least 8 characters for the local label
      name[len++] = '@';
      memcpy(name+len, yytext, MAX_ID_LENGTH-len);
      name[MAX_ID_LENGTH] = '\0';
    }
    else
    {
      sprintf(name, "@%s@%s", yylast_used_label, yytext);
    }
  }
  else
  {
    if(strlen(yytext)>MAX_ID_LENGTH)
    {
      warningin(W_IDENTIFIER_TOO_LONG);
      memcpy(name, yytext, MAX_ID_LENGTH);
      name[MAX_ID_LENGTH] = '\0';
    }
    else
      strcpy(name, yytext);
  }

  yylval.symb = getsym(name);

  if(yylval.symb -> macro)
    return macro(yylval.symb);

  return SYMBOL;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Registers : r0 - r31
  //

int registers(void)
{
  int registerno;

  if(strlen(yytext)==2)
  {
    if(isdigit(yytext[1]))
      registerno= yytext[1] - '0';
    else
      return identifier();
  }
  else if(strlen(yytext)==3)
  {
    if(isdigit(yytext[1]) && isdigit(yytext[2]) )
      registerno= 10 * (yytext[1] - '0') + yytext[2] - '0';
    else
      return identifier();
  }
  else
    return identifier();

  if(registerno <= 31)
  {
    yylval.regid = registerno;
    return REGISTER;
  }
  return identifier();
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Strings
  //

int string(void)
{
  if( !(yylval.string = new char[strlen(yytext)+1]) )
    errorexit(X_OUT_OF_MEMORY);

  strcpy(yylval.string, yytext);

  return STRING;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Integer constants : Decimal=10 Hex=0xA Hex=$A Octal=012 Binaray=0b1010
  //

int integers(void)
{
  int  val;
  int  pos;
  char temp[12];

  bool foundhex = (strpbrk(yytext,"ABCDEFabcdef")!=NULL);
  bool foundbin = !(strpbrk(yytext,"ACDEFacdef23456789")!=NULL);

  // Hex : $BABE

  if(yytext[0] == '$' )
  {
    val = 0;
    pos = 1;
    while( yytext[pos] )
    {
      if(isdigit(yytext[pos]))
        val = 16*val + yytext[pos] - '0';
      else
        val = 16*val + tolower(yytext[pos]) - 'a' + 10;
      pos++;
    }
    if(pos>9)
      warningin(W_CONSTANT_TO_BIG);
  }

  // Hex : 0xBABE or 0XBABE

  else if( (yytext[0] == '0' ) && (tolower(yytext[1]) == 'x' ))
  {
    val = 0;
    pos = 2;
    while( yytext[pos] )
    {
      if(isdigit(yytext[pos]))
        val = 16*val + yytext[pos] - '0';
      else
        val = 16*val + tolower(yytext[pos]) - 'a' + 10;
      pos++;

    }
    if(pos>10)
      warningin(W_CONSTANT_TO_BIG);
  }

  // Bin : 0b01010 or 0B01010

  else if( (yytext[0] == '0' ) && (tolower(yytext[1]) == 'b' ) && foundbin)
  {
    val = 0;
    pos = 2;
    while( yytext[pos] )
      val = 2*val + yytext[pos++] - '0';
    if(pos>34)
      warningin(W_CONSTANT_TO_BIG);
  }

  // Hex : 1BABE (Hex that starts with a decimal)

  else if(isdigit(yytext[0]) && foundhex )
  {
    val = 0;
    pos = 0;
    while( yytext[pos] )
    {
      if(isdigit(yytext[pos]))
        val = 16*val + yytext[pos] - '0';
      else
        val = 16*val + tolower(yytext[pos]) - 'a' + 10;
      pos++;
    }
    if(pos>8)
      warningin(W_CONSTANT_TO_BIG);
  }


  // Integer 47806

  else
  {
    int zp = 0;

    for(; yytext[zp]=='0' ; zp++);

    if(!isdigit(yytext[zp]) && (zp>0))
     zp--;

    val = atoi(yytext+zp);
    sprintf(temp, "%i", val);
    if(val && strcmp(temp,yytext+zp)) {
      val = atol(yytext+zp);
      sprintf(temp, "%u", val);
      if(strcmp(temp,yytext+zp)) {
        val = atoi(yytext);
        warningin(W_CONSTANT_TO_BIG);
      }
    }
  }

  yylval.val.value = val;
  yylval.val.valid = TRUE;
  return INTEGER;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Illegal character
  //

void illegal(void)
{
  if(isprint(yytext[0]))
    warningin(W_INVALID_CHAR,yytext[0]);
  else
    warningin(W_INVALID_ASCII,(unsigned char)yytext[0]);
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Include file
  //

int includefile(void)
{
  char name[MAX_LINE_LENGTH+2];
  char filename[MAX_FILENAME_LENGTH+1];
  int  c;
  FILE *f1,*f2;

  if(strlen(yytext)==strlen(".include"))
  {
    yyinput();
    endofline();
    error(E_NO_FILENAME);
    return EOL;
  }

  /// Check filename ////////////////////////////////////////////////////////

  if(strlen(yytext) >= MAX_LINE_LENGTH)
    errorexit(X_LINE_TOO_LONG, yyfilename);

  strcpy(name,yytext+8+strspn(yytext,"\t\v ")
              +strspn(yytext+8+strspn(yytext,"\t\v "),"\t\v "));

  if(!strlen(name))
  {
    yyinput();
    endofline();
    error(E_NO_FILENAME);
    return EOL;
  }

  striprem(name);

  if(!strlen(name))
  {
    yyinput();
    endofline();
    error(E_NO_FILENAME);
    return EOL;
  }

  /// Read NL (or EOF) //////////////////////////////////////////////////////

  yyinput();
  endofline();

  if( !getfilename(name, filename) )
  {
    error(E_INVALID_FILENAME);
    return EOL;
  }

  /// Check context depth ///////////////////////////////////////////////////

  if( yycontext == (MAX_CONTEXT_DEPTH-1) )
  {
    errorin(E_INCLUDE_DEPTH, name);
    return EOL;
  }

  if( !(f1 = fopenInIncpath(filename,"r")) )
  {
    error(E_OPEN_FILE, filename);
    return EOL;
  }
  else if( !(f2 = fopenInIncpath(filename,"r")) )
  {
    fclose(f1);
    error(E_OPEN_FILE, filename);
    return EOL;
  }

  /// Setup context stack ///////////////////////////////////////////////////

  CONTEXT->file       = yyin;
  CONTEXT->file2      = yyin2;
  CONTEXT->line       = yyline;
  CONTEXT->offset     = yyoffset;
  CONTEXT->dataoffset = yydataoffset;
  CONTEXT->eromoffset = yyeromoffset;

  strcpy(CONTEXT->yyinlineold, yyinlineold);
  strcpy(CONTEXT->yyinline,    yyinline);
  strcpy(CONTEXT->yyinlinenew, yyinlinenew);

  statestack[yycontext++] = YY_CURRENT_BUFFER;

  /// Open input file ///////////////////////////////////////////////////////

  if(yyfilecount==MAX_FILES)
    errorexit(X_TOO_MANY_FILES);

  yyfileno = -1;

  for(c=0;c<yyfilecount;c++)
    if(!strcmp(filename, yyfilelist[c]))
      yyfileno = c+1;

  if(yyfileno==-1)
  {
    if( !(yyfilelist[yyfilecount] = new char[MAX_FILENAME_LENGTH]) )
      errorexit(X_OUT_OF_MEMORY);

    strcpy(yyfilelist[yyfilecount], filename);
    yyfileno = ++yyfilecount;
  }

  yyin   = CONTEXT->file  = f1;
  yyin2  = CONTEXT->file2 = f2;

  strcpy(CONTEXT->filename, filename);
  strcpy(yyfilename, filename);

  /// Add .include line to log //////////////////////////////////////////////

  tolog();

  if(!yyinmacro)
    yyline = 1;
  else
    yyline =0;

  yyline = 1; // XXX

  /// Read new lines into log buffers ///////////////////////////////////////

  if(!fgets(yyinline, MAX_LINE_LENGTH, yyin2))
    yyinline[0] = 0;
  if(!fgets(yyinlinenew, MAX_LINE_LENGTH, yyin2))
    yyinlinenew[0] = 0;

  if(strlen(yyinline) == MAX_LINE_LENGTH-1)
    errorexit(X_LINE_TOO_LONG, yyfilename);
  if(strlen(yyinlinenew) == MAX_LINE_LENGTH-1)
    errorexit(X_LINE_TOO_LONG, yyfilename);

  STRIPNR(yyinlinenew);
  STRIPNR(yyinline);

  /// Setup scanner to read from new file ///////////////////////////////////

  yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));

  return TRUE;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // End of line
  //

int endofline(void)
{
  int pos;

  strcpy(yyinlineold, yyinline);
  strcpy(yyinline, yyinlinenew);

  if(!ISMACRO)
  {
    if(!fgets(yyinlinenew,MAX_LINE_LENGTH,yyin2))
      yyinlinenew[0] = '\0';
    if(strlen(yyinlinenew)==MAX_LINE_LENGTH-1)
      errorexit(X_LINE_TOO_LONG, yyfilename);
    //if(yyinmacro)
      yyline++;
  }
  else
  {
    if( (int)strlen(CONTEXT->macstr) > CONTEXT->stringpos)
    {
      pos = strcspn(CONTEXT->macstr+CONTEXT->stringpos,"\n");
      if(pos<MAX_LINE_LENGTH)
        memcpy(yyinlinenew,CONTEXT->macstr+CONTEXT->stringpos, pos);
      else
        errorexit(X_MACRO_LINE_TOO_LONG, yyline, yyfilename);

      yyinlinenew[pos] = 0;
      CONTEXT->stringpos += pos+1;
    }
    else
      yyinlinenew[0] = 0;
  }

  STRIPNR(yyinlinenew);
  return EOL;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Restart scanner after first parse, and handle macro/include context
  //

int restart(void)
{
  int c;

  if(!restartcount++ && !ISMACRO)
  {
    strcpy(yyinlineold, yyinline);
    strcpy(yyinline, yyinlinenew);
    strcpy(yyinlinenew, "This should never show up");
    yyline++;
    return EOL;
  }

  restartcount = 0;

  if(yycontext)
  {
    yy_delete_buffer(YY_CURRENT_BUFFER);
    yy_switch_to_buffer(statestack[--yycontext]);

    if( yycontextstack[yycontext+1]->ismacro )
    {
      yyinmacro--;
      strcpy(yyinlineold, CONTEXT->yyinlineold);
      strcpy(yyinline,    CONTEXT->yyinline);
      strcpy(yyinlinenew, CONTEXT->yyinlinenew);
      delete yycontextstack[yycontext+1]->macstr;

      yysymbolstack[yycontext+1] = NULL;
      yyoffset     = CONTEXT->offset;
      yydataoffset = CONTEXT->dataoffset;
      yyeromoffset = CONTEXT->eromoffset;
      yyline = CONTEXT->line + 1;
      if(ISMACRO)
        endofline();
      else
      {
        if(!fgets(yyinlinenew, MAX_LINE_LENGTH, yyin2))
          yyinlinenew[0] = 0;
      }

      if(strlen(yyinlinenew) == MAX_LINE_LENGTH-1)
        errorexit(X_LINE_TOO_LONG, yyfilename);
      memset(yycontextstack[yycontext+1],0,sizeof(context));
      return SKIPRESTART;
    }
    else
    {
      if(ISMACRO)
      {
        for(c=yycontext;c>=0;c--)
        {
          if(!yycontextstack[c]->ismacro)
          {
            strcpy(yyfilename, yycontextstack[c]->filename);
            c = -1;
          }
        }
      }
      else
        strcpy(yyfilename, CONTEXT->filename);

      yyline = CONTEXT->line;

      fclose(yycontextstack[yycontext+1]->file);
      fclose(yycontextstack[yycontext+1]->file2);

      yyin         = CONTEXT->file;
      yyin2        = CONTEXT->file2;
      yyoffset     = CONTEXT->offset;
      yydataoffset = CONTEXT->dataoffset;
      yyeromoffset = CONTEXT->eromoffset;

      strcpy(yyinline,    CONTEXT->yyinline);
      strcpy(yyinlinenew, CONTEXT->yyinlinenew);
      memset(yycontextstack[yycontext+1],0,sizeof(context));

      yyfileno = -1;
      for(c=0;c<yyfilecount;c++)
        if(!strcmp(yyfilename, yyfilelist[c]))
          yyfileno = c+1;

      if(yyfileno==-1)
        internalerror("FF");
      yylval.val.value = 0x1234;
      return SKIPRESTART;
    }
  }
  else
  {
    if(!yyparseno)
    {
      yyparseno++;
      INITEXTERN;
      yylinetxt[0]   = '\0';
      fseek(yyin, 0, SEEK_SET);
      fseek(yyin2, 0, SEEK_SET);
      if(!fgets(yyinline, MAX_LINE_LENGTH, yyin2))
        strcpy(yyinline,"\n");
      if(!fgets(yyinlinenew, MAX_LINE_LENGTH, yyin2))
        strcpy(yyinlinenew,"\n");
      STRIPNR(yyinline);
      STRIPNR(yyinlinenew);
      return RESTART;
    }
    else
      yyterminate();
  }
  return EOL;
  yyunput(0, NULL); // One yyunput() a day, keeps warning away :-)
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Handle macro definition
  //

int macrodef(void)
{
  int                   ch;
  int                   found      = FALSE;
  int                   bufsize    = 32;
  int                   pos        = 0;
  int                   founddot   = 0;
  int                   comment    = FALSE;
  char                  *buf1      = NULL;
  char                  *buf2      = NULL;
  char                  endmacro[] = ".endm";
  int                   temppos    = 0;
  int                   temptime   = 0;
  char                  temp[MAX_LINE_LENGTH+2];
  symbolTableEntry      *symb;
  int                   namestart;
  int                   nameend;
  char                  name[MAX_LINE_LENGTH+1];
  int                   l;
  int                   lastparm;
  int                   inmacro;
  int                   i;

  /// Check that this is not in a macro context /////////////////////////////

  if(CONTEXT->ismacro)
  {
    while( ((ch=yyinput())!='\r') && (ch!='\n') && (ch!=EOF) );
    errorin(E_MACRO_IN_MACRO);
    endofline();
    return EOL;
  }

  /// Allocate buffer ///////////////////////////////////////////////////////

  if( !(buf1 = new char[bufsize]) )
    errorexit(X_OUT_OF_MEMORY);

  /// Get macro string from input ///////////////////////////////////////////

  while(!found)
  {
    ch = yyinput();

    if(tolower(ch)==endmacro[founddot])
      founddot++;
    else
      founddot=0;

    if(founddot==4)
    {
      found = TRUE;
      buf1[pos-3] = '\0';
      sprintf(yyinlinenew,".endm");
    }
    else if(ch=='\n')
    {
      comment           = FALSE;
      temp[temppos]     = 0;
      temppos           = 0;
      if(temptime++)
      {
        STRIPNR(temp);
        strcpy(yyinlinenew, temp);
        endofline();
        tolog();
      }
    }
    else if(ch==EOF)
      found = TRUE;
    else if(ch==';')
      comment = TRUE;

    buf1[pos++] = ch;

    if(ch!='\n')
      temp[temppos++] = ch;

    if(pos==bufsize)
    {
      buf2 = new char[2*bufsize];
      memcpy(buf2, buf1, bufsize);
      delete buf1;
      buf1 = buf2;
      bufsize *=2;
    }
  }

  /// Read until end of .endmacro line //////////////////////////////////////

  while( ((ch=yyinput()) != EOF) && (ch!='\n') && (ch!='\r'))
    yyinlinenew[temppos++] = ch;

  yyinlinenew[temppos] = 0;

  strcpy(temp, yyinlinenew);

  endofline();
  tolog();

  if((ch==EOF)&&(founddot<4))
  {
    delete buf1;
    if(strlen(yyinlineold))
      yyinlineold[strlen(yyinlineold)-1] = '\0';
    STRIPWS(yyinlineold);
    error(E_EOF_IN_MACRO);
    return restart();
  }

  /// Process string ////////////////////////////////////////////////////////

  pos = strlen(buf1);

  if( pos  && !(buf1[pos-1]=='\n') )
    strcat(buf1, "\n");

  /// Get macro name ////////////////////////////////////////////////////////

  namestart = strspn(buf1,"\t\v ");
  nameend   = strcspn(buf1+namestart,"\t\v \r\n;")+namestart;
  memcpy(name, buf1+namestart, nameend-namestart);
  name[nameend-namestart] = '\0';

  /// Setup macro ///////////////////////////////////////////////////////////

  for(i=yycontext; !yysymbolstack[i]; i--);
 
  if(!yyparseno)
  {
    if(strlen(name)>MAX_ID_LENGTH) {
      warningin(W_MAC_IDENTIFIER_TOO_LONG);
      name[MAX_ID_LENGTH] = '\0';      
    }

    if( (symb=yysymbolstack[i]->get(name)) )
    {
      error(E_MACRO_REDEF,name);
      return EOL;
    }
    symb = yysymbolstack[i]->add(name);
  }
  else {
    if(strlen(name)>MAX_ID_LENGTH) {
      warningin(W_MAC_IDENTIFIER_TOO_LONG);
      name[MAX_ID_LENGTH] = '\0';      
    }
    symb=yysymbolstack[i]->get(name);
  }

  if(!symb)
    internalerror("DM");

  if( ISUSED(symb) && ((symb->macdefline != yyline)
      ||(strcmp(symb->macfilename, yyfilename)) ))
  {
    strcpy(yyinlineold,".endmacro");
    error(E_MACRO_REDEF,name);
  }
  else
  {
    // If first parse - allocate space in symbol table for macro

    if(!yyparseno)
    {
      symb -> macro      = TRUE;
      symb -> macsize    = strlen(buf1)-nameend;
      symb -> macdefline = yyline;

      if( !(symb -> macstr = new char[symb->macsize+2]))
        errorexit(X_OUT_OF_MEMORY);

      strcpy(symb->macstr, buf1+nameend);
      strcpy(symb->macfilename,yyfilename);
    }

    // In second parse - check argument usage

    else if( yyline == symb->macdefline )
    {
      l = symb->macsize;
      symb -> macrodone = TRUE;

      while(l)
      {
        if( isdigit(symb->macstr[l]) && (symb->macstr[l-1] == '@' ) )
          symb->macparmlist[(symb->macstr[l]) -'0'] = 1;
        l--;
      }
      lastparm = -1;
      for(l=0;l<10;l++)
        if(symb->macparmlist[l])
          lastparm = l;

      inmacro=yyinmacro;
      yyinmacro=1;

      for(l=0;l<=lastparm;l++)
        if(!symb->macparmlist[l])
          warningin(W_MACRO_UNUSES_PARM, l, name);

      yyinmacro=inmacro;

      strcpy(yyinlineold, yyinline);
      strcpy(yyinline, yyinlinenew);
    }
  }

  strcpy(yyinline, temp);
  endofline();
  delete buf1;

  if(!strlen(name))
    error(E_NO_MACRO_NAME);

  return MACRODEF;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Insert macro in input stream
  //

int macro(symbolSA symb)
{
  char  **parmlist;
  int   c;
  char  line[MAX_LINE_LENGTH+2];
  int   pos = 0;
  int   parm;
  int   inputparms = 0;
  int   parmpos;
  int   rem;
  int   next;
  int   l;
  int   len;
  char  *str;
  int   res;
  int   ch;
  char  orginline[MAX_LINE_LENGTH+2];

  if( (!symb -> macrodone) && (yyparseno) )
  {
    errorin(E_DEF_USE);
    while(((ch=yyinput())!='\n') && (ch!=EOF) );
    endofline();

    return EOL;
  }

  strcpy(orginline, yyinline);

  /// Allocate space for parameters /////////////////////////////////////////

  if( !(parmlist = new char*[10]) )
    errorexit(X_OUT_OF_MEMORY);

  for(c=0;c<10;c++)
  {
    if(!(parmlist[c]= new char [MAX_LINE_LENGTH+1]))
      errorexit(X_OUT_OF_MEMORY);
    else
      memset(parmlist[c], 0, MAX_LINE_LENGTH+1);
  }

  /// Get parameters from input /////////////////////////////////////////////

  while( ((line[pos]=yyinput())!='\n') && (line[pos++]!=EOF) );

  if(pos && (line[pos-1]==EOF) )
    line[--pos] = 0;

  line[pos]=0;
  line[pos+1]=0;

  /// Local strtok() that inserts parameters into parameter list ////////////

  if(pos)
  {
    pos         = 0;
    parm        = 0;
    rem         = FALSE;

    striprem(line);

    while(line[pos] && (parm<10))
    {
      next        = FALSE;
      parmpos     = 0;

      while( (!next) && (line[pos]) )
      {
        if( (line[pos]=='"') || (line[pos]=='\'') )
          rem ? rem = FALSE : rem = TRUE;
        else if( line[pos]=='\\' )
          parmlist[parm][parmpos++] = line[pos++];
        else if ( (line[pos]==',') && !rem)
        {
          parmlist[parm][parmpos] = '\0';
          next = TRUE;
        }
        parmlist[parm][parmpos++] = line[pos++];
      }
      parm++;
    }

    inputparms = parm;

  /// Clean up parametes (remove leading/trailing whitespaces and ',' ////////

    for(c=0;c<10;c++)
    {
      pos = strlen(parmlist[c]);
      if(pos)
        pos--;
      while( ((parmlist[c][pos]==',')||(isspace(parmlist[c][pos])))&&(pos>=0))
        parmlist[c][pos--] = '\0';

      if(pos)
      {
        l = strspn(parmlist[c], "\t\v ");
        len = strlen(parmlist[c])-l;
        if(l)
          memmove(parmlist[c], parmlist[c]+l,len);
        parmlist[c][len] = '\0';
      }
    }
  }

  /// Check that number of parameters specified, equals the ones used ////////

  for(c=0;c<10;c++)
  {
    if( parmlist[c][0] && !symb->macparmlist[c])
      warningin(W_ARG_SPEC, c);
    else if( !parmlist[c][0] && symb->macparmlist[c])
      warningin(W_ARG_USED, c);
  }

  /// Insert parameters in macro string //////////////////////////////////////

  if(! (str = new char[symb-> macsize + MAX_LINE_LENGTH +5] ) )
    errorexit(X_OUT_OF_MEMORY);

  memset(str,0, symb-> macsize + MAX_LINE_LENGTH +5);

  pos     = 0;
  parm    = 0;
  res     = 0;

  while( symb->macstr[pos] )
  {
    if( (symb->macstr[pos]=='@') && (isdigit(symb->macstr[pos+1])) )
    {
      parm     = symb->macstr[pos+1] - '0';
      pos     += 2;
      parmpos  = 0;

      while(parmlist[parm][parmpos])
        str[res++] = parmlist[parm][parmpos++];
    }
    else
      str[res++] = symb->macstr[pos++];
  }

  /// Setup new scanner context //////////////////////////////////////////////

  if( yycontext == (MAX_CONTEXT_DEPTH-1) )
  {
    errorexit(X_MACRO_DEPTH);

    for(c=0;c<10;c++)
      delete parmlist[c];
    delete parmlist;

    return EOL;
  }

  strcpy(yyinlineold, yyinline);
  strcpy(yyinline, yyinlinenew);

  strcpy(CONTEXT->yyinlineold, yyinlineold);
  strcpy(CONTEXT->yyinline,    yyinline);
  strcpy(CONTEXT->yyinlinenew, yyinlinenew);

  CONTEXT->line        = yyline;
  CONTEXT->offset      = yyoffset;
  CONTEXT->dataoffset  = yydataoffset;
  CONTEXT->eromoffset  = yyeromoffset;

  statestack[yycontext++]                = YY_CURRENT_BUFFER;
  CONTEXT->ismacro     = TRUE;
  CONTEXT->macstr      = str;
  yyoffset     = yycodepos/2;
  yyeromoffset = yyerompos;
  yydataoffset = yydatapos;

  memset(yyinlinenew, 0, MAX_LINE_LENGTH);
  memset(yyinline,  0, MAX_LINE_LENGTH);

  pos = strcspn(str,"\n") + 1;

  strcpy(yyinline, symb->name);

  for(l=0;l<inputparms;l++)
  {
    if(l)
      strcat(yyinline, ", ");
    else
      strcat(yyinline, " ");
    strcpy(yyinline+strlen(yyinline),parmlist[l]);
  }

  if(pos+strlen(yyinline) >= MAX_LINE_LENGTH-1 )
    errorexit(X_MACRO_LINE_TOO_LONG, yyline, yyfilename);

  memcpy(yyinline+strlen(yyinline), str ,pos);

  CONTEXT->stringpos = strcspn(str+pos,"\n") + 1;

  memcpy(yyinlinenew, str + pos, CONTEXT->stringpos);

  CONTEXT->stringpos += pos;

  strcpy(yyinline, orginline);

  yyinmacro++;
  yyfirstmacroline = TRUE;

  yy_scan_string(str);

  /// Delete parameter list //////////////////////////////////////////////////

  for(c=0;c<10;c++)
    delete parmlist[c];
  delete parmlist;

  /// Allocate new symbol table //////////////////////////////////////////////

  if(!yyparseno)
  {
    if(symb -> macsym)
      yysymbolstack[yycontext] = symb -> macsym;
    else
    {
      if( !(yysymbolstack[yycontext] = symb -> macsym = new symbolTable()) )
        errorexit(X_OUT_OF_MEMORY);

      if( !symb -> macsym->init(yycfg->casesensitive, 16) )
        errorexit(X_OUT_OF_MEMORY);
    }
  }
  else
    yysymbolstack[yycontext] = symb -> macsym;

  if(!yysymbolstack[yycontext])
    internalerror("MA %i", yycontext);

//  yyline++;

  return SKIPRESTART;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Turn listing on/off
  //

void list(int listtype)
{
  switch(listtype)
  {
    case LIST_NO        : yylist      = FALSE; break;
    case LIST_YES       : yylist      = TRUE ; break;
    case LIST_NO_MACRO  : yylistmacro = FALSE; break;
    case LIST_YES_MACRO : yylistmacro = TRUE ; break;
    default             : internalerror("LU %04X",listtype);
  }
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Read to end of current context
  //

int doexit(void)
{
  while(yyinput()!=EOF);
  yyeol=TRUE; 
  return restart();
}


/// END OF FILE //////////////////////////////////////////////////////////////
