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
#ifndef _WAVWRITE_H_
#define _WAVWRITE_H_

#include <inttypes.h>

extern uint8_t wav_create(const char *fname, uint8_t stereo, uint16_t Fs);
extern uint8_t presize_wav_file(const char *fname, uint16_t megabytes);
extern uint8_t wav_rec_finalize(void);

#endif // _WAVWRITE_H_
// vim: ts=2 sw=2 ai expandtab cindent
