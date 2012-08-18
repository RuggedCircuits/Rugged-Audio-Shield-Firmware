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
/*
 * This module handles reading WAV files from SD card
 */
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "buffers.h"
#include "wavread.h"
#include "ff.h"
#include "fail.h"

// WAV info structure used for playing, recording, ...
WAVInfo_t gWAVInfo;

// File structure used for reading/writing
FIL gFile;

// Number of bytes left to read in the current data chunk
static uint32_t gChunkBytesRemaining;

// Open a WAV file, parse the chunks, and prepare to start reading audio data
// Returns 0 if failure, 1 if successful.
// NOTE: It uses one of the global ping-pong buffers for temporary storage
uint8_t wav_open(const char *fname)
{
  FRESULT fresult;
  UINT bytesRead;
  uint32_t lChunkSize;
  uint8_t *buf = (uint8_t *)gBuffers;

  (void) fail_major(FAIL_WAV_OPEN);

  fresult = f_open(&gFile, fname, FA_READ | FA_OPEN_EXISTING);
  if (fresult != FR_OK) return fail_minor(FAIL_WAV_NO_FILE);

  fresult = f_read(&gFile, buf, 36, &bytesRead);
  if ((fresult != FR_OK) || (bytesRead<36)) return fail_minor(FAIL_WAV_NO_HEADER);

  // First 4 bytes: RIFF
  if (memcmp_P(buf, PSTR("RIFF"), 4)) return fail_minor(FAIL_WAV_NO_RIFF);

  // Bytes 4-7: size of remaining data, hence file size is this plus previous 8 bytes
  //gWAVInfo.mFileSize = *(uint32_t *)(buf+4) + 8;

  // Bytes 8-11: WAVE
  if (memcmp_P(buf+8, PSTR("WAVE"), 4)) return fail_minor(FAIL_WAV_NO_WAVE);

  // Bytes 12-15: "fmt "
  if (memcmp_P(buf+12, PSTR("fmt "), 4)) return fail_minor(FAIL_WAV_NO_FMT);

  // Bytes 16-19: fmt chunk size (should be 16 for PCM)
  lChunkSize = *(uint32_t *)(buf+16);
  if (lChunkSize < 16) return fail_minor(FAIL_WAV_BAD_FMT);

  // Bytes 20-21: audio format, should be 1 for PCM, something else for compression
  if (*(uint16_t *)(buf+20) != 1) return fail_minor(FAIL_WAV_NOT_PCM);

  // Bytes 22-36: number of channels, sample rate, byte rate, block alignment, bits per sample
  memcpy(& (gWAVInfo.mChannels), buf+22, 14);

  // If chunk size is not 16, skip to the end of the chunk
  //if (lChunkSize > 16) {
    f_lseek(&gFile, f_tell(&gFile) + (lChunkSize-16));
  //}

  // Now read data chunk
  fresult = f_read(&gFile, buf, 8, &bytesRead);
  if ((fresult != FR_OK) || (bytesRead<8)) return fail_minor(FAIL_WAV_NO_DATA);

  if (memcmp_P(buf, PSTR("data"), 4)) return fail_minor(FAIL_WAV_BAD_DATA);

  gChunkBytesRemaining = *(uint32_t *)(buf+4);

  return fail_nofail();
}

uint8_t wav_fill_buffer(uint16_t *buf, UINT *bytesRead)
{
  UINT bytesActuallyRead;
  FRESULT fresult;

  *bytesRead = BUFFER_SIZE;
  if (BUFFER_SIZE > gChunkBytesRemaining) {
    *bytesRead = (UINT) gChunkBytesRemaining;
  }
  gChunkBytesRemaining -= *bytesRead;
  if (*bytesRead == 0) return 1;

  fresult = f_read(&gFile, buf, *bytesRead, &bytesActuallyRead);
  if ((fresult != FR_OK) || (bytesActuallyRead < *bytesRead)) return fail(FAIL_WAV_READ, 0);

  return 1;
}

// vim: ts=2 sw=2 ai expandtab cindent
