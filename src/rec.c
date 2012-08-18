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
 * This module handles recording to SD card or recording to SPI stream
 */
#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>

#include "config.h"
#include "ff.h"
#include "adc.h"
#include "buffers.h"
#include "state.h"
#include "wavread.h"
#include "wavwrite.h"
#include "dma.h"
#include "rateclock.h"
#include "fail.h"
#include "rec.h"

static uint8_t volatile gSPIOutputBuffersFull;
static uint8_t gSPITailBuffer;   // Which buffer is currently being emptied by outgoing SPI data
static uint8_t gSPITailBufferIx; // Where in the buffer the next outgoing SPI data packet will be retrieved

static void _rec_common(uint16_t Fs, uint8_t stereo, uint8_t source)
{
  dma_begin(DMA_CFG_RECORD, stereo);
  gActiveDMABuffer = 0;
  gDMABufferDone = 0;

  // Enable Channel 0. Let double-buffering action enable buffer 1 after first block of channel 0 is done.
  // DMA trigger will be A/D conversion complete.
  DMA.CH0.CTRLA |= DMA_ENABLE_bm;

  // ADC conversion complete will trigger DMA action
  adc_start(source ? REC_MIC : REC_LINE); 

  // TCC0 events will trigger A/D sampling
  rateclock_start(Fs);
}

// Source is 0 for line in, 1 for mic.
void record_wav_file(uint8_t source, uint16_t Fs, uint8_t stereo, const uint8_t *fname)
{
  if (! wav_create((const char *)fname, stereo, Fs)) return;

  gState = STATE_RECORDING_TO_SD;

  _rec_common(Fs, stereo, source);
}

void rec_to_SPI(uint16_t Fs, uint8_t stereo, uint8_t source)
{
  gState = STATE_RECORDING_TO_SPI;

  gSPIOutputBuffersFull=0;    // No buffers filled yet
  gSPITailBuffer=0;           // First outgoing SPI packet will come from gBuffers[0]
  gSPITailBufferIx=0;         // First outgoing SPI packet will come from gBuffers[0][0]

  _rec_common(Fs, stereo, source);
}

uint8_t *rec_SPI_get_buffer(void)
{
  uint8_t *buf;

  buf = (uint8_t *)(gBuffers[gSPITailBuffer]) + gSPITailBufferIx;

  ATOMIC_BLOCK(ATOMIC_FORCEON) {
    gSPIOutputBuffersFull--;
  }

  gSPITailBufferIx += SPI_STREAM_SIZE_BYTES;
  if (gSPITailBufferIx >= BUFFER_SIZE) {
    gSPITailBuffer = 1 - gSPITailBuffer;
    gSPITailBufferIx = 0;
  }

  return buf;
}

uint8_t rec_SPI_get_full_buffers(void)
{
  return gSPIOutputBuffersFull;
}

// Called when a DMA buffer has been fully recorded
void rec_dma_isr(void)
{
  gSPIOutputBuffersFull += (BUFFER_SIZE/SPI_STREAM_SIZE_BYTES);
}

void rec_stop(void)
{
  dma_off();
  dma_wait_for_disable();
  dma_reset();

  adc_stop();
  rateclock_stop();

  if (gState == STATE_RECORDING_TO_SD) {
    wav_rec_finalize();
    f_close(&gFile);
  }

  gState = STATE_IDLE;
}

void rec_flush_buffer(void)
{
  if (gDMABufferDone) {
    UINT bytesWritten;
    FRESULT fresult;

    // Data coming from the ADC's is essentially exactly what we want. Write it out.
    gDMABufferDone = 0;
    fresult = f_write(&gFile, (const void *)(gBuffers[1-gActiveDMABuffer]), BUFFER_SIZE, &bytesWritten);
    if ((fresult != FR_OK) || (bytesWritten != BUFFER_SIZE)) {
      fail(FAIL_REC, FAIL_REC_BUFWRITE);
      rec_stop();
      return;
    }
  }
}
// vim: ts=2 sw=2 ai expandtab cindent
