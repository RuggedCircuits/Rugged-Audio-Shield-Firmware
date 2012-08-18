# 
# Rugged Audio Shield Firmware for ATxmega
#
# Copyright (c) 2012 Rugged Circuits LLC.  All rights reserved.
# http://ruggedcircuits.com
#
# This file is part of the Rugged Circuits Rugged Audio Shield firmware distribution.
#
# This is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 3 of the License, or (at your option) any later
# version.
#
# This software is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# A copy of the GNU General Public License can be viewed at
# <http://www.gnu.org/licenses>

        ###################################################################
        #                                                                 #
        # This file sets configuration options for compiling the firmware #
        #                                                                 #
        ###################################################################

# The name of the application. The Makefile will create ${APPNAME}.elf
# and ${APPNAME}.hex
APPNAME=ad210

# Target AVR architecture (parameter to -mmcu= compiler flag)
ARCH=atxmega32a4

# Include and library directories, if necessary.
INCDIR=
LIBDIR=

# Flags for the assembler. The -a flags are listing flags, and you can modify
# these to suit (or omit altogether).
#
AFLAGS=-Wa,-mmcu=$(ARCH) -Wa,-acdhlsm

# Flags for the C compiler.
#
#   -mmcu=XXXX             -- processor name (e.g., atmega64) or
#                             AVR architecture (e.g., avr5)
#   -mshort-calls          -- Use rjmp/rcall instead of jmp/call if possible
#                             (generates lots of errors?)
#   -fpack-struct          -- pack structures together without holes
#   -g                     -- debugging support
#   -Os                    -- optimize for minimum code size
#   -O2                    -- optimize for maximum speed
#   -Wall                  -- enable all warnings
#   -fomit-frame-pointers  -- smaller code, harder to debug
#   -fno-inline            -- No inline functions, possibly smaller code
#   -funsigned-char        -- Characters are unsigned.
#
CFLAGS=-std=gnu99 ${INCDIR} -mmcu=$(ARCH) -fpack-struct -Os -Wall -fomit-frame-pointer -funsigned-char -ffunction-sections -fshort-enums  #-g

# Linker flags.
#
#    -g                    -- debugging support
#    -Map icontrol.map     -- Generate program map (very useful to study)
#    --cref                -- Output cross reference to map file
#    -Wl,-u,vfprintf       -- Make vfprintf a label to be resolved (for floating point support)
#    -lprintf_flt          -- Link with floating-point library
#    -lm                   -- Link with math library
#
LFLAGS=-Tavrxmega2.x ${LIBDIR} -mmcu=$(ARCH) -Wl,-Map,${APPNAME}.map -Wl,--cref -Wl,--gc-sections #-g

# The AVR C Compiler and assembler, and related tools. You can specify
# full pathnames, but it's easier if you just put these on your path.
#TOOLPREFIX=wine ....
#TOOLSUFFIX=.exe
CC=${TOOLPREFIX}avr-gcc${TOOLSUFFIX}
AS=${TOOLPREFIX}avr-as${TOOLSUFFIX}
GDB=${TOOLPREFIX}avr-gdb${TOOLSUFFIX}
SIZE=${TOOLPREFIX}avr-size${TOOLSUFFIX}
OBJCOPY=${TOOLPREFIX}avr-objcopy${TOOLSUFFIX}

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#
# End of user-customizable area. Do not modify things below this point.
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

SRCS= main.c sio.c utils.c timer.c clocks.c spi_C_slave.c rec.c adc.c \
	buffers.c dac.c play.c state.c i2c.c wavwrite.c wavread.c fail.c dma.c \
	rateclock.c printf.c bootloader.c pass.c ff.c
OBJS=$(SRCS:.c=.o)

