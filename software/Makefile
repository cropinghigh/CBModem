.PHONY : all
all: modem

modem: modem.c
	avr-gcc --param=min-pagesize=0 -Wall -O2 -mmcu=atmega328p "modem.c" -o "modem.o"
flash:
	avr-objcopy -j .text -j .data -O ihex "modem.o" "modem.hex"
	avrdude -p m328p -c usbasp -U "flash:w:modem.hex:i"
clean:
	rm -f *.o
