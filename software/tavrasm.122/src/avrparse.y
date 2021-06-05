//////////////////////////////////////////////////////////////////////////////
//
//  File        : avrparse.y
//
//  Author      : Tom Mortensen - Copyright (C) 1999
//
//  Description : This module implements the grammar file for AVRASM
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

%{

/// Include //////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include "avrasm.hh"
#include "symbol.hh"
#include "semantic.hh"
#include "utils.hh"

/// Extern ///////////////////////////////////////////////////////////////////

GLOBALS(extern);
extern char *yytext;

/// Prototypes ///////////////////////////////////////////////////////////////

int yylex();

/// yyerror //////////////////////////////////////////////////////////////////

void yyerror(char *s)
{
  s = s; // Used for debugging purposes
}

/// Start of grammar /////////////////////////////////////////////////////////

%}

/// Attribute union //////////////////////////////////////////////////////////

%union
{
  regSA                 regid;
  instSA                inst;
  opcodeSA              opcode;
  valueSA               val;
  nameSA                name;
  symbolSA              symb;
  indirectSA            indi;
  functionSA            func;
  stringSA              string;
}

/// Terminal symbols that synthesizes default value //////////////////////////

%token  STAR    DIV     MOD
%token  LS      RS
%token  LE      GE      LESS    GREAT   EQ      NE
%token  OR      XOR     AND
%token  OROR    ANDAND
%token  LPAR    RPAR
%token  COLON   COMMA   DOT     EQUAL   PLUS    MINUS   WAVE    NOT
%token  EOL     RESTART ENDOFFILE
%token  DEF     EQU     DB      DW      ORG     ALIGN	CSEG    DSEG    ESEG
%token  BYTE    SET     DEVICE  STRING  MACRODEF

/// Attributes for terminal symbols //////////////////////////////////////////

%token  <regid>         REGISTER
%token  <regid>         REGXYZ
%token  <symb>          SYMBOL
%token  <val>           INTEGER
%token  <val>           COUNTER
%token  <func>          FUNCTION
%token  <opcode>        IREGREG
%token  <opcode>        IREGREGW
%token  <opcode>        IIMMIMM
%token  <opcode>        IREGIMM
%token  <opcode>        IREGIMMW
%token  <opcode>        IIMMREG
%token  <opcode>        IREG
%token  <opcode>        IIMM
%token  <opcode>        INOARGS
%token  <opcode>        IINDIRC
%token  <opcode>        ILPM
%token  <string>        STRING

/// Attributes for non-terminal symbols //////////////////////////////////////

%type   <inst>          instruction
%type   <inst>          lpminst
%type   <val>           expr
%type   <indi>          indirectaddr
%type   <regid>         registername
%type   <val>           unary_expr
%type   <val>           primary_expr
%type   <val>           mult_expr
%type   <val>           additive_expr
%type   <val>           shift_expr
%type   <val>           relational_expr
%type   <val>           equality_expr
%type   <val>           AND_expression
%type   <val>           exclusive_OR
%type   <val>           inclusive_OR
%type   <val>           logical_AND
%type   <val>           logical_OR
%type   <val>           composite_expr

/// Expect 4 shift/reduce conflicts //////////////////////////////////////////

%expect 4

/// The goal symbol //////////////////////////////////////////////////////////

%start  program

%% // Start of grammar ///////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////////
  //
  //  program : programlist RESTART programlist
  //
  //  e       :
  //
  ////////////////////////////////////////////////////////////////////////////
  //
  //  We start by defining the goal symbol 'program'. There is only one
  //  production that can reduce the input to 'program':
  //
  //  - 'programlist' RESTART 'programlist'
  //
  //  with one 'programlist' for each pass.
  //
  //  The symbol 'e' (or epsilon) is also defined. It should be considered
  //  a terminal symbol, and is only used to increase readability.
  //
  ////////////////////////////////////////////////////////////////////////////

program         : programlist RESTART programlist
                ;

