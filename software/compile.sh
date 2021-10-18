#!/bin/bash

avr-gcc -Wall -Os -mmcu=atmega328p "$1" -o "$1.o"


if [ $? -eq 0 ]
then
    avr-objcopy -j .text -j .data -O ihex "$1.o" "$1.hex"
    avrdude -p m328p -c usbasp -U "flash:w:$1.hex:i"
fi
