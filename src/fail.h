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
#ifndef _FAIL_H_
#define _FAIL_H_

#include <inttypes.h>

typedef enum {
  FAIL_NO_FAIL=0,

  FAIL_WAV_OPEN,
  FAIL_WAV_READ,
  FAIL_WAV_CREATE,
  FAIL_WAV_FINALIZE,
  FAIL_REC,
  FAIL_MOUNT,
  FAIL_MKFS,
  FAIL_WAV_PRESIZE,
} FailMajor_t;

typedef enum {
  FAIL_NO_FAIL_NO_FAIL=0,

  FAIL_WAV_NO_FILE,
  FAIL_WAV_NO_HEADER,
  FAIL_WAV_NO_RIFF,
  FAIL_WAV_NO_WAVE,
  FAIL_WAV_NO_FMT,
  FAIL_WAV_BAD_FMT,
  FAIL_WAV_NOT_PCM,
  FAIL_WAV_NO_DATA,
  FAIL_WAV_BAD_DATA,
  FAIL_WAV_SEEK,
  FAIL_REC_BUFWRITE,
  FAIL_MOUNT_NO_INIT,
  FAIL_MOUNT_NO_MOUNT,
  FAIL_MOUNT_SUCCESS,
  FAIL_MKFS_FAILED,
  FAIL_MKFS_BAD_CODE,
  FAIL_WAV_TRUNCATE,
} FailMinor_t;

extern uint8_t gFailMajor, gFailMinor;
extern uint8_t fail(FailMajor_t major, FailMinor_t minor);
extern uint8_t fail_major(FailMajor_t major);
extern uint8_t fail_minor(FailMinor_t minor);
static inline uint8_t fail_nofail(void) { return fail(0, 0), 1; }

#endif // _FAIL_H_
// vim: expandtab ts=2 sw=2 ai cindent