e               :
                ;
	
  ////////////////////////////////////////////////////////////////////////////
  //
  //  programlist := programlist programelement | programelement
  //
  ////////////////////////////////////////////////////////////////////////////
  //
  //  'programlist' is a left recursive list production, that parses one or
  //  more 'programelement's.
  //
  ////////////////////////////////////////////////////////////////////////////

programlist     : programlist programelement
                | programelement
                ;

  ////////////////////////////////////////////////////////////////////////////
  //
  //  programelement := label instruction | label EOL | label directive | EOL
  //
  ////////////////////////////////////////////////////////////////////////////
  //
  //  All instructions / defines are limited to a single line
  //
  ////////////////////////////////////////////////////////////////////////////

programelement  : label instruction  { tolog(); }
                | label EOL          { tolog(); }
                | label directive    { tolog(); }
                | EOL                { tolog(); }

                /// Error recovery //////////////////////////////////////////

                | error EOL          { error(E_UNKNOWN_OPCODE); tolog();  }
                ;

  ////////////////////////////////////////////////////////////////////////////
  //
  //  instruction := IREGREG registername COMMA registername EOL
  //               | IREGREGW registername COMMA registername EOL
  //               | IREGREGW REGXYZ COMMA registername EOL
  //               | IREGIMM registername COMMA expr EOL
  //               | IREGIMMW registername COMMA expr EOL
  //               | IREGIMMW REGXYZ COMMA expr EOL
  //               | IIMMREG expr COMMA registername EOL
  //               | IIMMIMM expr COMMA expr EOL
  //               | IIMM expr EOL
  //               | IREG registername EOL
  //               | INOARGS EOL
  //               | IINDIRC registername COMMA indirectaddr EOL
  //               | IINDIRC indirectaddr COMMA registername EOL
  //
  ////////////////////////////////////////////////////////////////////////////
  //
  //  There are nine different forms of 'instruction' :
  //
  //  MOV       r1, r1          ; Register  / Register
  //  MOV       r1, 0x10        ; Register  / expr
  //  OUT       0x10, r1        ; expr / Register
  //  BRBS      6, bitfound     ; expr / expr
  //  JMP       0x1000          ; expr
  //  CLR       r29             ; Register
  //  CLI                       ; Noargs
  //  LD        r2, -Y          ; Indirect
  //  ST        Y+, r2          ; Indirect
  //
  //  Notice: LPM/ELPM are special, since they can be specified with or 
  //          without arguments (arrgh).
  //
  ////////////////////////////////////////////////////////////////////////////

instruction     : IREGREG registername COMMA registername EOL
                  {genRegReg($<opcode>1, $<regid>2, $<regid>4);}
                | IREGREGW registername COMMA registername EOL
                  {genRegReg($<opcode>1, $<regid>2, $<regid>4);}
                | IREGREGW REGXYZ COMMA registername EOL
                  {genRegReg($<opcode>1, $<regid>2, $<regid>4);}
                | IREGIMM registername COMMA expr EOL
                  {genRegImm($<opcode>1, $<regid>2, &$<val>4);}
                | IREGIMMW registername COMMA expr EOL
                  {genRegImm($<opcode>1, $<regid>2, &$<val>4);}
                | IREGIMMW REGXYZ COMMA expr EOL
                  {genRegImm($<opcode>1, $<regid>2, &$<val>4);}
                | IIMMREG expr COMMA registername EOL
                  {genImmReg($<opcode>1, &$<val>2, $<regid>4);}
                | IIMMIMM expr COMMA expr   EOL
                  {genImmImm($<opcode>1, &$<val>2, &$<val>4);}
                | IIMM expr EOL
                  {genImmedi($<opcode>1, &$<val>2);}
                | IREG registername EOL
                  {genRegist($<opcode>1, $<regid>2);}
                | INOARGS EOL
                  {genNoargs($<opcode>1);}
                | IINDIRC registername COMMA indirectaddr EOL
                  {genIndirc($<opcode>1, &$<indi>4, $<regid>2,TRUE);}
                | IINDIRC indirectaddr COMMA registername EOL
                  {genIndirc($<opcode>1, &$<indi>2, $<regid>4);}

                | lpminst

                /// Error recovery //////////////////////////////////////////

                | IREGREG  error EOL { error(E_REGISTER_EXPECTED);  }
                | IREGREGW error EOL { error(E_REGISTER_EXPECTED);  }
                | IREGIMM  error EOL { error(E_INVALID_REGIMM_SPEC);    }
                | IREGIMMW error EOL { error(E_INVALID_REGIMM_SPEC);    }
                | IIMMREG  error EOL { error(E_INVALID_REGIMM_SPEC);    }
                | IIMMIMM  error EOL { error(E_INVALID_IMMEDIATE_SPEC); }
                | IIMM     error EOL { error(E_INVALID_IMMEDIATE_SPEC); }
                | IREG     error EOL { error(E_INVALID_REGISTER_SPEC);  }
                | INOARGS  error EOL { error(E_NOARGS_EXPECTED_SPEC);   }
                | IINDIRC  error EOL { error(E_INVALID_REGISTER_SPEC);  }
                ;

  ////////////////////////////////////////////////////////////////////////////
  //
  //  lpminst         := ILPM EOL | ILPM registername COMMA indirectaddr 
  //
  ////////////////////////////////////////////////////////////////////////////
  //
  //  Handles LPM and ELPM instructions
  //
  ////////////////////////////////////////////////////////////////////////////

