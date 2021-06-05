//////////////////////////////////////////////////////////////////////////////
//
//  Archivo     : msg_sp.hh
//
//  Autor       : Lluis Ballester - Copyright (C) 1999
//
//  Descripcion : Spanish assembler messages
//
//  Historia
//  ========================================================================
//
//  980127      : Tom - Archivo creado - Mensajes trasladados desde 'message.hh'.
//  991222      : Kurt- New command line option -x
//  040306      : Dariusz Kowalewski <darekk@automex.pl> added messages for
//                      W_DEF_REDEF, E_DEV_REDEF and E_DEV_AFTER_DSEG_DATA
//
//////////////////////////////////////////////////////// Tom hizo esto ///////
//
// Nota Copyright :
//
// tavrasm - Un ensamblador GNU/Linux para las series  Atmel AVR
// de microcontroladoras. Copyright (C) 1999 Tom Mortensen
//
// Este programa es  software gratuito; puedes redistribuirlo y/o modificarlo
// bajo los terminos de la licencia  publica general GNU
// como fue publicado por  la Free Software Foindation (findacion de Software gratuito); desde la
// version 2 de  la Licencia, o (a tu eleccion) cualquier version posterior.
//
// Este programa es distribuido con la esperanza de que sea totalmente funcional,
// pero SIN NINGuN TIPO DE GARANTiA; sin igualmente la garantia incluida de
// MERCHANTABILITY(mercantibilidad) o FITNESS (propiedad) PARA PROPOSITO 
// PERSONAL.Ver  la  GNU General Public License( Licencia Publica  General ) para mas detalles.
//
// Deberias haber recibido una copia de la GNU ( Licencia Publica General )
// con este programa ; si no lo has recibido, escribe a la:  Free Software
// Fundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// Tom Mortensen
//
// E-mail : tom@tavrasm.org
// WWW    : http://www.tavrasm.org
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _MSG_US_HH_
#define _MSG_US_HH_

// info linea comandos ///////////////////////////////////////////////////////

#define HELPSTR                                            \
"uso: tavrasm [-vwclxmihjgbafd] <en_archivo> \n"           \
"             [-I <includepath>]... [-o <archivo_salida>] [-r <rom>] [-e <listarchivo>]\n" \
"       -v verbose\n"                                      \
"       -w no advertencias\n"                              \
"       -c etiquetas case sensitive /defenes\n"            \
"       -l limite ancho log  a 80 caracters\n"             \
"       -x allow local labels\n"                           \
"       -m Formato de salida Motorola S-record \n"         \
"       -i Formato de salida Intel HEX  (default)\n"             \
"       -h omit address extension record from Intel HEX files\n" \
"       -j Formato de salida .obj \n"                      \
"       -g Salida generica hex\n"                          \
"       -b Formato de salida binaria\n"                    \
"       -a wrap relativa jumps\n"                          \
"       -f allow forward org's\n"                          \
"       -d Lista de los dispositivos soportados\n" 

// Cadenas de  mensajes //////////////////////////////////////////////////////

#ifdef _UTILS_CC_

