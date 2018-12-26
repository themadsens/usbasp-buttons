# Name: Makefile
# Project: Remote Sensor
# Author: Christian Starkjohann
# Creation Date: 2005-03-20
# Tabsize: 4
# Copyright: (c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt) or proprietary (CommercialLicense.txt)
# This Revision: $Id$

SERIAL = `echo /dev/tty.KeySerial*`
UISP = uisp -dprog=avr910 -dserial=$(SERIAL) -dpart=auto
AVRDUDE = avrdude -c usbtiny -p attiny85
PROJECT=usbasp-buttons
# The two lines above are for "uisp" and the AVR910 serial programmer connected
# to a Keyspan USB to serial converter to a Mac running Mac OS X.
# Choose your favorite programmer and interface.

COMPILE = avr-gcc -Wall -Os -Iusbdrv -I. -I .. -mmcu=attiny85 -DF_CPU=16500000L #-DDEBUG_LEVEL=2

OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o usbled.o


# symbolic targets:
all:	$(PROJECT).hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

flash:	all
	micronucleus $(PROJECT).hex
#	$(AVRDUDE) -U flash:w:$(PROJECT).hex:i
#	$(UISP) --erase --upload --verify if=$(PROJECT).hex

clean:
	rm -f $(PROJECT).* *.o usbdrv/*.o usbdrv/oddebug.s usbdrv/usbdrv.s

# file targets:
$(PROJECT).elf:	$(OBJECTS)
	$(COMPILE) -o $(PROJECT).elf $(OBJECTS)

$(PROJECT).hex:	$(PROJECT).elf
	rm -f $(PROJECT).hex $(PROJECT).eep.hex
	avr-objcopy -j .text -j .data -O ihex $(PROJECT).elf $(PROJECT).hex
	../checksize $(PROJECT).elf 6500 500
# do the checksize script as our last action to allow successful compilation
# on Windows with WinAVR where the Unix commands will fail.

size: $(PROJECT).elf
	../checksize $(PROJECT).elf 6500 500

disasm:	$(PROJECT).elf
	avr-objdump -d $(PROJECT).elf > $(PROJECT).asm

.PHONY: all flash clean disasm size patch-loader
