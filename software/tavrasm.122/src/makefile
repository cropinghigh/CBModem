#############################################################################
#
#  Title	: TAVRASM Makefile
#
#  Author	: Tom Mortensen - (C) 1998
#
#  Create	: 980902
#
#  Notice       : Select compiler as appropriate (egcs or gcc)
#
#  rm/mv        : Versions of rm (remove/delete) and mv (move) that supports
#                 "Unix" style parameters must be available.
#
#############################################################################

### Program name ############################################################

EXENAME		= tavrasm

### Language : US, DE #######################################################

LANGUAGE	= US
VERSION		= 122

### Compiler options #########################################################

CC		= gcc
CFLAGS          = -O2 -Wall -DAVRLANG=LANGUAGE_$(LANGUAGE)
CPPFLAGS	=
CCFLAGS         = $(CFLAGS) $(CPPFLAGS)
LINK            = -lm -lstdc++
OBJS		= avrparse.o avrlex.o avrasm.o symbol.o semantic.o utils.o

### Defaul CC files #########################################################

.cc.o:
		$(CC) -c $(CCFLAGS) $<

### Executable ##############################################################

$(EXENAME):	$(OBJS)
		$(CC) $(CCFLAGS) $(OBJS) $(LINK) -o $@

### Parser file #############################################################

avrparse.cc:	avrparse.y avrasm.hh symbol.hh semantic.hh opcodes.hh
		bison -d -o avrparse avrparse.y
		mv avrparse avrparse.cc
		mv avrparse.h avrparse.hh

### Scanner file ############################################################

avrlex.cc:	avrparse.cc avrlex.l symbol.hh semantic.hh avrparse.hh \
		opcodes.hh avrasm.hh messages.hh
		flex -o$@ avrlex.l

### Dependencies ############################################################

avrlex.o:	avrlex.l
avrparse.o:	avrparse.y
avrasm.o:	avrasm.cc avrasm.hh symbol.hh devices.hh messages.hh
symbol.o:	symbol.cc symbol.hh avrasm.hh
utils.o:	utils.cc utils.hh avrasm.hh messages.hh msg_us.hh msg_de.hh msg_sp.hh
semantic.o:	semantic.cc semantic.hh symbol.hh opcodes.hh avrasm.hh \
		messages.hh

### Clean ###################################################################

clean:
		rm -rf *.o
		rm -rf *.O
		rm -rf avrparse.cc
		rm -rf avrparse.hh
		rm -rf avrlex.cc 
		rm -rf core *~
		rm -rf *.output
		rm -rf $(EXENAME)
		rm -rf $(EXENAME).EXE	
		rm -rf *.obj
		rm -rf *.out

### Distribution ############################################################

dist:		
		rm -rf  tavrasm$(VERSION)
		mkdir tavrasm$(VERSION)
		mkdir tavrasm$(VERSION)/src
		cp *.cc tavrasm$(VERSION)/src/.
		cp *.hh tavrasm$(VERSION)/src/.
		cp *.l tavrasm$(VERSION)/src/.
		cp *.y tavrasm$(VERSION)/src/.
		cp makefile tavrasm$(VERSION)/src/.
		cp gpl.txt tavrasm$(VERSION)/.
		cp README tavrasm$(VERSION)/.

dist2:		clean dist $(EXENAME)
		cp  $(EXENAME) tavrasm$(VERSION)/.
		tar cvfz tavrasm.tar.gz tavrasm$(VERSION)

distribution:	dist2
		rm -rf *.o
		rm -rf avrparse.cc
		rm -rf avrparse.hh
		rm -rf avrlex.cc 
		rm -rf core *~
		rm -rf *.output
		rm -rf $(EXENAME)
		rm -rf $(EXENAME).EXE	
		rm -rf *.obj
		rm -rf *.out
		rm -rf  tavrasm$(VERSION)

### END OF FILE #############################################################




