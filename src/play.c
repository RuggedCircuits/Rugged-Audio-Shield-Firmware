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
 * This module handles playing from SD card
 */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>

#include "config.h"
#include "ff.h"
#include "adc.h"
#include "sio.h"
#include "utils.h"
#include "buffers.h"
#include "state.h"
#include "rateclock.h"
#include "play.h"
#include "dma.h"
#include "i2c.h"
#include "wavread.h"

static uint8_t volatile gSPIInputBuffersFree;
static uint8_t gSPIHeadBuffer;   // Which buffer is currently being filled from incoming SPI data
static uint16_t gSPIHeadBufferIx; // Where in the buffer the next incoming SPI data packet will be stored
static uint16_t gSPIFs;          // Sampling frequency to be used for SPI playback

/* WAV data is 16-bit signed left-adjusted, while DAC expects unsigned left-adjusted. Thus:

   - The input value 0x0000 maps to mid-scale--> 0x8000 for the DAC
   - Values from 0x0001-0x7FFF map to 0x8001-->0xFFFF
   - The input value 0x8000 maps to lowest-possible value--> 0x0000 for the DAC
   - Values from 0x8001-0xFFFF map to 0x0001-->0x7FFF

   So really, all we have to do is toggle the MSB of each 16-bit value. Since values are
   stored LSB-first we start at a byte offset of 1, and toggle the MSB of every other byte.

*/
static void _transform_buffer(uint16_t *samplebuf, register uint16_t samples)
{
  register uint8_t *buf = (uint8_t *)samplebuf + 1;

  for ( ; samples ; samples--, buf += 2) {
    *buf ^= (uint8_t)0x80U;
  }
}

void play_wav_file(const uint8_t *fname)
{
  if (! wav_open((const char *)fname)) return;

  // Don't enable yet. Do that in play_fill_buffer() below after we've filled the first 2 buffers
  gState = STATE_PLAYING_FROM_SD;

  gCtrlFlags = CTRL_FLAG_KICKSTART; // Tell play_fill_buffer() handler below to fill buffers then start DMA

  dma_begin(DMA_CFG_PLAY, (gWAVInfo.mChannels==2));
  gActiveDMABuffer = 1; // Trust me, it's right (look at how play_fill_buffer() works on kickstarting)
  gDMABufferDone = 1;  // Trust me, it's right (look at how play_fill_buffer() works on kickstarting)

#if 0 // r1: let user fully control OutputEnable to avoid clicks and pops
  I2C_shutdown_enable(0);
#endif
}

void play_from_SPI(uint16_t Fs, uint8_t stereo)
{
  gState = STATE_PLAYING_FROM_SPI;
  gCtrlFlags = CTRL_FLAG_KICKSTART; // Tell play_SPI_buffer() handler below to start DMA when data is received
  gSPIFs = Fs;

  dma_begin(DMA_CFG_PLAY, stereo);
  gActiveDMABuffer = 0;
  gDMABufferDone = 0;

  gSPIInputBuffersFree = (BUFFER_SIZE/SPI_STREAM_SIZE_BYTES)*2;
  gSPIHeadBuffer = 0;  // Start writing to gBuffers[0]
  gSPIHeadBufferIx = 0; // Start writing to gBuffers[0][0]...goes up by SPI_STREAM_SIZE_BYTES for each buffer added

#if 0 // r1: let user fully control OutputEnable to avoid clicks and pops
  I2C_shutdown_enable(0);
#endif
}

uint8_t play_SPI_get_free_buffers(void)
{
  return gSPIInputBuffersFree;
}

void play_SPI_add_buffer(const uint8_t *buf)
{
  uint8_t *dst;

  if (gSPIInputBuffersFree) {
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
      gSPIInputBuffersFree--;
    }
    dst = (uint8_t *)(gBuffers[gSPIHeadBuffer]) + gSPIHeadBufferIx;
#if 0
    memcpy(dst, buf, SPI_STREAM_SIZE_BYTES);
    _transform_buffer((uint16_t *)dst, SPI_STREAM_SIZE_BYTES/2);
#else
    // Same effect as memcpy+transform_buffer but all in one fell swoop. Is it faster?
    {
      uint8_t samples;

      for (samples=SPI_STREAM_SIZE_BYTES/2; samples; samples--) {
        *dst++ = *buf++;
        *dst++ = (*buf++) ^ (uint8_t)0x80U; // See documentation for _transform_buffer()
      }
    }
#endif

    gSPIHeadBufferIx += SPI_STREAM_SIZE_BYTES;
    if (gSPIHeadBufferIx >= BUFFER_SIZE) {
      gSPIHeadBufferIx = 0;
      gSPIHeadBuffer = 1 - gSPIHeadBuffer;
      
      // Start actual playback when the whole first buffer (1024 bytes) is filled
      if (gCtrlFlags & CTRL_FLAG_KICKSTART) {
        rateclock_start(gSPIFs); // DMA transfers will start shortly, triggered by Event Channel 0

        // Enable Channel 0. Let double-buffering action enable buffer 1 after first block of channel 0 is done.
        DMA.CH0.CTRLA |= DMA_ENABLE_bm;
        gCtrlFlags &= ~CTRL_FLAG_KICKSTART;
      }
    }
  } // else, we drop this buffer
}

