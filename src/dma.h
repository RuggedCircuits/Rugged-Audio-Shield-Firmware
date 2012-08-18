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
#ifndef _DMA_H_
#define _DMA_H_

#include <inttypes.h>

typedef enum {
  DMA_CFG_RECORD,      // Set up DMA for recording from ADC
  DMA_CFG_PLAY,        // Set up DMA for playing to DAC
} DMAConfig_t;

extern void dma_begin(DMAConfig_t config, uint8_t stereo);
extern void dma_off(void);
extern void dma_wait_for_disable(void);
extern void dma_reset(void);

#endif // _DMA_H_
// vim: expandtab ts=2 sw=2 ai cindent
