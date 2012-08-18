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
#ifndef _PLAY_H_
#define _PLAY_H_

#include "ff.h"

extern void    play_wav_file(const uint8_t *fname);
extern void    play_from_SPI(uint16_t Fs, uint8_t stereo);
extern uint8_t play_SPI_get_free_buffers(void);
extern void    play_SPI_add_buffer(const uint8_t *buf);
extern void    play_stop(void);
extern void    play_dma_ch0_isr(void);
extern void    play_dma_ch1_isr(void);
extern void    play_fill_buffer(void);

#endif // _PLAY_H_
// vim: expandtab ts=2 sw=2 ai cindent
