/* A Bison parser, made by GNU Bison 3.7.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_AVRPARSE_H_INCLUDED
# define YY_YY_AVRPARSE_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    STAR = 258,                    /* STAR  */
    DIV = 259,                     /* DIV  */
    MOD = 260,                     /* MOD  */
    LS = 261,                      /* LS  */
    RS = 262,                      /* RS  */
    LE = 263,                      /* LE  */
    GE = 264,                      /* GE  */
    LESS = 265,                    /* LESS  */
    GREAT = 266,                   /* GREAT  */
    EQ = 267,                      /* EQ  */
    NE = 268,                      /* NE  */
    OR = 269,                      /* OR  */
    XOR = 270,                     /* XOR  */
    AND = 271,                     /* AND  */
    OROR = 272,                    /* OROR  */
    ANDAND = 273,                  /* ANDAND  */
    LPAR = 274,                    /* LPAR  */
    RPAR = 275,                    /* RPAR  */
    COLON = 276,                   /* COLON  */
    COMMA = 277,                   /* COMMA  */
    DOT = 278,                     /* DOT  */
    EQUAL = 279,                   /* EQUAL  */
    PLUS = 280,                    /* PLUS  */
    MINUS = 281,                   /* MINUS  */
    WAVE = 282,                    /* WAVE  */
    NOT = 283,                     /* NOT  */
    EOL = 284,                     /* EOL  */
    RESTART = 285,                 /* RESTART  */
    ENDOFFILE = 286,               /* ENDOFFILE  */
    DEF = 287,                     /* DEF  */
    EQU = 288,                     /* EQU  */
    DB = 289,                      /* DB  */
    DW = 290,                      /* DW  */
    ORG = 291,                     /* ORG  */
    ALIGN = 292,                   /* ALIGN  */
    CSEG = 293,                    /* CSEG  */
    DSEG = 294,                    /* DSEG  */
    ESEG = 295,                    /* ESEG  */
    BYTE = 296,                    /* BYTE  */
    SET = 297,                     /* SET  */
    DEVICE = 298,                  /* DEVICE  */
    STRING = 299,                  /* STRING  */
    MACRODEF = 300,                /* MACRODEF  */
    REGISTER = 301,                /* REGISTER  */
    REGXYZ = 302,                  /* REGXYZ  */
    SYMBOL = 303,                  /* SYMBOL  */
    INTEGER = 304,                 /* INTEGER  */
    COUNTER = 305,                 /* COUNTER  */
    FUNCTION = 306,                /* FUNCTION  */
    IREGREG = 307,                 /* IREGREG  */
    IREGREGW = 308,                /* IREGREGW  */
    IIMMIMM = 309,                 /* IIMMIMM  */
    IREGIMM = 310,                 /* IREGIMM  */
    IREGIMMW = 311,                /* IREGIMMW  */
    IIMMREG = 312,                 /* IIMMREG  */
    IREG = 313,                    /* IREG  */
    IIMM = 314,                    /* IIMM  */
    INOARGS = 315,                 /* INOARGS  */
    IINDIRC = 316,                 /* IINDIRC  */
    ILPM = 317                     /* ILPM  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 78 "avrparse.y"

  regSA                 regid;
  instSA                inst;
  opcodeSA              opcode;
  valueSA               val;
  nameSA                name;
  symbolSA              symb;
  indirectSA            indi;
  functionSA            func;
  stringSA              string;

#line 138 "avrparse.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_AVRPARSE_H_INCLUDED  */
