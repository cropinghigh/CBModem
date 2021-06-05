/////////////////////////////////////////////////////////////////////////////
//
//  File        : opcodes.hh
//
//  Author      : Tom Mortensen - Copyright (C) 1999
//
//  Description : ATMEL AVR opcodes
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

#ifndef _OPCODES_HH_
#define _OPCODES_HH_

/// Include //////////////////////////////////////////////////////////////////

#include "avrasm.hh"

//////////////////////////////////////////////////////////////////////////////
//
// Branch instructions   - [1111 0#-- ---- -###]
//

#define O_BRCC      0xF400  // i  Branch if Carry Cleared
#define O_BRCS      0xF000  // i  Branch if Carry Set
#define O_BRNE      0xF401  // i  Branch if Not Equal
#define O_BREQ      0xF001  // i  Branch if Equal
#define O_BRPL      0xF402  // i  Branch if Plus
#define O_BRMI      0xF002  // i  Branch if Minus
#define O_BRVC      0xF403  // i  Branch if Overflow Cleared
#define O_BRVS      0xF003  // i  Branch if Overflow Set
#define O_BRGE      0xF404  // i  Branch if Greater or Equal (Signed)
#define O_BRLT      0xF004  // i  Branch if Less Than (Signed)
#define O_BRHC      0xF405  // i  Branch if Half Carry Flag is Cleared
#define O_BRHS      0xF005  // i  Branch if Half Carry Flag is Set
#define O_BRTC      0xF406  // i  Branch if the T Flag is Cleared
#define O_BRTS      0xF006  // i  Branch if the T Flag is Set
#define O_BRID      0xF407  // i  Branch if Global Interrupt is Disabled
#define O_BRIE      0xF007  // i  Branch if Global Interrupt is Enabled
#define O_BRSH   (8+0xF400) // i  Branch if Same or Higher (Unsigned)(BRCC)
#define O_BRLO   (8+0xF000) // i  Branch if Lower                    (BRCS)

//////////////////////////////////////////////////////////////////////////////
//
// Noarg instructions    - [1001 0100 #### 0100]
//

#define O_SEC       0x9408  //    Set Carry Flag
#define O_SEZ       0x9418  //    Set Zero Flag
#define O_SEN       0x9428  //    Set Negative Flag
#define O_SEV       0x9438  //    Set Overflow Flag
#define O_SES       0x9448  //    Set Signed Flag
#define O_SEH       0x9458  //    Set Half Carry Flag
#define O_SET       0x9468  //    Set T Flag
#define O_SEI       0x9478  //    Set Global Interrupt Flag

#define O_CLC       0x9488  //    Clear Carry Flag
#define O_CLZ       0x9498  //    Clear Zero Flag
#define O_CLN       0x94A8  //    Clear Negative Flag
#define O_CLV       0x94B8  //    Clear Overflow Flag
#define O_CLS       0x94C8  //    Clear Signed Flag
#define O_CLH       0x94D8  //    Clear Half Carry Flag
#define O_CLT       0x94E8  //    Clear T Flag
#define O_CLI       0x94F8  //    Clear Global Interrupt Flag

#define O_NOP       0x0000  //    No Operation
#define O_ICALL     0x9509  //    Indirect Call to Subroutine
#define O_IJMP      0x9409  //    Indirect Jump

#define O_RETI      0x9518  //    Return from Interrupt
#define O_RET       0x9508  //    Return from Subroutine
#define O_SLEEP     0x9588  //    Sleep
#define O_WDR       0x95A8  //    Wathcdog Reset

#define O_EIJMP     0x9419  //    Extended indirect jump                  (1)
#define O_EICALL    0x9519  //    Extended indirect call to subroutine    (1)
#define O_ESPM      0x95F8  //    Extended store program memory           (1)
#define O_SPM       0x95E8  //    Store program memory                    (1)

//////////////////////////////////////////////////////////////////////////////
//
// Reg/Reg instructions  - [#-## ##-- ---- ----]
//

#define O_CPC       0x0400  // rr Compare with Carry
#define O_CP        0x1400  // rr Compare
#define O_SBC       0x0800  // rr Subtract with Carry
#define O_SUB       0x1800  // rr Subtract without Carry
#define O_ADD       0x0C00  // rr Add without Carry
#define O_ADC       0x1C00  // rr Add with Carry
#define O_CPSE      0x1000  // rr Compares Skip if Equal
#define O_AND       0x2000  // rr Logical AND
#define O_EOR       0x2400  // rr Exclusive OR
#define O_OR        0x2800  // rr Logical OR
#define O_MOV       0x2C00  // rr Copy Register
#define O_MUL       0x9C00  // rr Multiply
#define O_MOVW      0x0100  // rr Copy register word	                  (1)
#define O_MULS      0x0200  // rr Multiply Signed                         (1)
#define O_MULSU     0x0300  // rr Multiply Signed with Unsiged            (1)
#define O_FMUL      0x0308  // rr Fractional Multiply                     (1)
#define O_FMULS     0x0380  // rr Fractional Multiply Signed              (1)
#define O_FMULSU    0x0388  // rr Fractional Multiply Signed with Unsiged (1)

