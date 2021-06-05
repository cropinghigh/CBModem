//////////////////////////////////////////////////////////////////////////////
//
//  File        : semantic.cc
//
//  Author      : Tom Mortensen - Copyright (C) 1999
//
//  Description : Implements code generation functions
//
//  History  
//  ========================================================================
//
//  980902      : Tom - File created.
//  990124      : Tom - Added GPL notice.
//  990516      : Tom - Added support for wrapping in branch/rcall/rjmp
//  990721	: Tom - Fixed SBR (bit 0 was always set)
//  991209	: Tom - Fixed problem with BRBC/BRBS (they did not work :(
//  991211      : Tom - Added the new instructions: 'eijmp', 'eicall', 'espm',
//                      'mulsu', 'fmul', 'fmuls', 'fmulsu', 'muls', 'movw', 
//                      'elpm'. And changed behaviour of 'lpm' according to
//                      Atmel specs.
//  991217      : Kurt- Valid range for W_CONST_OUT_OF_RANGE_FF expanded to
//                      -255...255. TODO: Change the error messages.
//  991217      : Kurt- Support of local labels.
//  040109	: John Romein (john@cs.vu.nl) - Added .align support
//  040306      : Dariusz Kowalewski <darekk@automex.pl> - Modified handling
//                      of '.device' directive (support for ATmega devices
//                      with extended I/O - different 'datastart' addresses)
//  041204      : Tom - Fixed problem with the .align directive
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
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include "avrasm.hh"
#include "semantic.hh"
#include "symbol.hh"
#include "utils.hh"

/// Externals ////////////////////////////////////////////////////////////////

GLOBALS(extern);

/// Functions ////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////////
  //
  // Handle '.device' directive
  //