static void _dma_off(void)
{
  dma_off();
#if 0 // r1: do not shut down the headphone amp to avoid pops
  I2C_shutdown_enable(1);
#endif
}

// Called from dma.c when gBuffers[0] has been played out to the DAC
// This is called from within an ISR
void play_dma_ch0_isr(void)
{
  switch (gState) {
    case STATE_PLAYING_FROM_SD:
      // Did we just play out the last buffer?
      if (gCtrlFlags & CTRL_FLAG_BUFFER0_IS_LAST) {
        _dma_off();
      } else if (gCtrlFlags & CTRL_FLAG_BUFFER1_IS_LAST) {
        // Fill this buffer with silence so that once the other one ping-pongs back to us, we play silence
        buffers_clear(0);
      }
      break;

    case STATE_PLAYING_FROM_SPI:
      // If we finished playing a buffer, one more SPI buffer is free
      gSPIInputBuffersFree += (BUFFER_SIZE/SPI_STREAM_SIZE_BYTES);
      if (gSPIInputBuffersFree > (BUFFER_SIZE/SPI_STREAM_SIZE_BYTES)*2) {
        gSPIInputBuffersFree = (BUFFER_SIZE/SPI_STREAM_SIZE_BYTES)*2;
      }
      break;

    default:
      break;
  }
}

// Called when gBuffers[1] has been played out to the DAC
// This is called from within an ISR
void play_dma_ch1_isr(void)
{
  switch (gState) {
    case STATE_PLAYING_FROM_SD:
      // Did we just play out the last buffer?
      if (gCtrlFlags & CTRL_FLAG_BUFFER1_IS_LAST) {
        _dma_off();
      } else if (gCtrlFlags & CTRL_FLAG_BUFFER0_IS_LAST) {
        // Fill this buffer with 0's so that once the other one ping-pongs back to us, we play silence
        buffers_clear(1);
      }
      break;

    case STATE_PLAYING_FROM_SPI:
      // If we finished playing a buffer, one more SPI buffer is free
      gSPIInputBuffersFree += (BUFFER_SIZE/SPI_STREAM_SIZE_BYTES);
      if (gSPIInputBuffersFree > (BUFFER_SIZE/SPI_STREAM_SIZE_BYTES)*2) {
        gSPIInputBuffersFree = (BUFFER_SIZE/SPI_STREAM_SIZE_BYTES)*2;
      }
      break;

    default:
      break;
  }
}

void play_stop(void)
{
  _dma_off();
  dma_wait_for_disable();
  dma_reset();

  rateclock_stop();

  if (gState == STATE_PLAYING_FROM_SD) {
    f_close(&gFile);
  }

  gState = STATE_IDLE;
}

void play_fill_buffer(void)
{
  if (gDMABufferDone) {
    UINT bytesRead;
    uint8_t readBufIx = 1 - gActiveDMABuffer; // Which buffer we are going to fill from SD card data

    // Have we marked a "last buffer to play"?
    if (gCtrlFlags & (CTRL_FLAG_BUFFER0_IS_LAST|CTRL_FLAG_BUFFER1_IS_LAST)) { 
      if (DMA.CTRL & DMA_CH_ENABLE_bm) {
        // Still transferring...just let it go
        return;
      }

      // DMA ISR has stopped the transfer process. We're done.
      play_stop();
      return;
    }

    gDMABufferDone = 0;

    if (! wav_fill_buffer((uint16_t *)(gBuffers[readBufIx]), &bytesRead)) {
      play_stop();
      return;
    }

    _transform_buffer((uint16_t *)gBuffers[readBufIx], BUFFER_SIZE/2);

    // Is this the last buffer? If so, fill it with 0's and set a flag indicating
    // that on the next ping-pong, we should quit.
    if (bytesRead < BUFFER_SIZE) {
      char *ptr;

      ptr = (char *)(gBuffers[readBufIx]) + bytesRead;
      memset(ptr, 0, BUFFER_SIZE-bytesRead);

      // Indicate that after the buffer we've just read in plays, we should stop
      gCtrlFlags = (1 << (readBufIx));
    }

    // Are we waiting to kickstart the playback?
    if (gCtrlFlags & CTRL_FLAG_KICKSTART) {
      rateclock_start(gWAVInfo.mSamplingRate); // DMA transfers will start shortly, triggered by Event Channel 0
      // Enable Channel 0. Let double-buffering action enable buffer 1 after first block of channel 0 is done.
      DMA.CH0.CTRLA |= DMA_ENABLE_bm;

      // Fill the other buffer now too
      gDMABufferDone = 1;
      gActiveDMABuffer = 0;
      gCtrlFlags &= ~CTRL_FLAG_KICKSTART;
    }
  }
}
// vim: ts=2 sw=2 ai expandtab cindent
