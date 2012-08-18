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
#ifndef _CLOCKS_H_
#define _CLOCKS_H_

typedef enum {
  CLKSRC_2MHZ=0,        // 2 MHz internal oscillator
  CLKSRC_16MHZ_EXT=1,   // 16 MHz external oscillator
  CLKSRC_32MHZ_EXT=2,   // 16 MHz external oscillator, PLL up to 32 MHz
  CLKSRC_32MHZ_INT=3    // 32 MHz internal oscillator
} ClockSource_t;

void setClockSource(ClockSource_t clk);
ClockSource_t getClockSource(void);

#endif // _CLOCKS_H_
// vim: expandtab ts=2 sw=2 ai cindent