lpminst         : ILPM EOL                             
                  { genLpm($<opcode>1, 0, NULL, FALSE); }
                | ILPM registername COMMA indirectaddr EOL
                  { genLpm($<opcode>1, $<regid>2, &$<indi>4, TRUE); }

                /// Error recovery //////////////////////////////////////////

                | ILPM error EOL { error(E_INVALID_REGISTER_SPEC); }
                ;

  ////////////////////////////////////////////////////////////////////////////
  //
  //  registername  := REGISTER | SYMBOL
  //
  ////////////////////////////////////////////////////////////////////////////
  //
  // Handle registers: r0 - r31
  //
  ////////////////////////////////////////////////////////////////////////////

registername    : REGISTER
                | SYMBOL
                  { if($<symb>1->isdefine) 
                      $<regid>$=$<symb>1->reg;
                    else
		    {
                      $<regid>$=-1;
                      errorin(E_INVALID_REGISTER_SPEC);
                    }
                  }
                ;

  ////////////////////////////////////////////////////////////////////////////
  //
  // lable      :=  SYMBOL COLON
  //             |  e
  //
  ////////////////////////////////////////////////////////////////////////////
  //
  // SYMBOL COLON = Create new label
  //
  ////////////////////////////////////////////////////////////////////////////

label           : SYMBOL COLON  { doLab($<symb>1); }
                | e
                ;

  ////////////////////////////////////////////////////////////////////////////
  //
  // directive := DEF SYMBOL EQUAL REGISTER EOL
  //            | DEF SYMBOL EQUAL SYMBOL EOL
  //            | EQU SYMBOL EQUAL expr EOL
  //            | DB {doAdb();} byteexprlist EOL
  //            | DW {doAdw();} wordexprlist EOL
  //            | ORG expr EOL
  //            | ALIGN expr EOL
  //            | CSEG EOL
  //            | DSEG EOL
  //            | ESEG EOL
  //            | BYTE expr EOL
  //            | SET SYMBOL EQUAL expr EOL
  //            | MACRODEF
  //            | DEVICE SYMBOL EOL
  //
  ////////////////////////////////////////////////////////////////////////////
  //
  // 'directive' insert assembler directives
  //
  ////////////////////////////////////////////////////////////////////////////

