//////////////////////////////////////////////////////////////////////////////
//
//  File        : avrasm.hh
//
//  Author      : Tom Mortensen - Copyright (C) 1999
//
//  Description : AVR assembler header file
//
//  History
//  ========================================================================
//
//  980902      : Tom - File created.
//  990124      : Tom - Added GPL notice.
//  001101	: Brian Rhodefer - Added globals for command-line include search paths
//  010112	: Tom - Increased max id length to 64
//  021218	: Andreas Schwarz <andreas-s@web.de> increased MAX_DEVICES from 13 to 23
//  040306      : Dariusz Kowalewski <darekk@automex.pl> increased MAX_DEVICES to 29
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

#ifndef _AVRASM_HH_
#define _AVRASM_HH_

/// Defines //////////////////////////////////////////////////////////////////

#define FALSE                             0  // No
#define TRUE                           (!0)  // Yes

#define MAX_CODE_SIZE              0x100000  // Code size in bytes
#define MAX_EROM_SIZE               0x10000  // Erom size in bytes
#define MAX_LINE_LENGTH             (255+3)  // Maximum length of input lines
#define MAX_ID_LENGTH                    64  // name defined as MAX_ID_LEN+1
#define MAX_DEVICE_NAME_LENGTH           16  // Like "90S1200"
#define MAX_DEVICES                      29  // Update with "devices.hh"
#define MAX_FILENAME_LENGTH             512  // Should be changed to platform
#define MAX_INCPATH_LENGTH              512  // Should be changed to platform
#define MAX_CONTEXT_DEPTH                32  // Depth of macros/include files
#define MAX_FILES                       127  // Files/includes (don't change)
#define MAX_INCPATH_QTY		          8  // Max number of include search dirs

#define VERSIONSTR                   "1.22"  // Version

#define SEGMENT_CODE                      1  // CSEG
#define SEGMENT_DATA                      2  // DSEG
#define SEGMENT_EEPROM                    3  // ESEG
#define SEGMENT_DEFINE_BYTE               1  // '.byte' in ESEG

#define LIST_NO                           1  // .nolist
#define LIST_NO_MACRO                     2  // .nolistmac
#define LIST_YES_MACRO                    3  // .listmac
#define LIST_YES                          4  // .list

  /// Command line info //////////////////////////////////////////////////////

#define INFOSTR   "Toms AVR macro assembler version "VERSIONSTR" ("__DATE__ \
                  ")\nCopyright (C) 1998-2005 - Tom Mortensen\n"            \
		  "E-mail: tom@tavrasm.org  WWW: http://www.tavrasm.org\n"

/// Supported instructions ///////////////////////////////////////////////////

#define S_ADIW      (1<< 0)
#define S_ICALL     (1<< 1)
#define S_CALL      (1<< 2)
#define S_IJMP      (1<< 3)
#define S_JMP       (1<< 4)
#define S_LD        (1<< 5)
#define S_LDD       (1<< 6)
#define S_LDS       (1<< 7)
#define S_LPM       (1<< 8)
#define S_MUL       (1<< 9)
#define S_PUSH      (1<<10)
#define S_POP       (1<<11)
#define S_SBIW      (1<<12)
#define S_ST        (1<<13)
#define S_STD       (1<<14)
#define S_STS       (1<<15)
#define S_EIJMP     (1<<16)
#define S_EICALL    (1<<17)
#define S_ESPM      (1<<18)
#define S_MULSU     (1<<19)
#define S_FMUL      (1<<20)
#define S_FMULS     (1<<21)
#define S_FMULSU    (1<<22)
#define S_MULS      (1<<23)
#define S_MOVW      (1<<24)
#define S_ELPM      (1<<25)
#define S_SPM       (1<<26)

#define S_EXTENDED  (0xFFFF0000)

/// Macros ///////////////////////////////////////////////////////////////////

#define STRIPNR(s)    {char* pos;if((pos=strpbrk(s,"\r\n"))&&(*pos='\0'));}
#define STRIPWS(s)    {char* pos;if((pos=strpbrk(s,"\v\t \r\n")))*pos='\0';}
#define DEVINF        yydevices[yydeviceno]
#define ISUSED(s)     (s->macro||s->isvar||s->valid||s->isdefine)
#define MSG(m)        messages[MESSAGE_OFFSET_MESSAGES+m]
#define CONTEXT       yycontextstack[yycontext]
#define ISMACRO       (yycontextstack[yycontext]->ismacro)
#define CHECKREG(r,d) { if(r<0) r = d; else if(r>31) internalerror("REG"); } 

/// Globals //////////////////////////////////////////////////////////////////