char messages[MESSAGE_COUNT][MESSAGES_LEN]=
{
  "No advertencias",
  "Caracter invalido \'%c\' en codigo",
  "Caracter invalido 0x%02X en codigo",
  "Argumento @%i especificados, pero no usado en macro",
  "Argumento @%i no especificado, pero usado en macro",
  "\'%s\' ya definida como etiqueta en linea %i en \'%s\'",
  "\'%s\' ya definida en linea %i en \'%s\'",
  "Resultado Integer  de EXP2(%i) esta fijado a cero",
  "Resultado Integer  de LOG2(%i) esta fijado a cero",
  "Numero bit '\%i\' fuera de rango (0-7)",
  "I/O Pot 0x%X fuera de rango (0x00-0x1F)",
  "I/O Pot 0x%X fuera de rango (0x00-0x3F)",
  "Bit mask 0x%X fuera de rango (0x00-0xFF)",
  "Constante 0x%X fuera de rango (0x00-0x3F)",
  "Constante 0x%X fuera de rango (0x00-0xFF)",
  "Constante 0x%X fuera de rango (0x00-0xFFFF)",
  "Direccion 0x%X fuera de rango (0x0-0xFFFF)",
  "Desplazamiento \'%X\' esta fuera de rango (0x00 - 0x3F)",
  "Direccion de destino \'0x%X\' fuera de rango (0x0-0x3FFFFF)",
  "Valor %i fuera de rango en .db (-128 to 255)",
  "Valor %i fuera de rango en .dw (-32768 to 65535)",
  "Valor %i mueve PC atras sobre codigo (by %i wods)",
  "Valor %i sobreescribe informacion EEPROM (by %i byte(s))",
  "Valor %i sobreescribe informacion (by %i byte(s))",
  "Valor %i sobreescribe Registro o I/O area (by %i byte(s))",
  "SRAM tamanyo excedido por  %i byte(s)",
  " Parametro no usado  @%i en definicion de macro",
  "La directiva '.listmac' no es soportada (ignorada)",
  "La directiva '.list' no es soportada  (ignorada)",
  "La directiva '.nolist' is nosuppoted (ignorada)",
  "\'%s\' no soportado por este dispositivo",
  "Dispositivo desconocido \'%s\'",
  "Constante demasiada grande",
  "Identificador demasiado largo",
  "Tamanyo EEPROM excedido por  %i byte(s)",
  "Valor de escape octal ilegal"
  "Valor de escape hexadecimal  ilegal",
  "Valor Octal de escape fuera de rango",
  " Caracter de constante ilegal",
  "Tamanyo flash excedido por %i word(s)",
  "Macro name too long",
  "Consecutive '.device' directive (\'%s\', ignored)",

  /// Mensajes de error //////////////////////////////////////////////////////

  "Variable referenciada indefinida ",
  "Variable referenciada indefinida en numero bit ",
  "Variable referenciada indefinida en I/O puerto direccion",
  "Variable referenciada indefinida en rama",
  "Variable referenciada indefinida en bit mask",
  "Variable referenciada indefinida en constante",
  "Variable referenciada indefinida en direccion",
  "Variable referenciada indefinida en desplazamiento",
  "Variable referenciada indefinida en relative call",
  "Variable referenciada indefinida en relative jump",
  "Variable referenciada indefinida en jump/call",
  "Variable referenciada indefinida en constante en .db",
  "Variable referenciada indefinida en constante en .dw",
  "Variable referenciada indefinida en offset",
  "Variable referenciada indefinida en constante en .byte",
  "Division por cero",
  "Registro invalido (r16-r31)",
  "Registro invalido %s",
  "Rama fuera de rango por %i words (por detras)",
  "Rama fuera de rango por %i words (por delante)",
  "Registro invalido (r24/r26/r28/r30)",
  "Registro de codigo invalido r%i (X/Y/Z)",
  "Registro de destino invalido r%i (X/Y/Z)",
  "Llamada(call)  fuera de rango por %i words (por detras)",
  "Llamada(call) fuera de rango por%i words (por delante)",
  "Jump fuera de rango por %i words (por detras)",
  "Jump fuera de rango por %i words (por delante)",
  "Etiqueta \'%s\' ya definida en linea %i en \'%s\'",
  "Etiqueta \'%s\' ya definida en linea %i en \'%s\'",
  "Variable \'%s\' ya definida como etiqueta en  linea %i en \'%s\'",
  ".dw en DSEG (solo permitido en CSEG y ESEG)",
  ".db en DSEG (solo permitido en  CSEG y ESEG)",
  "Macro \'%s\' ya definida (o etiqueta duplicada)",
  "Codigo de programa en DSEG (solo permitido en CSEG)",
  "Codigo de programa en ESEG (solo permitido en  CSEG)",
  "Imposible  incluir archivo \'%s\' - incluir profundidad(depth) excedida",
  "Nombre de archivo incluido invalido especificado",
  "Imposible abrir archivo \'%s\'",
  "Imposible expandir macro - profundidad contextual (context depth) excedida",
  "Final de archivo alcanzado en definicion de macro",
  "Nombre de archivo demasiado largo en linea  %i en \'%s\'",
  "Invalido nombre de archivo",
  "Registro esperado",
  "Registro invalido especificado",
  "Registro invalido o inmediato",
  "Constante inmediata invalida",
  "No argumento esperado",
  "Desconocido opcodigo",
  "Esperado ID = registro",
  "Esperado valor (,valor ,...) ",
  "No argumentos deben ser especificados",
  "Esperado valor/etiqueta",
  "Esperado ID = expresion",
  "Esperado nombre de dispositivo",
  "Declaracion de variable en CSEG",
  "Macro sin nombre",
  "Secuencia de escape indefinida %s",
  "No nombre de archivo especificado",
  "Macro usada antes definida",
  "Secuencia de escape indefinida \'\\%c\'",
  "Inesperada directiva .finalmacro ",
  "Definicion de macro dentro de macro no permitida",
  "Registro esperado, desplazamiento",
  "Desplazamiento esperado, registro",
  "No desplazamiento pueden ser especificados para X (usar Y/Z)",
  "Invalid register r%d (expected r23-r31)",
  "Displacement can not be specified here",
  "Invalid register specified, expected 'Z'",
  "Pre-increment is invalid here (use 'Z' or 'Z+')",
  "Invalid register r%d (expected r%d-r%d)",
  "Invalid register r%d (expected r0, r2, ...)",
  "Variable referenciada indefinida en constante en .align",
  ".align value not a power of two",
  "Unexpected '.device' directive after DSEG data (\'%s\')",
  "Multiple '.device' directives (\'%s\' and \'%s\')",

  /// Errores fatales ////////////////////////////////////////////////////////

  "No error fatal",
  "Fuera de memoria",
  "Linea demasiada larga en \'%s\'",
  "Linea de macro demasiado larga en linea %i en \'%s\'",
  "Imposible abrir archivo de entrada \'%s\'",
  "Imposible abrir archivo list\'%s\'",
  "Imposible abrir archivo de salida \'%s\'",
  "Imposible abrir archivo rom\'%s\'",
  "Demasiados archivos incluidos",
  "Tamanyo de codigo excedido",
  "Tamanyo EEPROM excedido",
  "Profundidad maxima de macro excedida",

  /// Mensajes ///////////////////////////////////////////////////////////////

  "No mensaje",
  "Archivo rom vacio",
  "No se puede salvar archivo rom con formato .obj",
  "No nombre de archivo de entrada especificado",
  "Opcion desconocida '%s'",
  "Mas de uno de m/i/j/b/h especificados",
  "No nombre de archivo rom especificados",
  "Nombre de archivo rom demasiado largo",
  "Nombre de Archivo rom ilegal",
  "No nombre de archivo rom especificado",
  "Lista de nombre de archivos demasiada larga",
  "Lista de nombres de archivos ilegal",
  "No nombre de archivo de salida especificado",
  "Nombre de archivo de salida demasiado largo",
  "Nombre de archivo de salida ilegal",
  "Multiples archivos de salida especificados",
  "Nombre de entrada demasiado largo ",
  "Nombre de entrada ilegal",
  "Multitud de archivos de entrada especificados",
  "Errores           : %i",
  "Advertencias      : %i",
  "Codigo            : %i",
  "Rom               : %i",
  "Informacion(data) : %i",
  "Codigo sin usar   : %i",
  "Rom sin usar      : %i",
  "No codigo generado",
  "| Dispositivo      | Ram start | Flash      | Ram      | EEPROM      |",
  "No include path found after '-I' flag",
  "Too many include paths specified",
  "Include pathname too long"

};

#endif

#endif/* _MESSAGES_HH_ */

/// FINAL DE ARCHIVO /////////////////////////////////////////////////////////
