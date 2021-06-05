//////////////////////////////////////////////////////////////////////////////
//
//  File        : msg_de.hh
//
//  Author      : Tom Mortensen - Copyright (C) 1999
//
//  Description : German assembler messages
//
//  History
//  ========================================================================
//
//  980127      : Tom - File created - Messages received from Uwe Bonnes.
//  991222      : Kurt- New command line option -x
//  040306      : Dariusz Kowalewski <darekk@automex.pl> added messages for
//                      W_DEF_REDEF, E_DEV_REDEF and E_DEV_AFTER_DSEG_DATA
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

#ifndef _MSG_DE_HH_
#define _MSG_DE_HH_

// Command line info /////////////////////////////////////////////////////////

#define HELPSTR                                            \
"usage: tavrasm [-vwclxmihjgbafd] <infile> "               \
"               [-I <includepath>]... [-o <outfile>] [-r <rom>] [-e <listfile>]\n" \
"       -v verbose\n"                                      \
"       -w no warnings\n"                                  \
"       -c case sensitive labels/defines\n"                \
"       -l limit log width to 80 characters\n"             \
"       -x allow local labels\n"                           \
"       -m output Motorola S-record format\n"              \
"       -i output Intel HEX format (default)\n"            \
"       -h omit address extension record from Intel HEX files\n" \
"       -j output .obj format\n"                           \
"       -g output generic hex\n"                           \
"       -b output binary format\n"                         \
"       -a wrap relative jumps\n"                          \
"       -f allow forward org's\n"                          \
"       -d list supported devices\n"                           

/// Message strings //////////////////////////////////////////////////////////

#ifdef _UTILS_CC_

