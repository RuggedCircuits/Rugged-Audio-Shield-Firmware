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
 * This module handles writing WAV files to the SD card
 */
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "buffers.h"
#include "wavread.h"
#include "wavwrite.h"
#include "ff.h"
#include "fail.h"

// Create a WAV file, fill in basic info, then skip over the header
// to get to the data. We have to seek back here to fill in the
// data size when all is said and done.
// Returns 0 if failure, 1 if successful.
// NOTE: It uses one of the global ping-pong buffers for temporary storage
uint8_t wav_create(const char *fname, uint8_t stereo, uint16_t Fs)
{
  FRESULT fresult;

  (void) fail_major(FAIL_WAV_CREATE);

#if 0
  {
    UINT bytesWritten;
    uint8_t buf[2];
    *(uint16_t *)buf = (uint16_t)fname;
    f_open(&gFile, "junk.txt", FA_WRITE | FA_CREATE_ALWAYS);
    f_write(&gFile, buf, 2, &bytesWritten);
    f_write(&gFile, fname-4, MAX_SPI_BUF_SIZE, &bytesWritten);
    f_close(&gFile);

    //return fail_minor(FAIL_WAV_NO_FILE);
  }
#endif

  fresult = f_open(&gFile, fname, FA_WRITE | FA_CREATE_ALWAYS);
  if (fresult != FR_OK) return fail_minor(FAIL_WAV_NO_FILE);

  // Skip over the first 44 bytes, all the header/chunk stuff
  fresult = f_lseek(&gFile, 44);
  if (fresult != FR_OK) return fail_minor(FAIL_WAV_SEEK);

  // All ready to start writing data. When done, we'll have to go back and fill in the header.
  // For now, fill in the WAVINFO header so we know how to finalize.
  stereo = stereo ? 2 : 1;
  gWAVInfo.mChannels       = stereo;
  gWAVInfo.mSamplingRate   = Fs;
  gWAVInfo.mBytesPerSecond = Fs*stereo*2;
  gWAVInfo.mBlockAlignment = stereo*2;
  gWAVInfo.mBitsPerSample  = 16;

  return fail_nofail();
}

uint8_t wav_rec_finalize(void)
{
  FRESULT fresult;
  UINT bytesWritten;
  DWORD subChunk2Size;
  uint8_t *buf = (uint8_t *)gBuffers;

  (void) fail_major(FAIL_WAV_FINALIZE);

  // Raw number of bytes written after all header info
  subChunk2Size = f_tell(&gFile) - 44;

  // Truncate file here in case it was presized
  fresult = f_truncate(&gFile);
  if (fresult != FR_OK) return fail_minor(FAIL_WAV_TRUNCATE);

  fresult = f_lseek(&gFile, 0);
  if (fresult != FR_OK) return fail_minor(FAIL_WAV_SEEK);

  memcpy_P(buf, PSTR("RIFF    WAVEfmt \x10\x00\x00\x00\x01\x00"), 22);
  *(uint32_t *)(buf+4) = subChunk2Size + 36;

  fresult = f_write(&gFile, buf, 22, &bytesWritten);
  if ((fresult != FR_OK) || (bytesWritten != 22)) return fail_minor(FAIL_WAV_NO_HEADER);

  // Now write out WAVINFO header
  fresult = f_write(&gFile, &gWAVInfo, 14, &bytesWritten);
  if ((fresult != FR_OK) || (bytesWritten != 14)) return fail_minor(FAIL_WAV_NO_HEADER);

  // Finally write out last 8 bytes of header, "data" plus subchunk2 size
  memcpy_P(buf, PSTR("data    "), 8);
  *(uint32_t *)(buf+4) = subChunk2Size;

  fresult = f_write(&gFile, buf, 8, &bytesWritten);
  if ((fresult != FR_OK) || (bytesWritten != 8)) return fail_minor(FAIL_WAV_NO_HEADER);

  return fail_nofail();
}

uint8_t presize_wav_file(const char *fname, uint16_t megabytes)
{
  FRESULT fresult;

  (void) fail_major(FAIL_WAV_PRESIZE);

  fresult = f_open(&gFile, fname, FA_WRITE | FA_CREATE_ALWAYS);
  if (fresult != FR_OK) return fail_minor(FAIL_WAV_NO_FILE);

  fresult = f_lseek(&gFile, (DWORD)megabytes * 1048576UL/*1024UL * 1024UL*/);
  if (fresult != FR_OK) return fail_minor(FAIL_WAV_SEEK);

  (void) f_close(&gFile);

  return fail_nofail();
}
// vim: ts=2 sw=2 ai expandtab cindent