//////////////////////////////////////////////////////////////////////////////
//
// Reg/Imm instructions  - [#-## ##-- ---- ----]
//

  // Reg/Imm word

#define O_ADIW      0x9600  // rw Add Immediate to Word
#define O_SBIW      0x9700  // rw Subtract Immediate to Word

  // Reg/imm              - [#### ---- ---- ----]

#define O_CPI       0x3000  // ri Compare with Immediate
#define O_SBCI      0x4000  // ri Subtract Immediate with Carry
#define O_SUBI      0x5000  // ri Subtract Immediate
#define O_ORI       0x6000  // ri Logical OR with Immediate
#define O_ANDI      0x7000  // ri Logical AND with Immediate
#define O_LDI       0xE000  // ri Load Immediate
#define O_SBR    (1+0x6000) // ri Set Bits in register (Same as ORI)

//////////////////////////////////////////////////////////////////////////////
//
// Register instructions - [---- ---- ---- ----]
//

#define O_COM       0x9400  // r  One's Complement
#define O_NEG       0x9401  // r  Two's Complement
#define O_SWAP      0x9402  // r  Swap Nibbles
#define O_INC       0x9403  // r  Increment
#define O_ASR       0x9405  // r  Arithmetic Shift Right
#define O_LSR       0x9406  // r  Logical Shift Right
#define O_ROR       0x9407  // r  Rotate Right Trough Carry
#define O_DEC       0x940A  // r  Decrement
#define O_POP       0x900F  // r  Pop Register from Stack
#define O_PUSH      0x920F  // r  Push Register on Stack
#define O_CLR    (1+0x2400) // r  Clear Regiser (Same as EOR Rd, Rd)
#define O_TST       0x2000  // r  Test for Zero or Minus

//////////////////////////////////////////////////////////////////////////////
//
// immediate instructions - [---- ---- ---- ----]
//

#define O_BCLR      0x9488  // i  Bit Clear in SREG
#define O_BSET      0x9408  // i  Bit Set in SREG
#define O_RCALL     0xD000  // i  Relative Call To Subroutine
#define O_RJMP      0xC000  // i  Relative Jump
#define O_BRBC      0xF400  // ii Branch if Bit in SREG is Cleared
#define O_BRBS      0xF000  // ii Branch if Bit in SREG is Set
#define O_CBI       0x9800  // ii Clear Bit in I/O Register
#define O_SBI       0x9A00  // ii Set Bit in I/O Register
#define O_SBIC      0x9900  // ii Skip if Bit in I/O Register is Cleared
#define O_SBIS      0x9B00  // ii Skip if Bit in I/O Register is Set
#define O_OUT       0xB800  // ir Store Register to I/O Port
#define O_BLD       0xF800  // ri Bit Load from T in Sreg to bit in Register
#define O_BST       0xFA00  // ri Bit store from Bit in Register to T in SREG
#define O_CBR    (1+0x7000) // ri Clear Bits in Register (Same as ANDI)
#define O_IN        0xB000  // ri Load an I/O Port to register
#define O_SBRC      0xFC00  // ri Skip if Bit in Register is Cleared
#define O_SBRS      0xFE00  // ri Skip if Bit in Register is Set
#define O_ROL    (1+0x1C00) // r  Rotate Left trough Carry (Same as ADC Rd,Rd)
#define O_SER       0xEF0F  // r  Set all bits in Register
#define O_LSL    (1+0x0C00) // r  Logical Shift Left (Same as ADD Rd,Rd)
#define O_JMP       0x940C  // i  Jump
#define O_CALL      0x940E  // i  Long Call to a Subroutine
#define O_LDS       0x9000  // rl Load direct from SRAM
#define O_STS       0x9200  // lr Store direct to SRAM

#define O_LD        0x8000  // rX Load indirect from SRAM using index X/Y/Z
#define O_ST        0x8200  // Xr Store Indirect to SRAM using index X/Y/Z
#define O_LDD    (8+0x8000) // rD Load indirect from SRAM using displacement
#define O_STD    (8+0x8200) // Dr Store Indirect to SRAM using displacement

#define O_STD    (8+0x8200) // Dr Store Indirect to SRAM using displacement

//////////////////////////////////////////////////////////////////////////////
//
// (E)LPM instructions    - [---- ---- ---- ----]
//

#define O_LPM       0x95C8  //    Load Program Memory
#define O_ELPM      0x95D8  // __ Extended load program memory            (1)







#endif /* _OPCODES_HH_ */

/// END OF FILE //////////////////////////////////////////////////////////////