char messages[MESSAGE_COUNT][MESSAGES_LEN]=
{
  "Keine Warnung",
  "Ungültiger Character \'%c\' im Quelltext",
  "Ungültiger Character 0x%02X im Quelltext",
  "Argument @%i angegeben, aber nicht im Makro benutzt",
  "Argument @%i nicht angegeben, aber im Makro benutzt",
  "\'%s\' wurde schon in Zeile %i in Routine \'%s als Label definiert\'",
  "\'%s\' wurde schon in Zeile %i in Routine \'%s definiert\'",
  "Ganzzahlergebnis von EXP2(%i) wurde Null gesetzt",
  "Ganzzahlergebnis von LOG2(%i) wurde Null gesetzt",
  "Bit Nummer '\%i\' ist außerhalb des Bereichs (0-7)",
  "I/O Port 0x%X ist außerhalb des Bereichs (0x00-0x1F)",
  "I/O Port 0x%X ist außerhalb des Bereichs (0x00-0x3F)",
  "Bit mask 0x%X ist außerhalb des Bereichs (0x00-0xFF)",
  "Konstante 0x%X ist außerhalb des Bereichs (0x00-0x3F)",
  "Konstante 0x%X ist außerhalb des Bereichs (0x00-0xFF)",
  "Konstante 0x%X ist außerhalb des Bereichs (0x00-0xFFFF)",
  "Adresse 0x%X ist außerhalb des Bereichs (0x0-0xFFFF)",
  "Versatz \'%X\' ist außerhalb des Bereichs (0x00 - 0x3F)",
  "ZielAdresse \'0x%X\' ist außerhalb des Bereichs (0x0-0x3FFFFF)",
  "Wert %i ist außerhalb des Bereichs .db (-128 to 255)",
  "Wert %i ist außerhalb des Bereichs .dw (-32768 to 65535)",
  "Wert %i setzt PC (um %i Worte) im Kode zurück",
  "Wert %i überschreibt EEPROM Daten (um %i Byte(s))",
  "Wert %i überschreibt Daten (um %i Byte(s))",
  "Wert %i überschreibt Register oder I/O Bereich (um %i Byte(s))",
  "SRAM Größe um %i byte(s) überschritten",
  "Unbenutzter Parameter @%i in Makro Definition",
  "Directive '.listmac' nicht unterstützt (ignoriert)",
  "Directive '.list' nicht unterstützt (ignoriert)",
  "Directive '.nolist' nicht unterstützt (ignoriert)",
  "\'%s\' für diesen Baustein nicht unterstützt",
  "Unbekannter Baustein \'%s\'",
  "Konstante zu groß",
  "Bezeichner zu lang",
  "EEPROM Größe um %i byte(s) überschritten",
  "Illegaler oktaler Escapewert",
  "Illegaler hex Escapewert",
  "Oktaler Escapewert außerhalb des Bereichs",
  "Illegale Character Konstante",
  "Flash Speichergröße um %i word(s) überschritten",
  "Macro name too long",
  "Consecutive '.device' directive (\'%s\', ignored)",

  /// Error messages /////////////////////////////////////////////////////////

  "Undefinierte Variable referenziert",
  "Undefinierte Variable referenziert in Bit Nummer",
  "Undefinierte Variable referenziert in I/O Port Adresse",
  "Undefinierte Variable referenziert in Sprung",
  "Undefinierte Variable referenziert in Bitmaske",
  "Undefinierte Variable referenziert in Konstante",
  "Undefinierte Variable referenziert in Adresse",
  "Undefinierte Variable referenziert in Versatz",
  "Undefinierte Variable referenziert in relativen Aufruf",
  "Undefinierte Variable referenziert in relativen Sprung",
  "Undefinierte Variable referenziert in Sprung/Aufruf",
  "Undefinierte Variable referenziert in Konstante in .db",
  "Undefinierte Variable referenziert in Konstante in .dw",
  "Undefinierte Variable referenziert in Offset",
  "Undefinierte Variable referenziert in Konstante in .byte",
  "Division durch Null",
  "Ungültiges register (r16-r31)",
  "Ungültiges register %s",
  "Verzweigung (rückwärts) überschreitet Bereich um %i Worte",
  "Verzweigung (vorwärts) überschreitet Bereich um %i Worte",
  "Ungültiges Register (r24/r26/r28/r30)",
  "Ungültiges Quellregister r%i (X/Y/Z)",
  "Ungültiges Zielregister r%i (X/Y/Z)",
  "Aufruf (rückwärts) überschreitet Bereich um %i Worte",
  "Aufruf (vorwärts) überschreitet Bereich um %i Worte",
  "Sprung (rückwärts) überschreitet Bereich um %i Worte",
  "Sprung (vorwärts) überschreitet Bereich um %i Worte",
  "Label \'%s\' bereits in Zeile %i in \'%s\' definiert",
  "Label \'%s\' bereits in Zeile %i in \'%s\' definiert",
  "Variable \'%s\' bereits in Zeile %i in \'%s\' als Label definiert",
  ".dw in DSEG (nur in CSEG and ESEG zulässig)",
  ".db in DSEG (nur in CSEG and ESEG zulässig)",
  "Makro \'%s\' bereits definiert (oder Label doppelt verwendet)",
  "Programkode in DSEG (nur in CSEG erlaubt)",
  "Programkode in ESEG (nur in CSEG erlaubt)",
  "Include \'%s\' kann nicht verarbeitet werden  - zu viele Includedateien", /*FIXME*/
  "Ungültiger Name für Includedatei",
  "Datei \'%s\' kann nicht geöffnet werden",
  "Makro kann nicht aufgelöst werden - zu viele Makroebenen",/*FIXME*/
  "Dateiende in Makrodefinition erreicht",
  "Dateiname zu lang in Zeile %i in \'%s\'",
  "Ungültiger Dateiname",
  "Register erwartet",
  "Ungültiges Register verwendet",
  "Ungültiges Register oder immediate",/*FIXME*/
  "Ungültige immediate Konstante",/*FIXME*/
  "Unerwartetes Argument",
  "Unbekannter Opcode",
  "ID = register erwartet",
  "Wert (,Wert ,...) erwartet",
  "Argumente dürfen nicht angegeben werden",
  "Wert/Label erwartet",
  "ID = Ausdruck erwartet",
  "Bausteinname erwartet",
  "Variablen Deklaration in CSEG",
  "Macro ohne Name",
  "Undefinierte Escapesequence %s",
  "Kein Dateiname angegeben",
  "Macro vor Definition benutzt",
  "Undefinierte Escapesequence \'\\%c\'",
  ".endmacro Directive nicht erwartet",
  "Macro Definition innerhalb Makro nicht erlaubt",
  "Register, Versatz erwartet",
  "Versatz, Register erwartet",
  "Für X kann kein Versatz angegeben werden ( nur für Y/Z)",
  "Invalid register r%d (expected r23-r31)",
  "Displacement can not be specified here",
  "Invalid register specified, expected 'Z'",
  "Pre-increment is invalid here (use 'Z' or 'Z+')",
  "Invalid register r%d (expected r%d-r%d)",
  "Invalid register r%d (expected r0, r2, ...)",
  "Undefinierte Variable referenziert in Konstante in .align",
  ".align Wert kein Exponent von Zwei",
  "Unexpected '.device' directive after DSEG data (\'%s\')",
  "Multiple '.device' directives ('%s\' and \'%s\')",

  /// Fatal errors ///////////////////////////////////////////////////////////

  "Kein fataler Fehler",
  "Speicherbedarf zu groß",
  "Zeile zu kang in \'%s\'",
  "Makro mit zu langer Zeile in %i in \'%s\'",
  "Eingabedatei kann nicht geöffnet werden \'%s\'",
  "Listdatei kann nicht geöffnet werden \'%s\'",
  "Ausgabedatei kann nicht geöffnet werden\'%s\'",
  "ROM Datei kann nicht geöffnet werden \'%s\'",
  "Zu viele Includedateinen",
  "Kodegröße überschritten",
  "EEPROM Größe überschritten",
  "Makroschachtelung zu tief",

  /// Messages ///////////////////////////////////////////////////////////////

  "Keine Mitteilung",
  "Leere ROM Datei",
  "ROM Datei kann nicht im .obj Formay gespeichert werden",
  "Keine Eingabedatei angegeben",
  "Unbekannte Option '%s'",
  "Mehr Optionen als m/i/j/b/h angegeben",
  "Keine ROM Datei angegeben",
  "Rom Dateiname zu lang",
  "Ungültiger Name für ROM Datei",
  "Keine Listdatei angegeben",
  "Name für Listdatei zu lang",
  "Ungültiger Name für Listdatei",
  "Name für Ausgabedatei fehlt",
  "Name für Ausgabedatei zu lang",
  "Ungültiger Name für Ausgabedatei",
  "Mehrere Namen für Ausgabedatei angegeben",
  "Name für Eingabedatei zu lang",
  "Ungültiger Name für Eingabedatei",
  "Mehrere Namen für Eingabedatei angegeben",
  "Fehler      : %i",
  "Warnungen   : %i",
  "Kode        : %i",
  "Rom         : %i",
  "Data        : %i",
  "Freier Kode : %i",
  "Freies Rom  : %i",
  "Kein Kode erzeugt",
  "| Bausteintype     | Ram Start |Flash Größe |Ram Größe |EEPROM Größe |",
  "No include path found after '-I' flag",
  "Too many include paths specified",
  "Include pathname too long"
};

#endif 

#endif /* _MSG_DE_HH_ */

/// END OF FILE //////////////////////////////////////////////////////////////
