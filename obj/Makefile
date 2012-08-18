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

include ../config.mk

.SUFFIXES:
.SUFFIXES: .c .s .o

vpath
vpath %.s ../src
vpath %.c ../src
vpath %.h ../src

# This rule cancels the implicit rule to make .o directly from .c
%.o: %.c

%.o: %.s

%.o: %.c
	$(CC) -c $(CFLAGS) $(DEFINES) -o $*.o $<

#%.s: %.c
#	$(CC) -S -o $*.s $(CFLAGS) $(DEFINES) $<

%.o: %.s
	$(CC) -c $(AFLAGS) -Wa,-a=$*.lst -o $*.o $<

ifeq ($(wildcard depend.mk),depend.mk)
include depend.mk
endif

dep depend.mk: ../src/*.c ../src/*.h
	$(CC) $(CFLAGS) $(DEFINES) -MM ../src/*.c > depend.mk

all: ${APPNAME}.elf

clean:
	rm -f *.o *.hex *.elf *.map $(AOBJS) *.lst depend.mk

${APPNAME}.elf: $(OBJS)
	$(CC) $(LFLAGS) -o ${APPNAME}.elf $(OBJS) $(LIBS)
	$(SIZE) -Bd ${APPNAME}.elf
	cp ${APPNAME}.elf ..

# vim: noexpandtab