void  doDev(symbolSA devicename)
{
  int  devfound = -1;

  for(int devno=0; (devfound==-1) && yydevices[devno].name[0]; devno++)
  {
    // this is just a stricmp/strcasecmp

    int c;

    for(c=0; devicename->name[c] &&
            (tolower(yydevices[devno].name[c]) ==
             tolower(devicename->name[c])); c++);

    if(!yydevices[devno].name[c] && !devicename->name[c])
      devfound = devno;
  }

  if(devfound == -1)
    warning(W_UNKNOWN_DEV, devicename -> name);
  else
  {
    if(yydatapos != DEVINF.datastart)
      // ".device" directive following ".dseg" declarations
      error(E_DEV_AFTER_DSEG_DATA, devicename->name);
    else
    {
      if(yydeviceno == 0) //initial 'GENERIC' device
      {
        // set new deviceno and initial yydatapos
        yydeviceno = devfound;
        yydatapos = DEVINF.datastart;
      }
      else if(yydeviceno == devfound)
          // the same device has already been specified
          warning(W_DEV_REDEF, devicename->name);
      else
        // different devices specified
        error(E_DEV_REDEF, DEVINF.name, devicename->name);
    }
  }
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Handle directive '.byte'
  //

void doByt(valueSA *value)
{
  if(! (value->valid))
    error(E_UNKNOWN_BYTE);
  else if(yysegment == SEGMENT_CODE)
    error(E_VAR_IN_CSEG);
  else if(yysegment == SEGMENT_DATA)
  {
    yydefinestart = yydatapos;
    yydatapos+=value->value;
  }
  else if(yysegment == SEGMENT_EEPROM)
  {
    yydefinestart =  yyerompos;
    yydefinetype  =  SEGMENT_DEFINE_BYTE;

    for(int c=0;c<value->value;c++)
      insertdata(0);

    if(yyerompos > DEVINF.eepromsize)
      warning(W_EROM_EXCEED,yyerompos-DEVINF.eepromsize);
  }
  else
    internalerror("BY %04X", yysegment);

}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Handle directive '.org'
  //

void doOrg(valueSA *value)
{
  if( !value->valid )
  {
    error(E_UNKNOWN_ORG);
    return;
  }

  if(yysegment == SEGMENT_CODE)
  {
    if(!yycfg->forwardorg && (value->value < (yycodepos+1)/2))
      warning(W_ORG_OUT_OF_RANGE,value->value,(yycodepos+1)/2-value->value);
    if(yycodeposhigh < yycodepos)
      yycodeposhigh = yycodepos;
    yycodepos = 2*value->value;
  }
  else if(yysegment == SEGMENT_EEPROM)
  {
    if(!yycfg->forwardorg && (value->value < yyerompos))
      warning(W_ORG_OUT_OF_RANGE_ESEG,value->value, yyerompos - value->value);
    if(yyeromposhigh<yyerompos)
      yyeromposhigh = yyerompos;
    yyerompos = value->value;
  }
  else if(yysegment == SEGMENT_DATA)
  {
    if(value->value < DEVINF.datastart)
      warning(W_ORG_OVER_REG,value->value,DEVINF.datastart-value->value);
    else if(!yycfg->forwardorg && (value->value < yydatapos))
      warning(W_ORG_OUT_OF_RANGE_DSEG,value->value, yydatapos - value->value);
    if(yydataposhigh < yydatapos)
      yydataposhigh = yydatapos;
    yydatapos = value->value;
  }
  else
    internalerror("US %08X", yysegment);
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Handle directive '.align'
  //

void doAlign(valueSA *value)
{
  if (!value->valid)
    error(E_UNKNOWN_ALIGN);
  else if ((value->value | (value->value - 1)) != 2 * value->value - 1)
    error(E_ALIGN_NOT_POWER_OF_TWO);
  else switch (yysegment) {
    case SEGMENT_CODE:
      if(yycodepos&(2*value->value - 1))
	yycodepos = (yycodepos&~(2*value->value - 1)) + 2*value->value;
	 break;
    case SEGMENT_EEPROM:
      if(yyerompos&(value->value - 1))
	 yyerompos = (yyerompos & ~(value->value - 1)) + value->value;
	 break;
    case SEGMENT_DATA:
      if(yydatapos&(value->value - 1))
	 yydatapos = (yydatapos & ~(value->value - 1)) + value->value;
	 break;
    default:
	 internalerror("US %08X", yysegment);
  }
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Initialize '.db' directive
  //

void doAdb(void)
{
  if(yysegment == SEGMENT_CODE)
    yydefinestart = yycodepos;
  else if(yysegment == SEGMENT_EEPROM)
    yydefinestart = yyerompos;
  else
    errorin(E_DB_IN_DSEG);

  yyeol = FALSE;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Add '.db' arguments
  //

void doAdb(valueSA *value)
{
  if( value->valid)
  {
    if( yyeol && ((value->value < -128) || (value->value > 255)) )
      warning(W_DB_OUT_OF_RANGE, value->value);
    else if( (value->value < -128) || (value->value > 255) )
      warningin(W_DB_OUT_OF_RANGE, value->value);
    if(yysegment == SEGMENT_CODE)
      insertdbdw(value->value & 0xFF);
    else if(yysegment == SEGMENT_EEPROM)
      insertdata(value->value & 0xFF);
  }
  else if(yyparseno && !(value->valid) )
    error(E_UNKNOWN_DB);
  else
  {
    if(yysegment == SEGMENT_CODE)
      insertdbdw(0);
    else if(yysegment == SEGMENT_EEPROM)
      insertdata(value->value);
  }
}

void doAdb(stringSA str)
{
  int c;
  int val;

  str++;

  for(c=0;c<(int)strlen(str)-1; c++)
  {
    if(str[c]=='\\')
    {
      switch (str[c+1])
      {
        case 'n'  : val = '\n'; break;
        case 't'  : val = '\t'; break;
        case 'v'  : val = '\v'; break;
        case 'b'  : val = '\b'; break;
        case 'r'  : val = '\r'; break;
        case 'f'  : val = '\f'; break;
        case 'a'  : val = '\a'; break;
        case '\\' : val = '\\'; break;
        case '\'' : val = '\''; break;
        case '\"' : val = '\"'; break;
        case '\?' : val = '\?'; break;
        case 'X'  :
        case 'x'  :
        {
          val = 0;

          if( isxdigit(str[c+2] ) )
          {
            if(isdigit(str[c+2]))
              val = str[c+2] - '0';
            else
              val = tolower(str[c+2]) - 'a' + 10;
            c++;

            if( isxdigit(str[c+2] ) )
            {
              if(isdigit(str[c+2]))
                val = val*16 + str[c+2] - '0';
              else
                val = val*16 + tolower(str[c+2]) - 'a' + 10;
              c++;
            }
          }
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
          val = str[c+1] - '0';
          if( (str[c+2]>='0') && (str[c+2]<='7'))
          {
            val = val * 8 + str[++c+1] - '0';
            if( (str[c+2]>='0') && (str[c+2]<='7'))
              val = val * 8 + str[++c+1] - '0';
          }

          if(val > 0xFF)
          {
            val = 0;
            warningin(W_OCT_OUT_OF_RANGE);
          }
        } break;

        default   : val = 0   ; errorin(E_UNDEF_ESCAPE_C,str[c+1]);
      }
      c++;
    }
    else
      val = str[c];

    if(yysegment == SEGMENT_CODE)
      insertdbdw(val);
    else if(yysegment == SEGMENT_EEPROM)
      insertdata(val);
  }
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Initialize '.dw' directive
  //

void doAdw(void)
{
  if(yysegment == SEGMENT_CODE)
    yydefinestart = yycodepos;
  else if(yysegment == SEGMENT_EEPROM)
    yydefinestart = yyerompos;
  else
    errorin(E_DW_IN_DSEG);

  yyeol = FALSE;
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Add '.dw' arguments
  //

void doAdw(valueSA *value)
{
  if( value-> valid )
  {
    if(yyeol && ((value->value < -32768) || (value->value > 65535) ) )
      warning(W_DW_OUT_OF_RANGE, value->value);
    else if( (value->value < -32768) || (value->value > 65535)  )
      warningin(W_DW_OUT_OF_RANGE, value->value);

    if(yysegment == SEGMENT_CODE)
    {
      insertdbdw( (value->value & 0x000000FF) >>  0);
      insertdbdw( (value->value & 0x0000FF00) >>  8);
    }
    else if(yysegment == SEGMENT_EEPROM)
    {
      insertdata( (value->value & 0x000000FF) >>  0);
      insertdata( (value->value & 0x0000FF00) >>  8);
    }
  }
  else if(yyparseno)
    error(E_UNKNOWN_DW);
  else
  {
    if(yysegment == SEGMENT_CODE)
    {
      insertdbdw(0);
      insertdbdw(0);
    }
    else if(yysegment == SEGMENT_EEPROM)
    {
      insertdata(0);
      insertdata(0);
    }
  }
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Handle directive '.equ'
  //

void doEqu(symbolSA symb, valueSA *value)
{
  if( symb->valid && (symb->valdefline != yyline-1) )
    error(E_EQU_REDEF, symb->name, symb->valdefline,symb->valfilename);
  else
  {
    symb -> valid      = value -> valid;
    symb -> value      = value -> value;
    symb -> valdefline = yyline - 1;
    symb -> segment    = yysegment;
    strcpy(symb->valfilename, yyfilename);
    if( !value -> valid )
      error(E_UNKNOWN);
  }
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Handle directive '.set'
  //

void doSet(symbolSA symb, valueSA *value)
{
  if( (symb->valid) && (!symb->isvar))
    error(E_LABEL_VAR, symb->name, symb->valdefline, symb->valfilename);
  else
  {
    symb -> isvar       = TRUE;
    symb -> valid       = value -> valid;
    symb -> value       = value -> value;
    symb -> valdefline  = yyline;
    symb -> segment     = yysegment;
    strcpy(symb->valfilename, yyfilename);
  }
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Defines like ".def REG0 = r0"
  //

void doDef(symbolSA symb, regSA reg)
{
  if( symb->isdefine )
  {
    if( (symb->reg != reg) && yyparseno )
    {
      warning(W_REGREDEF ,symb->name,symb->regdefline,symb->regfilename);
      symb -> regdefline  = yyline - 1;
      symb -> reg         = reg;
      strcpy(symb->regfilename, yyfilename);
    }
    if(symb->valid)
      warning(W_REGLABEL ,symb->name,symb->valdefline,symb->valfilename);
  }
  else
  {
    symb -> isdefine    = TRUE;
    symb -> regdefline  = yyline - 1;
    symb -> reg         = reg;
    strcpy(symb->regfilename, yyfilename);
    if(symb->valid)
      warning(W_REGLABEL ,symb->name,symb->valdefline,symb->valfilename);
  }
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Defines like ".def REG0 = IR0" - Notice: Not supported by ATMEL Assembler
  //

void doDef(symbolSA symbl, symbolSA symbr)
{
  if(!yyparseno)
    return;

  if( symbr->isdefine )
  {
    if( symbl->isdefine && (symbl->reg != symbr->reg) )
      warning(W_REGREDEF,symbl->name,symbl->regdefline,symbl->regfilename);
    symbl -> isdefine    = TRUE;
    symbl -> regdefline  = yyline - 1;
    symbl -> reg         = symbr -> reg;
    strcpy(symbl->regfilename, yyfilename);
  }
  else
    error(E_INVALID_REGISTER, symbr->name);
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Add label
  //

void doLab(symbolSA symb)
{
  if(yyinmacro)
  {
    // Check if it is a redef

    if(symb->valid && (
    ((yyoffset    ==symb->offset)&&(symb->segment==SEGMENT_CODE  )) ||
    ((yyeromoffset==symb->offset)&&(symb->segment==SEGMENT_EEPROM)) ||
    ((yydataoffset==symb->offset)&&(symb->segment==SEGMENT_DATA  ))   ) )
    {
      if( (yysegment == SEGMENT_CODE) 
	  && (symb -> value  != yycodepos/2 - yyoffset) )
        errorin(E_LABEL_REDEF,symb->name,symb->valdefline,symb->valfilename);
      else if( (yysegment == SEGMENT_EEPROM) 
	       && (symb -> value  != yyerompos - yyeromoffset) )
        errorin(E_LABEL_REDEF,symb->name,symb->valdefline,symb->valfilename);
      else if( (yysegment == SEGMENT_DATA) 
	       && (symb -> value  != yydatapos - yydataoffset) )
        errorin(E_LABEL_REDEF,symb->name,symb->valdefline,symb->valfilename);
    }

    // No it is not, so we set value if this is first occurence of macro

    else if( !symb->valid )
    {
      symb -> valid      = TRUE;
      symb -> valdefline = yyline;
      symb -> segment    = yysegment;
      symb -> islabel    = 1;
      symb -> macrolabel = 1;

      strcpy(symb->valfilename, yyfilename);

      if(yysegment == SEGMENT_CODE)
      {
        symb -> value   = yycodepos/2 - yyoffset;
        symb -> offset  = yyoffset;
      }
      else if(yysegment == SEGMENT_EEPROM)
      {
        symb -> value   = yyerompos - yyeromoffset;
        symb -> offset  = yyeromoffset;
      }
      else
      {
        symb -> value   = yydatapos - yydataoffset;
        symb -> offset  = yydataoffset;
      }
    }
  }
  else
  {
    if(symb->valid && ((symb->valdefline !=yyline)
                      || strcmp(symb->valfilename, yyfilename)))
      errorin(E_LABEL_REDEF,symb->name,symb->valdefline,symb->valfilename);
    else
    {
      symb -> valid      = TRUE;
      symb -> valdefline = yyline;
      symb -> segment    = yysegment;
      symb -> islabel    = 1;

      strcpy(symb->valfilename, yyfilename);

      if(yysegment == SEGMENT_CODE)
      {
        symb -> value   = yycodepos/2 - yyoffset;
        symb -> offset  = yyoffset;
      }
      else if(yysegment == SEGMENT_EEPROM)
      {
        symb -> value   = yyerompos - yyeromoffset;
        symb -> offset  = yyeromoffset;
      }
      else
      {
        symb -> value   = yydatapos - yydataoffset;
        symb -> offset  = yydataoffset;
      }

      if (symb->name[0] != '@')
      {      
        strcpy(yylast_used_label, symb->name);
      }
    }
  }
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Generate Register / Register opcodes
  //

void genRegReg(int opcode, regSA dest, regSA src)
{
  int   inst    = opcode;

  checkSupported(opcode);

  CHECKREG(src  ,0);
  CHECKREG(dest ,0);

  switch (opcode)
  {
    //////////////////////////////////////////////////////////////////////////

    case O_MULSU  : // Multiply Signed with Unsiged
    case O_FMUL   : // Fractional Multiply
    case O_FMULS  : // Fractional Multiply Signed
    case O_FMULSU : // Fractional Multiply Signed with Unsiged
    {
      CHECKREG(src, 16);
      CHECKREG(dest, 16);

      if( (src<16) || (src>23) )
        error(E_INVALID_REG_EXPECTED, src, 16,23);
      if( (dest<16) || (dest>23) )
        error(E_INVALID_REG_EXPECTED, dest, 16,23);

      inst |= (((src-16)  & 0x0007) << 0) | (((dest-16) & 0x0007) << 4);

    } break;

    //////////////////////////////////////////////////////////////////////////

    case O_MOVW   : // Copy register word
    {
      CHECKREG(src, 0);
      CHECKREG(dest, 0);

      if( src & 1 )
        error(E_INVALID_REG_EXPECTED_0_2_4, src);
      if( dest & 1 )
        error(E_INVALID_REG_EXPECTED_0_2_4, dest);

      inst |= (((src>>1)  & 0x000F) << 0) | (((dest>>1) & 0x000F) << 4);

    } break;

    //////////////////////////////////////////////////////////////////////////

    case O_MULS   : // Multiply Signed
    {
      CHECKREG(src, 16);
      CHECKREG(dest, 16);

      if( (src<16) || (src>31) )
        error(E_INVALID_REG_EXPECTED, dest, 16,31);
      if( (dest<16) || (dest>31) )
        error(E_INVALID_REG_EXPECTED, dest, 16,31);

      inst |= (((src-16)  & 0x000F) << 0) | (((dest-16) & 0x000F) << 4);

    } break;

    //////////////////////////////////////////////////////////////////////////

    case O_CPC    : // Compare with Carry
    case O_CP     : // Compare
    case O_SBC    : // Subtract with Carry
    case O_SUB    : // Subtract without Carry
    case O_ADD    : // without Carry
    case O_ADC    : // with Carry
    case O_CPSE   : // Compares Skip if Equal
    case O_AND    : // Logical AND
    case O_EOR    : // Exclusive OR
    case O_OR     : // Logical OR
    case O_MOV    : // Copy Register
    case O_MUL    : // Multiply
    {
      inst |= (src  & 0x0010) << 5;
      inst |= (src  & 0x000F) << 0;
      inst |= (dest & 0x0010) << 4;
      inst |= (dest & 0x000F) << 4;
    } break;

    //////////////////////////////////////////////////////////////////////////

    default     : internalerror("RR %04X",opcode);
  }
  insertInst(inst, 2);
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Generate Immediate opcodes
  //

void genImmedi(int opcode, valueSA *value)
{
  int   size    = 2;
  int   inst    = opcode;
  int   tmp;

  checkSupported(opcode);

  switch (opcode)
  {
    //////////////////////////////////////////////////////////////////////////

    case O_BRCC : // Branch if Carry Cleared
    case O_BRCS : // Branch if Carry Set
    case O_BRNE : // Branch if Not Equal
    case O_BREQ : // Branch if Equal
    case O_BRPL : // Branch if Plus
    case O_BRMI : // Branch if Minus
    case O_BRVC : // Branch if Overflow Cleared
    case O_BRVS : // Branch if Overflow Set
    case O_BRGE : // Branch if Greater or Equal (Signed)
    case O_BRLT : // Branch if Less Than (Signed)
    case O_BRHC : // Branch if Half C Flag is Cleared
    case O_BRHS : // Branch if Half C Flag is Set
    case O_BRTC : // Branch if the T Flag is Cleared
    case O_BRTS : // Branch if the T Flag is Set
    case O_BRID : // Branch if Global Int is Disabled
    case O_BRIE : // Branch if Global Int is Enabled
    case O_BRSH : // Branch if Same or Higher (BRCC)
    case O_BRLO : // Branch if Lower          (BRCS)
    {
      inst = opcode & 0xFC07;
      if(value->valid)
      {
        tmp = value -> value - yycodepos/2 - 1;

        if(tmp < -64 ) {

	  // Maybe we can do a wrap

	  if(yycfg->wrap) {

	    // We might reach it by jumping forward
	    
	    int dist = DEVINF.flashsize - ( yycodepos/2 + 1) + value->value;

	    if( (dist>63) || (dist<0) )
	      error(E_BRANCH_OUT_OF_RANGE_B,-1*(tmp+64));
	    else
	      inst  |= ((dist & 0x7F ) << 3);
	  }

	  // No wrapping

	  else
	    error(E_BRANCH_OUT_OF_RANGE_B,-1*(tmp+64));
	}
        else if(tmp > 63) {

	  // Maybe we can do a wrap

	  if(yycfg->wrap) {

	    int dist = -1*(DEVINF.flashsize - value->value + yycodepos/2 + 1);

	    if( (dist < -64) || (dist >= 0) )
	      error(E_BRANCH_OUT_OF_RANGE_F,tmp-63);
 	    else
	      inst  |= ((dist & 0x7F ) << 3);
	  }

	  // No wrapping

	  else
	    error(E_BRANCH_OUT_OF_RANGE_F,tmp-63);
	}
        else
          inst  |= ( tmp & 0x7F ) << 3;
      }
      else
        error(E_UNKNOWN_BRANCH);
    } break;

    //////////////////////////////////////////////////////////////////////////

    case O_BCLR : // Bit Clear in SREG
    case O_BSET : // Bit Set in SREG
    {
      if( value->valid )
      {
        if( (value->value < 0) || (value->value > 7) )
          warning(W_BIT_OUT_OF_RANGE, value->value);
        inst  |= (value->value & 7) << 4;
      }
      else
        error(E_UNKNOWN_BIT);
    } break;

    //////////////////////////////////////////////////////////////////////////

    case O_RCALL: // Relative Call To Subroutine
    {
      if(value->valid)
      {
        tmp = value -> value - yycodepos/2 -1;
        if(tmp < -2048 ) {

	  // Maybe we can do a wrap

	  if(yycfg->wrap) {

	    // We might reach it by jumping forward
	    
	    int dist = DEVINF.flashsize - ( yycodepos/2 + 1) + value->value;

	    if( (dist>2047) || (dist<0) )
	      error(E_RCALL_OUT_OF_RANGE_B, -1*(tmp+2048));
	    else
	      inst  |= (dist & 0xFFF);
	  }

	  // No wrapping

	  else
	    error(E_RCALL_OUT_OF_RANGE_B, -1*(tmp+2048));
	}
        else if(tmp > 2047) {

	  // Maybe we can do a wrap

	  if(yycfg->wrap) {

	    int dist = -1*(DEVINF.flashsize - value->value + yycodepos/2 + 1);

	    if( (dist < -2048) || (dist >= 0) )
	      error(E_RCALL_OUT_OF_RANGE_F, tmp-2047);
 	    else
	      inst  |= (dist & 0xFFF);
	  }

	  // No wrapping

	  else
	    error(E_RCALL_OUT_OF_RANGE_F, tmp-2047);
	}

        else
          inst  |=  tmp & 0xFFF ;
      }
      else
        error(E_UNKNOWN_RCALL_DEST);
    } break;

    //////////////////////////////////////////////////////////////////////////

    case O_RJMP : // Relative Jump
    {
      if(value->valid)
      {
        tmp = value -> value - yycodepos/2 -1;
        if(tmp < -2048 ) {

	  // Maybe we can do a wrap

	  if(yycfg->wrap) {

	    // We might reach it by jumping forward
	    
	    int dist = DEVINF.flashsize - ( yycodepos/2 + 1) + value->value;

	    if( (dist>2047) || (dist<0) )
	      error(E_RJUMP_OUT_OF_RANGE_B, -1*(tmp+2048));
	    else
	      inst  |= (dist & 0xFFF);
	  }

	  // No wrapping

	  else
	    error(E_RJUMP_OUT_OF_RANGE_B, -1*(tmp+2048));
	}
        else if(tmp > 2047) {

	  // Maybe we can do a wrap

	  if(yycfg->wrap) {

	    int dist = -1*(DEVINF.flashsize - value->value + yycodepos/2 + 1);

	    if( (dist < -2048) || (dist >= 0) )
	      error(E_RJUMP_OUT_OF_RANGE_F, tmp-2047);
 	    else
	      inst  |= (dist & 0xFFF);
	  }

	  // No wrapping

	  else
	    error(E_RJUMP_OUT_OF_RANGE_F, tmp-2047);
	}
        else
          inst  |=  tmp & 0xFFF ;

      }
      else
        error(E_UNKNOWN_RJUMP_DEST);
    } break;

    //////////////////////////////////////////////////////////////////////////

    case O_JMP  : // Jump
    case O_CALL : // Long Call to a Subroutine
    {
      size   = 4;
      if(value->valid)
      {
        if( (value -> value <0) || (value -> value > 0x3FFFFF) )
          warning(W_ILLEGAL_DEST_ADDR, value->value);

        inst  |=  ( (value -> value & 0x3FFFFF) >> 16) & 0x0001;
        inst  |= (( (value -> value & 0x3F0000) >> 16) & 0x003E) << 3;
        inst  |=  ( (value -> value & 0x00FFFF) << 16);
      }
      else
        error(E_UNKNOWN_JUMP_CALL_DEST);
    } break;

    //////////////////////////////////////////////////////////////////////////

    default     : internalerror("IM %04X",opcode);
  }
  insertInst(inst, size);
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Generate Load / Store opcods - Notice : 'ld' = 'ldd' and 'ld' = 'ldd'
  //

void genIndirc(int opcode, indirectSA *indi, regSA reg, int right)
{
  int   inst    = opcode;
  int   store   = 0x0000;

  checkSupported(opcode);

  CHECKREG(reg, 0);
  CHECKREG(indi->regno, 28);

  switch (opcode)
  {
    //////////////////////////////////////////////////////////////////////////

    case O_ST   : // Store Indirect SRAM index X/Y/Z
    case O_STD  : // Store Indirect SRAM displacement
    {
      store = 0x0200;
    }
    case O_LD   : // Load indirect SRAM index X/Y/Z
    case O_LDD  : // Load indirect SRAM displacement
    {
      if( ((opcode==O_LD) || (opcode==O_LDD)) && !right )
	error(E_REG_DISP);
      if( indi->disp && (indi->regno==26) )
	error(E_NO_DISP);
      else if( ((opcode==O_ST) || (opcode==O_STD)) && right )
	error(E_DISP_REG);
      else
      {
        if( indi -> regno == 26 )
          inst = 0x900C | (reg << 4) | indi->plus;
        else if( (indi -> regno == 28) || (indi -> regno == 30 ) )
        {
          if(indi -> disp)
          {
            inst = 0x8000 | (reg<<4) | (8*(indi -> regno == 28));
            if(indi->offset.valid)
            {
              if( (indi->offset.value<0) || (indi->offset.value>63) )
                warning(W_DISPLACEMENT_OUT_OF_RANGE, indi->offset.value);
              inst |= (indi->offset.value & 0x20) << 8;
              inst |= (indi->offset.value & 0x18) << 7;
              inst |= (indi->offset.value & 0x07) << 0;
            }
            else
              error(E_UNKNOWN_DISP);
          }
          else
          {
            inst  = 0x8000|(reg<<4)|indi->plus|(0x1000*(indi->plus!=0));
            inst |= 8*(indi -> regno == 28) ;
          }
        }
        else
        {
          if(store)
            error(E_INVALID_SOURCE_REG_XYZ, indi->regno);
          else
            error(E_INVALID_DEST_REG_XYZ, indi->regno);
        }
        inst |= store;
      }
    } break;

    //////////////////////////////////////////////////////////////////////////

    default     : internalerror("LS %04X", opcode);

  }
  insertInst(inst, 2);
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Generate Immediate / Register opcodes
  //

void genImmReg(int opcode, valueSA *value, regSA reg)
{
  int   size    = 2;
  int   inst    = opcode;

  checkSupported(opcode);

  CHECKREG(reg, 0);

  switch (opcode)
  {
    //////////////////////////////////////////////////////////////////////////

    case O_OUT  : // Store Register to I/O Port
    {
      inst |= reg << 4;

      if(value->valid)
      {
        if( (value->value<0) || (value->value>63) )
          warning(W_PORT_OUT_OF_RANGE_3F, value->value);
        inst |= (value->value & 0x30) << 5;
        inst |=  value->value & 0x0F;
      }
      else
        error(E_UNKNOWN_PORT);

    } break;

    //////////////////////////////////////////////////////////////////////////

    case O_STS  : // Store direct to SRAM
    {
      size   = 4;
      inst  |= reg << 4;

      if(value->valid)
      {
        if( (value->value<0) || (value->value>0xFFFF) )
          warning(W_ADDR_OUT_OF_RANGE_FFFF, value->value);
        inst |=  (value -> value & 0xFFFF) << 16;
      }
      else
        error(E_UNKNOWN_ADDRESS);
    } break;

    //////////////////////////////////////////////////////////////////////////

    default     : internalerror("IR %04X",opcode);
  }
  insertInst(inst, size);
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Generate Register / Immediate opcodes
  //

void genRegImm(int opcode, regSA reg, valueSA *value)
{
  int   size    = 2;
  int   inst    = opcode;

  checkSupported(opcode);

  switch (opcode)
  {
    //////////////////////////////////////////////////////////////////////////

    case O_SBRC : // Skip if Bit in Register is Cleared
    case O_SBRS : // Skip if Bit in Register is Set
    {
      CHECKREG(reg, 0);

      inst |= reg << 4;

      if(value->valid)
      {
        if( (value->value < 0) || (value->value > 7) )
          warning(W_BIT_OUT_OF_RANGE, value->value);
        inst |= value->value & 7;
      }
      else
        error(E_UNKNOWN_BIT);
    } break;

    //////////////////////////////////////////////////////////////////////////

    case O_IN   : // Load an I/O Port to register
    {
      CHECKREG(reg, 0);

      inst |= reg << 4;

      if(value->valid)
      {
        if( (value->value < 0) || (value->value > 0x3F) )
          warning(W_PORT_OUT_OF_RANGE_3F, value->value);
        inst |= (value->value & 0x30) << 5;
        inst |= (value->value & 0x0F);
      }
      else
        error(E_UNKNOWN_PORT);
    } break;

    //////////////////////////////////////////////////////////////////////////

    case O_CBR  : // Clear Bits in Reg. (Same as 'andi')
    {
      CHECKREG(reg, 16);

      inst = 0x7000;

      if( (reg<16) || (reg>31) )
        error(E_INVALID_REGISTER_R16_R31);
      else
      {
        inst |= (reg-16) << 4;

        if(value->valid)
        {
          if( (value->value < 0) || (value->value > 0xFF) )
            warning(W_BITMASK_OUT_OF_RANGE, value->value);
          inst |= ((0xFF - (value->value & 0xFF)) & 0xF0) << 4;
          inst |= ((0xFF - (value->value & 0xFF)) & 0x0F);
        }
        else
          error(E_UNKNOWN_BITMASK);
      }
    } break;

    //////////////////////////////////////////////////////////////////////////

    case O_BLD  : // Bit Load from T to Register bit
    case O_BST  : // Bit store from Register to T
    {
      CHECKREG(reg, 0);

      inst |= reg << 4;

      if(value->valid)
      {
        if( (value->value < 0) || (value->value > 7) )
          warning(W_BIT_OUT_OF_RANGE, value->value);
        inst |= value->value & 7;
      }
      else
        error(E_UNKNOWN_BIT);
    } break;

    //////////////////////////////////////////////////////////////////////////

    case O_ADIW : // Add Immediate to Word
    case O_SBIW : // Subtract Immediate from Word
    {
      CHECKREG(reg, 24);

      if( (reg!=24) && (reg!=26) && (reg!=28) && (reg!=30) )
        error(E_INVALID_REG_R24_R30);
      else
      {
        inst |= (reg-24) << 3;

        if(value->valid)
        {
          if( (value->value < 0) || (value->value >= 0x40) )
            warning(W_CONST_OUT_OF_RANGE_3F, value->value);
          inst |= (value->value & 0x30) << 2;
          inst |= (value->value & 0x0F);
        }
        else
          error(E_UNKNOWN_CONSTANT);
      }
    } break;

    //////////////////////////////////////////////////////////////////////////

    case O_SBR  : // Set Bits in register (Same as ORI)
    { inst = O_ORI; }
    case O_CPI  : // Compare with Immediate
    case O_SBCI : // Subtract Immediate with Carry
    case O_SUBI : // Subtract Immediate
    case O_ORI  : // Logical OR with Immediate
    case O_ANDI : // Logical AND with Immediate
    case O_LDI  : // Load Immediate
    {
      CHECKREG(reg, 16);

      if( (reg<16) || (reg>31) )
        error(E_INVALID_REGISTER_R16_R31);
      else
      {
        inst |= (reg-16) << 4;

        if(value->valid)
        {
          if( (value->value < -0xFF) || (value->value > 0xFF) )
            warning(W_CONST_OUT_OF_RANGE_FF, value->value);
          inst |= (value->value & 0xF0) << 4;
          inst |= (value->value & 0x0F);
        }
        else
          error(E_UNKNOWN_CONSTANT);
      }
    } break;

    //////////////////////////////////////////////////////////////////////////

    case O_LDS  : // Load direct from SRAM
    {
      CHECKREG(reg, 0);

      size   = 4;
      inst  |= reg << 4;
      if(value->valid)
      {
        if( (value->value < 0) || (value->value > 0xFFFF) )
          warning(W_CONST_OUT_OF_RANGE_FFFF, value->value);
        inst |=  ((value -> value)&0xFFFF) << 16;
      }
      else
        error(E_UNKNOWN_CONSTANT);
    } break;

    //////////////////////////////////////////////////////////////////////////

    default     : internalerror("RI %04X",opcode);
  }
  insertInst(inst, size);
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Generate Immediate / Immediate opcodes
  //

void genImmImm(int opcode, valueSA *valued, valueSA *values)
{
  int   inst    = opcode;
  int   tmp;

  checkSupported(opcode);

  switch (opcode)
  {
    //////////////////////////////////////////////////////////////////////////

    case O_BRBC : // Branch if Bit in SREG is Cleared
    case O_BRBS : // Branch if Bit in SREG is Set
    {

      if( valued->valid )
      {
        if( (valued->value < 0) || (valued->value > 7) )
          warning(W_BIT_OUT_OF_RANGE ,valued->value);
        inst |= valued->value & 7;
      }
      else
        error(E_UNKNOWN_BIT);

      if(values->valid)
      {

        tmp = values -> value - yycodepos/2 - 1;

        if(tmp < -64 ) {

	  // Maybe we can do a wrap

	  if(yycfg->wrap) {

	    // We might reach it by jumping forward
	    
	    int dist = DEVINF.flashsize - ( yycodepos/2 + 1) + values->value;

	    if( (dist>63) || (dist<0) )
	      error(E_BRANCH_OUT_OF_RANGE_B,-1*(tmp+64));
	    else
	      inst  |= ((dist & 0x7F ) << 3);
	  }

	  // No wrapping

	  else
	    error(E_BRANCH_OUT_OF_RANGE_B,-1*(tmp+64));
	}
        else if(tmp > 63) {

	  // Maybe we can do a wrap

	  if(yycfg->wrap) {

	    int dist = -1*(DEVINF.flashsize - values->value + yycodepos/2 + 1);

	    if( (dist < -64) || (dist >= 0) )
	      error(E_BRANCH_OUT_OF_RANGE_F,tmp-63);
 	    else
	      inst  |= ((dist & 0x7F ) << 3);
	  }

	  // No wrapping

	  else
	    error(E_BRANCH_OUT_OF_RANGE_F,tmp-63);
	}
        else
          inst  |= ( tmp & 0x7F ) << 3;

      }
      else
        error(E_UNKNOWN_BRANCH);
    } break;

    //////////////////////////////////////////////////////////////////////////

    case O_CBI  : // Clear Bit in I/O Register
    case O_SBI  : // Set Bit in I/O Register
    case O_SBIC : // Skip if Bit in I/O is Cleared
    case O_SBIS : // Skip if Bit in I/O is Set
    {
      if( valued->valid )
      {
        if( (valued->value < 0) || (valued->value > 31) )
          warning(W_PORT_OUT_OF_RANGE_1F, valued->value);
        inst |= ((valued->value)&0x1F) << 3;
      }
      else
        error(E_UNKNOWN_PORT);

      if( values->valid )
      {
        if( (values->value < 0) || (values->value > 7) )
          warning(W_BIT_OUT_OF_RANGE,values->value);
        inst |= values->value & 7;
      }
      else
        error(E_UNKNOWN_BIT);
    } break;

    //////////////////////////////////////////////////////////////////////////

    default     : internalerror("II %04X",opcode);
  }
  insertInst(inst, 2);
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Generate no argument opcodes
  //

void genNoargs(int opcode)
{
  int   inst    = opcode;

  checkSupported(opcode);

  switch (opcode)
  {
    //////////////////////////////////////////////////////////////////////////

    case O_SEC    : // Set Carry Flag
    case O_SEZ    : // Set Zero Flag
    case O_SEN    : // Set Negative Flag
    case O_SEV    : // Set Overflow Flag
    case O_SES    : // Set Signed Flag
    case O_SEH    : // Set Half Carry Flag
    case O_SET    : // Set T Flag
    case O_SEI    : // Set Global Interrupt Flag
    case O_CLC    : // Clear Carry Flag
    case O_CLZ    : // Clear Zero Flag
    case O_CLN    : // Clear Negative Flag
    case O_CLV    : // Clear Overflow Flag
    case O_CLS    : // Clear Signed Flag
    case O_CLH    : // Clear Half Carry Flag
    case O_CLT    : // Clear T Flag
    case O_CLI    : // Clear Global Interrupt Flag
    case O_NOP    : // No Operation
    case O_ICALL  : // Indirect Call to Subroutine
    case O_IJMP   : // Indirect Jump
    case O_RETI   : // Return from Interrupt
    case O_RET    : // Return from Subroutine
    case O_SLEEP  : // Sleep
    case O_WDR    : // Wathcdog Reset
    case O_EIJMP  : // Extended indirect jump
    case O_EICALL : // Extended indirect call to subroutine
    case O_ESPM   : // Extended store program memory
    case O_SPM	  : // Store program memory
    {
    } break;

    //////////////////////////////////////////////////////////////////////////

    default     : internalerror("NA %04X",opcode);
  }
  insertInst(inst, 2);
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Generate Register only opcodes
  //

void genRegist(int opcode, regSA reg)
{
  int   inst    = opcode;

  checkSupported(opcode);

  switch (opcode)
  {
    //////////////////////////////////////////////////////////////////////////

    case O_ROR  : // Rotate Right Trough Carry
    case O_COM  : // One's Complement
    case O_NEG  : // Two's Complement
    case O_SWAP : // Swap Nibbles
    case O_INC  : // Increment
    case O_ASR  : // Arithmetic Shift Right
    case O_LSR  : // Logical Shift Right
    case O_DEC  : // Decrement
    case O_POP  : // Pop Register from Stack
    case O_PUSH : // Push Register on Stack
    {
      CHECKREG(reg, 0);
      inst |= reg << 4;
    } break;

    //////////////////////////////////////////////////////////////////////////

    case O_TST  : // Test for Zero or Minus
    case O_CLR  : // Logical Shift Left (ADD Rd,Rd)
    {
      CHECKREG(reg, 0);
      inst  = opcode & 0x2400;
      inst |= (reg  & 0x10) << 5;
      inst |= (reg  & 0x0F) << 0;
      inst |= (reg  & 0x10) << 4;
      inst |= (reg  & 0x0F) << 4;
    } break;

    //////////////////////////////////////////////////////////////////////////

    case O_ROL  : // Rotate Left trough C (ADC Rd,Rd)
    case O_LSL  : // Logical Shift Left (ADD Rd,Rd)
    {
      CHECKREG(reg, 0);
      inst  = opcode & 0x1C00;
      inst |= (reg  & 0x10) << 5;
      inst |= (reg  & 0x0F) << 0;
      inst |= (reg  & 0x10) << 4;
      inst |= (reg  & 0x0F) << 4;
    } break;

    //////////////////////////////////////////////////////////////////////////

    case O_SER  : // Set all bits in Register
    {
      CHECKREG(reg, 16);
      if( (reg<16) || (reg>31) )
        error(E_INVALID_REGISTER_R16_R31);
      else
        inst |= reg << 4;

    } break;

    //////////////////////////////////////////////////////////////////////////

    default     : internalerror("RO %04X",opcode);
  }
  insertInst(inst, 2);
}


  ////////////////////////////////////////////////////////////////////////////
  //
  // Generate 'Load Program Memory' instructions
  //

void genLpm(int opcode, regSA reg, indirectSA *indi, int useregs)
{
  int inst = opcode;

  // Generate: (E)LPM rX, Z(+)

  if(useregs) {

    // Check that no displacement is specified

    if(indi->disp)
      error(E_DISP_ILLEGAL);

    // Check that Z is specified

    CHECKREG(indi->regno, 30);

    if(indi->regno != 30)
      error(E_INVALID_REG_Z_EXPECTED);

    // Check that pre-increment is not used

    if( indi->plus == 2)
      error(E_INVALID_PREINCREMENT);

    if(opcode==O_LPM)
      inst = 0x9004 + (indi->plus==1 ? 1 : 0) + ((reg&0x1f)<<4);
    else
      inst = 0x9006 + (indi->plus==1 ? 1 : 0) + ((reg&0x1f)<<4);
  }

  insertInst(inst, 2);
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Handle functions
  //

void genFun(functionSA func, valueSA *value, valueSA *res)
{
  if( !value->valid )
  {
    res -> valid = FALSE;
    res -> value = 0;
  }
  else
  {
    res -> valid = TRUE;
    res -> value = 0;
    switch(func)
    {
      case OP_LOW       : res->value = (value->value & 0x000000FF)>> 0; break;
      case OP_HIGH      : res->value = (value->value & 0x0000FF00)>> 8; break;
      case OP_BYTE2     : res->value = (value->value & 0x0000FF00)>> 8; break;
      case OP_BYTE3     : res->value = (value->value & 0x00FF0000)>>16; break;
      case OP_BYTE4     : res->value = (value->value & 0xFF000000)>>24; break;
      case OP_LWRD      : res->value = (value->value & 0x0000FFFF)>> 0; break;
      case OP_HWRD      : res->value = (value->value & 0xFFFF0000)>>16; break;
      case OP_PAGE      : res->value = (value->value & 0x003F0000)>>16; break;
      case OP_LOG2      :
      {
        if( value->value < 0 )
          warningin(W_INTEGERRESULT_LOG2_IS_ZERO, value->value);
        else
          res->value = (int) (0.5+log10(value->value)/log10(2));
      } break;
      case OP_EXP2      :
      {
        if( (value->value > 31) || (value->value < 0) )
          warningin(W_INTEGERRESULT_EXP2_IS_ZERO, value->value);
        else
          res->value = 1<< value->value;
      } break;
      default           : internalerror("FU %08X",func);
    }
  }
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Handle unary operators
  //

void oprUna(int opr, valueSA *value, valueSA *res)
{
  if( !value->valid )
  {
    res -> valid = FALSE;
    res -> value = 0;
  }
  else
  {
    res -> valid = TRUE;
    res -> value = 0;
    switch(opr)
    {
      case OP_MINUS : ; res->value = -1*value->value; break;
      case OP_WAVE  : ; res->value =   ~value->value; break;
      case OP_NOT   : ; res->value =   !value->value; break;
      default       : internalerror("UO %08X", opr);
    }
  }
}

  ////////////////////////////////////////////////////////////////////////////
  //
  // Handle binary operators
  //

void oprBin(valueSA *vl, int opr, valueSA *vr, valueSA *res)
{
  if( !vl->valid || !vr->valid )
  {
    res -> valid = FALSE;
    res -> value = 0;
  }
  else
  {
    res -> valid = TRUE;
    res -> value = 0;

    switch(opr)
    {
      case OP_STAR   : res->value = ( vl->value *  vr->value ); break;
      case OP_MOD    : res->value = ( vl->value %  vr->value ); break;
      case OP_PLUS   : res->value = ( vl->value +  vr->value ); break;
      case OP_MINUS  : res->value = ( vl->value -  vr->value ); break;
      case OP_LS     : res->value = ( vl->value << vr->value ); break;
      case OP_RS     : res->value = ( vl->value >> vr->value ); break;
      case OP_LESS   : res->value = ( vl->value <  vr->value ); break;
      case OP_GREAT  : res->value = ( vl->value >  vr->value ); break;
      case OP_LE     : res->value = ( vl->value <= vr->value ); break;
      case OP_GE     : res->value = ( vl->value >= vr->value ); break;
      case OP_EQ     : res->value = ( vl->value == vr->value ); break;
      case OP_NE     : res->value = ( vl->value != vr->value ); break;
      case OP_AND    : res->value = ( vl->value &  vr->value ); break;
      case OP_XOR    : res->value = ( vl->value ^  vr->value ); break;
      case OP_OR     : res->value = ( vl->value |  vr->value ); break;
      case OP_ANDAND : res->value = ( vl->value && vr->value ); break;
      case OP_OROR   : res->value = ( vl->value || vr->value ); break;
      case OP_DIV    :
      {
        if(vr->value)
          res->value = ( vl->value /  vr->value );
        else
        {
          errorin(E_DIVISION_BY_ZERO);
          res->valid = FALSE;
        }
      } break;
      default        : internalerror("BO %08X",opr);
    }
  }
}

/// END OF FILE //////////////////////////////////////////////////////////////
