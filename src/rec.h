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
#ifndef _REC_H_
#define _REC_H_

#include "ff.h"

typedef enum {
  REC_LINE,
  REC_MIC
} RecType_t;

extern void record_wav_file(uint8_t source, uint16_t Fs, uint8_t stereo, const uint8_t *fname);
extern void rec_init(void);
extern void rec_stop(void);
extern void rec_begin(RecType_t type);
extern void rec_to_SPI(uint16_t Fs, uint8_t stereo, uint8_t source);
extern uint8_t *rec_SPI_get_buffer(void);
extern uint8_t rec_SPI_get_full_buffers(void);
extern void rec_flush_buffer(void);
extern void rec_dma_isr(void);

#endif // _REC_H_
// vim: expandtab ts=2 sw=2 ai cindent
