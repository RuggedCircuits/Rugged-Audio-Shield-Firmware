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
#include "buffers.h"

// This is used for playback as well
uint16_t volatile gBuffers[2][BUFFER_SIZE/2];
uint8_t  volatile gActiveDMABuffer;  // Which buffer is being currently accessed by DMA
uint8_t  volatile gDMABufferDone;   // Set when buffer is full, done, ping-pong'ed etc.
uint8_t gCtrlFlags;

void buffers_init(void)
{
  gDMABufferDone = 0;
}

// Set an unsigned value of 0x8000 in a buffer, as that is essentially "0V" for the DAC outputs
void buffers_clear(uint8_t bufix)
{
  const uint16_t *bufend;
  uint16_t *buf;
  
  if (bufix) {
    buf = (uint16_t *) (gBuffers[1]);
    bufend = gBuffersEnd;
  } else {
    buf = (uint16_t *) (gBuffers[0]);
    bufend = (const uint16_t *) (gBuffers[1]);
  }

  while (buf != bufend) {
    *buf++ = 0x8000U;
  }
}

void buffers_clear_all(void)
{
  buffers_clear(0);
  buffers_clear(1);
}
// vim: expandtab ts=2 sw=2 ai cindent