directive       : DEF SYMBOL EQUAL REGISTER EOL   {doDef($<symb>2,$<regid>4);}
                | DEF SYMBOL EQUAL SYMBOL EOL     {doDef($<symb>2,$<symb>4); }
                | EQU SYMBOL EQUAL expr EOL       {doEqu($<symb>2,&$<val>4); }
                | DB {doAdb();} byteexprlist EOL  {/*XXX*/                   }
                | DW {doAdw();} wordexprlist EOL  {                          }
                | ORG expr EOL                    {doOrg(&$<val>2);          }
                | ALIGN expr EOL                  {doAlign(&$<val>2);        }
                | CSEG EOL                        {                          }
                | DSEG EOL                        {                          }
                | ESEG EOL                        {                          }
                | BYTE expr EOL                   {doByt(&$<val>2);          }
                | SET SYMBOL EQUAL expr EOL       {doSet($<symb>2,&$<val>4); }
                | MACRODEF                        {                          }
                | DEVICE SYMBOL EOL               {doDev($<symb>2);          }

                /// Error recovery //////////////////////////////////////////

                | DEF error EOL    { error(E_EXPECTED_ID_REG);    }
                | EQU error EOL    { error(E_EXPECTED_ID_EXPR);   }
                | DB  error EOL    { error(E_EXPECTED_VALLIST);   }
                | DW  error EOL    { error(E_EXPECTED_VALLIST);   }
                | ORG error EOL    { error(E_EXPECTED_VAL_LABEL); }
                | ALIGN error EOL  { error(E_EXPECTED_VAL_LABEL); }
                | CSEG error EOL   { error(E_EXPECTED_NOARGS);    }
                | DSEG error EOL   { error(E_EXPECTED_NOARGS);    }
                | ESEG error EOL   { error(E_EXPECTED_NOARGS);    }
                | BYTE error EOL   { error(E_EXPECTED_VAL_LABEL); }
                | SET error EOL    { error(E_EXPECTED_ID_EXPR);   }
                | DEVICE error EOL { error(E_EXPECTED_DEVICE);    }
                ;

  ////////////////////////////////////////////////////////////////////////////
  //
  // indirectaddr := MINUS REGXYZ
  //               | REGXYZ
  //               | REGXYZ PLUS
  //               | REGXYZ PLUS expr
  //
  ////////////////////////////////////////////////////////////////////////////
  //
  // There are 4 different forms of indirect addressing:
  //
  //   "-Z", "Z", "Z+" and "Z+offset"
  //
  ////////////////////////////////////////////////////////////////////////////

indirectaddr    : MINUS REGXYZ
                  {$<indi>$.regno=$<regid>2;$<indi>$.plus=2;$<indi>$.disp=0;}
                | REGXYZ
                  {$<indi>$.regno=$<regid>1;$<indi>$.plus=0;$<indi>$.disp=0;}
                | REGXYZ PLUS
                  {$<indi>$.regno=$<regid>1;$<indi>$.plus=1;$<indi>$.disp=0;}
                | REGXYZ PLUS expr
                  { $<indi>$.regno  = $<regid>1;
                    $<indi>$.plus=1;$<indi>$.disp=1;$<indi>$.offset=$<val>3;}
                ;

  ////////////////////////////////////////////////////////////////////////////
  //
  //  byteexprlist := byteexprlist COMMA expr
  //                | expr
  //
  //  wordexprlist := wordexprlist COMMA expr
  //                | expr
  //
  ////////////////////////////////////////////////////////////////////////////
  //
  // List of bytes and words for .db and .dw respectively
  //
  ////////////////////////////////////////////////////////////////////////////

byteexprlist    : byteexprlist COMMA byteelement
                | byteelement
                ;

byteelement     : STRING                  {doAdb($<string>1); }
                | expr                    {doAdb(&$<val>1);}
                ;

wordexprlist    : wordexprlist COMMA expr {doAdw(&$<val>3);}
                | expr                    {doAdw(&$<val>1);}
                ;

  ////////////////////////////////////////////////////////////////////////////
  //
  //  expr := ...
  //
  ////////////////////////////////////////////////////////////////////////////
  //
  // Constant expressions with 'C' precedence
  //
  ////////////////////////////////////////////////////////////////////////////

expr            : composite_expr
                ;
    
