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
#ifndef _BUFFERS_H_
#define _BUFFERS_H_

#include <inttypes.h>
#include "config.h"

// This is used for playback as well
extern volatile uint16_t gBuffers[2][BUFFER_SIZE/2];
static uint16_t * const gBuffersEnd = (uint16_t *)(gBuffers[0] + BUFFER_SIZE);
extern uint8_t  volatile gActiveDMABuffer;  // Which buffer is being transferred by DMA
extern uint8_t  volatile gDMABufferDone;   // Set when buffer is full, done, ping-pong'ed etc.

extern void buffers_init(void);
extern void buffers_clear(uint8_t buffer);
extern void buffers_clear_all(void);

/*
   Bit 0: Set when gBuffers[0] is the last buffer that should play
   Bit 1: Set when gBuffers[1] is the last buffer that should play
   Bit 2: Set when the playback process needs to be kickstarted
*/
#define CTRL_FLAG_BUFFER0_IS_LAST 0x1  // Must be (1<<0) to represent 1<<buffer0
#define CTRL_FLAG_BUFFER1_IS_LAST 0x2  // Must be (1<<1) to represent 1<<buffer1
#define CTRL_FLAG_KICKSTART       0x4

extern uint8_t gCtrlFlags;

#endif // _BUFFERS_H_
// vim: expandtab ts=2 sw=2 ai cindent
