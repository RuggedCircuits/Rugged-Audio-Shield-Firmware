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

# Edit 'config.mk' to set compiling options
include config.mk

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#
# End of user-customizable area. Do not modify things below this point.
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

all: ${APPNAME}.hex

${APPNAME}.hex: ${APPNAME}_elf
	#
	# Converting from ELF to HEX format
	#
	$(OBJCOPY) --strip-unneeded --remove-section=.comment --remove-section=.eeprom --output-format=binary ${APPNAME}.elf PROGRAM.BIN
	@echo "-------------------------------------------------------------"
	@echo "Compilation complete. Output is PROGRAM.BIN"
	@echo "-------------------------------------------------------------"

.PHONY: clean ${APPNAME}_elf
${APPNAME}_elf:
	./buildplusplus.py src/version.c
	$(MAKE) -C obj dep all

clean:
	-$(MAKE) -C obj clean
	-rm -f *.hex *.elf *.bin *.BIN

realclean: clean

# vim: noexpandtab
