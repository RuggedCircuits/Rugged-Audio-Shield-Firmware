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
#ifndef _STATE_H_
#define _STATE_H_

// These integers are hard-coded into Arduino library so don't reorder them
typedef enum {
  STATE_IDLE=0,

  STATE_RECORDING_TO_SD=1,
  STATE_PLAYING_FROM_SD=2,
  STATE_PLAYING_FROM_SPI=3,
  STATE_RECORDING_TO_SPI=4,
  STATE_PASS_THROUGH=5,

  STATE_BUSY=6,

  STATE_NUM_STATES
} State_t;

extern State_t gState;

#endif // _STATE_H_
// vim: expandtab ts=2 sw=2 ai cindent
