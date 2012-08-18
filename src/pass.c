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
#include <avr/io.h>
#include <avr/interrupt.h>
#include "config.h"
#include "dma.h"
#include "state.h"
#include "buffers.h"
#include "rec.h"
#include "adc.h"
#include "rateclock.h"
#include "i2c.h"
#include "play.h"
#include "pass.h"

static uint8_t gEffect; // '0' through '9'
static uint8_t gStereo;

// Pointer to the next location to store incoming samples
static int16_t *gPassBufferPtr;

// We treat pass-through as essentially a "play" function, using DMA function for
// playback when possible, or doing direct-to-DAC when not.
void pass_through(uint8_t effect, uint16_t Fs, uint8_t stereo, uint8_t source)
{
  gState = STATE_PASS_THROUGH;

  gEffect = effect;
  gStereo = stereo;

  gPassBufferPtr = (int16_t *)gBuffers;
  gCtrlFlags = CTRL_FLAG_KICKSTART;

  buffers_clear_all(); // Make sure we start with 0's in all samples

  // Rate clock will trigger ADC conversions and DMA transactions. We will enable DMA, however, only
  // after we have some samples to play.
  adc_start(source ? REC_MIC : REC_LINE);
  dma_begin(DMA_CFG_PLAY, stereo);
  rateclock_start(Fs);

#if 0 // r1: let user fully control OutputEnable to avoid clicks and pops
  I2C_shutdown_enable(0);
#endif
}

void pass_stop(void)
{
  gEffect = 0; // No effect, so that ISR below doesn't try to do anything
  play_stop();
  adc_stop();
}

// Given samples coming from the A/D, store them in ADC format (signed) into the buffers
static void _store_samples_signed(int16_t L, int16_t R)
{
  *gPassBufferPtr++ = L;
  if (gStereo) {
    *gPassBufferPtr++ = R;
  }
  if (gPassBufferPtr == (int16_t *)gBuffersEnd) {
    gPassBufferPtr = (int16_t *)gBuffers;
  }
}

// Given samples coming from the A/D, store them in DAC format (unsigned) into the buffers
static inline void _store_samples_unsigned(int16_t L, int16_t R)
{
  _store_samples_signed(L ^ 0x8000U, R ^ 0x8000U);
  return;
#if 0
  // Since these are being played back to our DAC's, which use unsigned format, we have
  // to convert signed-->unsigned (see the documentation to _transform_buffer() in play.c)
  *gPassBufferPtr++ = L ^ 0x8000;
  if (gStereo) {
    *gPassBufferPtr++ = R ^ 0x8000;
  }
  if (gPassBufferPtr == (int16_t *)gBuffersEnd) {
    gPassBufferPtr = (int16_t *)gBuffers;
  }
#endif
}

static void _lets_get_it_started(void)
{
  DMA.CH0.CTRLA |= DMA_ENABLE_bm;
  gCtrlFlags &= ~CTRL_FLAG_KICKSTART;
}

// MUST HAVE ISR for Channel 1, since this is the DMA trigger and DMA is triggered from interrupt flag,
// and interrupt flag only clears when interrupt is serviced. We put the ISR here since it saves
// a function call from adc.c to pass.c and audio effects are time-critical.
ISR(ADCA_CH1_vect)
{
  if (! gEffect) {
    return;
  } else {
    int16_t sampleL, sampleR;

    sampleL = ADCA.CH0RES;
    sampleR = ADCA.CH1RES;

    switch (gEffect) {
      default:
        break;

      case '0':     // No effect
        _store_samples_unsigned(sampleL, sampleR);
        if (gCtrlFlags & CTRL_FLAG_KICKSTART) {
          _lets_get_it_started();
        }
        break;

      case '1':     // Pure echo...add in a scaled version of a delayed signal
        {
          int16_t *curbuf   = gPassBufferPtr;
          int16_t *delaybuf = gPassBufferPtr - (BUFFER_SIZE/2-2) - (gStereo ? (BUFFER_SIZE/2-2) : 0);

          if (delaybuf < (int16_t *)gBuffers) delaybuf += BUFFER_SIZE;
          _store_samples_signed(sampleL, sampleR);

          *delaybuf = ((*delaybuf>>1) + (*curbuf)) ^ 0x8000U;
          if (gStereo) {
            delaybuf++; curbuf++;
            *delaybuf = ((*delaybuf>>1) + (*curbuf)) ^ 0x8000U;
          }
        }
        if ((gCtrlFlags & CTRL_FLAG_KICKSTART) && ((gPassBufferPtr - (int16_t *)gBuffers) > (BUFFER_SIZE-4))) {
          _lets_get_it_started();
        }
        break;

      case '2':     // Comb filter, implement the function y[n] = (x[n] - x[n-64])
        {
          int16_t *curbuf   = gPassBufferPtr;
          int16_t *delaybuf = gPassBufferPtr - 64 - (gStereo ? 64 : 0);

          if (delaybuf < (int16_t *)gBuffers) delaybuf += BUFFER_SIZE;
          _store_samples_signed(sampleL, sampleR);

          *delaybuf = (-(*delaybuf>>0) + (*curbuf>>0)) ^ 0x8000U;
          if (gStereo) {
            delaybuf++; curbuf++;
            *delaybuf = (-(*delaybuf>>0) + (*curbuf>>0)) ^ 0x8000U;
          }
        }
        if ((gCtrlFlags & CTRL_FLAG_KICKSTART) && ((gPassBufferPtr - (int16_t *)gBuffers) > 128)) {
          _lets_get_it_started();
        }
        break;
    }
  }
}
// vim: expandtab ts=2 sw=2 ai cindent
