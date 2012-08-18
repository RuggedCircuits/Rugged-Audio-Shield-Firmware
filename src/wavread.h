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
#ifndef _WAVREAD_H_
#define _WAVREAD_H_

#include <inttypes.h>
#include "ff.h"

typedef struct {
  // These have to come in this order so we can copy directly to/from WAV file
  uint16_t mChannels;       // 1-mono, 2-stereo
  uint32_t mSamplingRate;   // Samples per second
  uint32_t mBytesPerSecond; // Not really used
  uint16_t mBlockAlignment;
  uint16_t mBitsPerSample;  // 8 or 16
} WAVInfo_t;

extern WAVInfo_t gWAVInfo;
extern FIL gFile;
extern uint8_t wav_open(const char *fname);
extern uint8_t wav_fill_buffer(uint16_t *buf, UINT *bytesRead);

#endif // _WAVREAD_H_
// vim: ts=2 sw=2 ai expandtab cindent