primary_expr    : INTEGER
		| COUNTER
                | SYMBOL
                  {$<val>$.valid=$<symb>1->valid;
                   $<val>$.value=$<symb>1->value
                   +$<symb>1->islabel*$<symb>1->macrolabel*
                    (yyoffset    *($<symb>1->segment==SEGMENT_CODE))
                   +$<symb>1->islabel*$<symb>1->macrolabel*
                    (yydataoffset*($<symb>1->segment==SEGMENT_DATA))
                   +$<symb>1->islabel*$<symb>1->macrolabel*
                    (yyeromoffset*($<symb>1->segment==SEGMENT_EEPROM));}
                | LPAR expr RPAR
                  { $<val>$=$<val>2; }
                | FUNCTION LPAR expr RPAR
                  { genFun($<func>1, &$<val>3, &$<val>$); }
                ;

unary_expr      : MINUS unary_expr { oprUna(OP_MINUS, &$<val>2, &$<val>$); }
                | WAVE  unary_expr { oprUna(OP_WAVE , &$<val>2, &$<val>$); }
                | NOT   unary_expr { oprUna(OP_NOT  , &$<val>2, &$<val>$); }
                | primary_expr
                ;

mult_expr       : unary_expr
                | mult_expr STAR unary_expr
                  { oprBin(&$<val>1,OP_STAR  , &$<val>3, &$<val>$); }
                | mult_expr DIV  unary_expr
                  { oprBin(&$<val>1,OP_DIV   , &$<val>3, &$<val>$); }
                | mult_expr MOD  unary_expr
                  { oprBin(&$<val>1,OP_MOD   , &$<val>3, &$<val>$); }
                ;

additive_expr   : mult_expr
                | additive_expr PLUS  mult_expr
                  { oprBin(&$<val>1,OP_PLUS  , &$<val>3, &$<val>$); }
                | additive_expr MINUS mult_expr
                  { oprBin(&$<val>1,OP_MINUS , &$<val>3, &$<val>$); }
                ;

shift_expr      : additive_expr
                | shift_expr LS additive_expr
                  { oprBin(&$<val>1,OP_LS    , &$<val>3, &$<val>$); }
                | shift_expr RS additive_expr
                  { oprBin(&$<val>1,OP_RS    , &$<val>3, &$<val>$); }
                ;

relational_expr : shift_expr
                | relational_expr LESS  shift_expr
                  { oprBin(&$<val>1,OP_LESS  , &$<val>3, &$<val>$); }
                | relational_expr GREAT shift_expr
                  { oprBin(&$<val>1,OP_GREAT , &$<val>3, &$<val>$); }
                | relational_expr LE    shift_expr
                  { oprBin(&$<val>1,OP_LE    , &$<val>3, &$<val>$); }
                | relational_expr GE    shift_expr
                  { oprBin(&$<val>1,OP_GE    , &$<val>3, &$<val>$); }
                ;

equality_expr   : relational_expr
                | equality_expr EQ relational_expr
                  { oprBin(&$<val>1,OP_EQ    , &$<val>3, &$<val>$); }
                | equality_expr NE relational_expr
                  { oprBin(&$<val>1,OP_NE    , &$<val>3, &$<val>$); }
                ;

AND_expression  : equality_expr
                | AND_expression AND equality_expr
                  { oprBin(&$<val>1,OP_AND   , &$<val>3, &$<val>$); }
                ;

exclusive_OR    : AND_expression
                | exclusive_OR XOR AND_expression
                  { oprBin(&$<val>1,OP_XOR   , &$<val>3, &$<val>$); }
                ;

inclusive_OR    : exclusive_OR
                | inclusive_OR OR exclusive_OR
                  { oprBin(&$<val>1,OP_OR    , &$<val>3, &$<val>$); }
                ;

logical_AND     : inclusive_OR
                | logical_AND ANDAND inclusive_OR
                  { oprBin(&$<val>1,OP_ANDAND, &$<val>3, &$<val>$); }
                ;

logical_OR      : logical_AND
                | logical_OR  OROR logical_AND
                  { oprBin(&$<val>1,OP_OROR  , &$<val>3, &$<val>$); }
                ;

composite_expr  : logical_OR
                ;

/// END OF FILE //////////////////////////////////////////////////////////////
