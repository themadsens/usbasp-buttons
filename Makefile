###############################################################################
# Makefile for the project $(PROJECT)
###############################################################################

## General Flags
PROJECT = usbasp-buttons

MCU = atmega8
#MCU = atmega16
#MCU = atmega48
#MCU = atmega88
#MCU = atmega168
#MCU = atmega328p

CLK = 12000000UL
#CLK = 15000000UL
#CLK = 16000000UL
#CLK = 18000000UL
#CLK = 20000000UL

PROGRAMMER = usbtiny

OUT := $(shell mkdir -p out)

TARGET = out/$(PROJECT).elf
CC = avr-gcc

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU) -DF_CPU=$(CLK)

## UART_INVERT enables software-inverter (PC0 -|>o- PB0, PC1 -|>o- PB1)
## to connect to RS-232C line directly. ( <= 2400 bps )
## atmega8 doesn't support this
#COMMON += -DUART_INVERT

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -Os -fsigned-char
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d -DUSBASP

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -Wl,-Map,out/$(PROJECT).map 


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Include Directories
INCLUDES = -I"." -I"v-usb/usbdrv"

## Objects that must be built in order to link
OBJECTS = usbdrv.o usbdrvasm.o oddebug.o $(patsubst %.c,%.o, $(wildcard *.c))
OUTOBJS = $(patsubst %, out/%, $(OBJECTS))

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(OUT) $(TARGET) out/$(PROJECT).hex out/$(PROJECT).eep out/$(PROJECT).lss size

flash: all
	avrdude -c $(PROGRAMMER) -p $(MCU) -U flash:w:out/$(PROJECT).hex

## Compile
out/usbdrvasm.o: v-usb/usbdrv/usbdrvasm.S
	$(CC) $(INCLUDES) $(ASMFLAGS) -o $@ -c  $<

out/usbdrv.o: v-usb/usbdrv/usbdrv.c
	$(CC) $(INCLUDES) $(CFLAGS) -o $@ -c  $<

out/oddebug.o: v-usb/usbdrv/oddebug.c
	$(CC) $(INCLUDES) $(CFLAGS) -o $@ -c  $<

out/%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) -o $@ -c  $<

##Link
$(TARGET): $(OUTOBJS)
	 $(CC) $(LDFLAGS) $(OUTOBJS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	rm -rf out

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

.PHONY: all clean flash size
