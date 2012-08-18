/*
  Rugged Audio Shield Firmware for ATxmega

  Copyright (c) 2012 Rugged Circuits LLC.  All rights reserved.
  http://ruggedcircuits.com

  This file is part of the Rugged Circuits Rugged Audio Shield firmware distribution.

  This is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation; either version 3 of the License, or (at your option) any later
  version.

  This software is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  A copy of the GNU General Public License can be viewed at
  <http://www.gnu.org/licenses>
*/
/*
 * Utility functions
 *
 */
#include <stdlib.h>
#include "sio.h"
#include "utils.h"

#ifdef WITH_SIO
static void disp_u16(uint16_t value, uint8_t base)
{
	char buf[16];
	utoa(value, buf, base);
	outstr_E0(buf);
}

static void disp_u32(uint32_t value, uint8_t base)
{
	char buf[32];
	ultoa(value, buf, base);
	outstr_E0(buf);
}

void disp10(uint16_t value)
{
	disp_u16(value, 10);
}

void disp10l(uint32_t value)
{
	disp_u32(value, 10);
}

void disp16(uint16_t value)
{
	disp_u16(value, 16);
}

#endif // WITH_SIO
