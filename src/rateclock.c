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
 * This configures TCC0 to generate Event Channel 0 at the user-selected sampling rate
 */

#include <stddef.h>
#include <avr/io.h>
#include "config.h"
#include "rateclock.h"

void rateclock_start(uint16_t Fs)
{
  TCC0.CTRLB = TC_WGMODE_NORMAL_gc;
  TCC0.PER = (32000000UL / Fs)-1; // 32 MHz / 16 kHz --> 2000
  TCC0.INTFLAGS = TC0_OVFIF_bm; // Clear timer overflow flag
  TCC0.CTRLA = TC_CLKSEL_DIV1_gc; // Divide by 1, so 32 MHz clock (31.25ns period)

  // Configure event system to route TCC0 overflow to Event Channel 0
  EVSYS.CH0MUX = EVSYS_CHMUX_TCC0_OVF_gc;
}

void rateclock_stop(void)
{
  TCC0.CTRLA = 0;
}
// vim: expandtab ts=2 sw=2 ai cindent