#define GLOBALS(asextern)                                               \
asextern        int             yyline;                                 \
asextern        int             yyoffset;                               \
asextern        int             yyeromoffset;                           \
asextern        int             yydataoffset;                           \
asextern        int             yycodepos;                              \
asextern        int             yyerompos;                              \
asextern        int             yydatapos;                              \
asextern        int             yycodeposhigh;                          \
asextern        int             yyeromposhigh;                          \
asextern        int             yydataposhigh;                          \
asextern        int             yyparseno;                              \
asextern        int             yyerrorcount;                           \
asextern        int             yyerrorline;                            \
asextern        int             yywarningline;                          \
asextern        int             yywarningcount;                         \
asextern        int             yysegment;                              \
asextern        int             yydefinestart;                          \
asextern        int             yyinmacro;                              \
asextern        int             yycontext;                              \
asextern        int             yydeviceno;                             \
asextern        int             yydefinetype;                           \
asextern        int             yylist;                                 \
asextern        int             yylistmacro;                            \
asextern        int             yyfirstmacroline;                       \
asextern        int             yyinmacrodef;                           \
asextern        int             yyfileno;                               \
asextern        int             yyfilecount;                            \
asextern        int             yyeol;                                  \
asextern        int		yyIncPathCount;                         \
asextern        char            *yylinetxt;                             \
asextern        char            *yyinline;                              \
asextern        char            *yyinlinenew;                           \
asextern        char            *yyinlineold;                           \
asextern        char            *yyfilename;                            \
asextern        char            *yyromfilename;                         \
asextern        char            *yylogfilename;                         \
asextern        char            *yyoutfilename;                         \
asextern        char            *yyfilelist[MAX_FILES];                 \
asextern        char            *yyIncludePaths[MAX_INCPATH_QTY];       \
asextern        unsigned char   *yycodebuf;                             \
asextern        unsigned char   *yyerombuf;                             \
asextern        unsigned char   *yycodeusage;                           \
asextern        unsigned char   *yyeromusage;                           \
asextern        unsigned short  *yycodeline;                            \
asextern        configuration   *yycfg;                                 \
asextern        symbolTable*    symtab;                                 \
asextern        context         *yycontextstack[MAX_CONTEXT_DEPTH];     \
asextern        symbolTable     *yysymbolstack[MAX_CONTEXT_DEPTH];      \
asextern        deviceinfo      yydevices[MAX_DEVICES];                 \
asextern        FILE*           yyin2;                                  \
asextern        FILE*           yylogfile;                              \
asextern        char            yylast_used_label[MAX_ID_LENGTH+1];

/// Init Globals /////////////////////////////////////////////////////////////

#define INITEXTERN                                                      \
{                                                                       \
  yyline                = 1;                                            \
  yyoffset              = 0;                                            \
  yyeromoffset          = 0;                                            \
  yydataoffset          = 0;                                            \
  yycodepos             = 0;                                            \
  yyerompos             = 0;                                            \
  yydatapos             = 96;                                           \
  yycodeposhigh         = 0;                                            \
  yyeromposhigh         = 0;                                            \
  yydataposhigh         = 0;                                            \
  yyerrorcount          = 0;                                            \
  yywarningcount        = 0;                                            \
  yydefinestart         = -1;                                           \
  yysegment             = SEGMENT_CODE;                                 \
  yyinmacro             = FALSE;                                        \
  yycontext             = 0;                                            \
  yyerrorline           =-1;                                            \
  yywarningline         = -1;                                           \
  yydeviceno            = 0;                                            \
  yydefinetype          = 0;                                            \
  yylist                = TRUE;                                         \
  yylistmacro           = FALSE;                                        \
  yyfirstmacroline      = FALSE;                                        \
  yyinmacrodef          = FALSE;                                        \
  yyfileno              = 1;                                            \
  yyeol                 = FALSE;                                        \
  yylast_used_label[0]  = '\0';                                         \
}

/// Configuration ////////////////////////////////////////////////////////////

struct configuration
{
        char            aligndb;        // Allow byte alignment
        char            casesensitive;  // Case sensitive labels/vars
        char            cutlog;         // Limit logfile width to 80 chars
        char            info;           // Show info after compilation
        char            warnings;       // Print warnings
        char            local_labels;   // Allow local loabels
        char            motorola;       // Output Motorola S-record format
        char            intel;          // Output Intel HEX format
        char            obj;            // Output ATMEL .obj format
        bool		hex;		// Output generic hex
        char            bin;            // Output binary format
        bool		wrap;		// Wrap in relative jumps/branches
        bool            forwardorg;     // Allow .org to jump around
        bool		addressext;     // Include address extension record
};

/// Device info //////////////////////////////////////////////////////////////

struct deviceinfo {
        char    name[MAX_DEVICE_NAME_LENGTH+1]; // Device name
        int     datastart;                      // Registers + I/O ports
        int     ramsize;                        // Size of ram
        int     eepromsize;                     // Size of EEPROM
        int     flashsize;                      // Size of flash (in words)
        int     supported;                      // Unsupported instructions
};

/// Types ////////////////////////////////////////////////////////////////////

struct context
{
        // Regular file

        char            filename[MAX_FILENAME_LENGTH];
        int             line;
        FILE            *file;
        FILE            *file2;

        // Macro

        int             ismacro;
        char            *macstr;
        int             offset;
        int             eromoffset;
        int             dataoffset;
        int             stringpos;

        // Line buffers

        char            yyinline[MAX_LINE_LENGTH+1];
        char            yyinlinenew[MAX_LINE_LENGTH+1];
        char            yyinlineold[MAX_LINE_LENGTH+1];
};

/// Prototypes ///////////////////////////////////////////////////////////////

void yyerror(char *s);
int  getargs(int argc, char **args);
int  saveIHF(void);
int  saveGeneric(void);
int  saveObj(void);
int  saveMotorola(void);
int  saveBin(void);

#endif /* _AVRASM_HH_ */

/// END OF FILE //////////////////////////////////////////////////////////////
