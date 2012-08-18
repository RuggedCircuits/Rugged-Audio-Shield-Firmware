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
 * This module handles DMA functions. It is used in the following ways:
 *
 *    - For SD card playback, ping-pong buffers are used to transfer data
 *      from the SD card drivers to the DAC
 *
 */
#include <inttypes.h>
#include <avr/interrupt.h>

#include "config.h"
#include "buffers.h"
#include "state.h"
#include "dac.h"
#include "play.h"
#include "rec.h"
#include "dma.h"

#if 0
void dma_init(void)
{
  gDMABufferDone = 0;
}
#endif

void dma_begin(DMAConfig_t config, uint8_t stereo)
{
  uint16_t addr;

  //gBufIx = 0;

  // Enable the DMA, configure DMA channels 0/1 for double buffering, allow default round-robin mode
  // (doesn't matter since in double-buffering only 1 DMA channel is enabled at a time).
  DMA.CTRL = DMA_CH_ENABLE_bm | DMA_DBUFMODE_CH01_gc;
  DMA.INTFLAGS = 0xFF; // Clear all interrupt flags
  
  // Configure Channel 0 to be enabled, 2/4-byte burst mode (to read mono/stereo
  // 16-bit samples), unlimited repeat, only a burst transfer on transfer
  // trigger, clear ERRIF and TRNIF flags, no interrupt on error, HIGH priority
  // interrupt on transfer complete, reload source after each block, increment
  // source, reload destination after each burst, increment destination,
  // trigger DMA transfer on Event Channel 0 (TCC0 setting sample rate) We use
  DMA.CH0.REPCNT = 0;
  DMA.CH1.REPCNT = 0; // Unlimited repeat
  if (stereo) {
    DMA.CH0.CTRLA = DMA_CH_REPEAT_bm | DMA_CH_BURSTLEN_4BYTE_gc | DMA_CH_SINGLE_bm;
    DMA.CH1.CTRLA = DMA_CH_REPEAT_bm | DMA_CH_BURSTLEN_4BYTE_gc | DMA_CH_SINGLE_bm;
  } else {
    DMA.CH0.CTRLA = DMA_CH_REPEAT_bm | DMA_CH_BURSTLEN_2BYTE_gc | DMA_CH_SINGLE_bm;
    DMA.CH1.CTRLA = DMA_CH_REPEAT_bm | DMA_CH_BURSTLEN_2BYTE_gc | DMA_CH_SINGLE_bm;
  }
  DMA.CH0.CTRLB = DMA_CH_ERRIF_bm | DMA_CH_TRNIF_bm | DMA_CH_ERRINTLVL_OFF_gc | DMA_CH_TRNINTLVL_HI_gc;
  DMA.CH1.CTRLB = DMA_CH_ERRIF_bm | DMA_CH_TRNIF_bm | DMA_CH_ERRINTLVL_OFF_gc | DMA_CH_TRNINTLVL_HI_gc;
  switch (config) {
    case DMA_CFG_PLAY:
      DMA.CH0.ADDRCTRL = DMA_CH_SRCRELOAD_BLOCK_gc | DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTRELOAD_BURST_gc | DMA_CH_DESTDIR_INC_gc;
      DMA.CH1.ADDRCTRL = DMA_CH_SRCRELOAD_BLOCK_gc | DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTRELOAD_BURST_gc | DMA_CH_DESTDIR_INC_gc;
      DMA.CH0.TRIGSRC = DMA_CH_TRIGSRC_EVSYS_CH0_gc;
      DMA.CH1.TRIGSRC = DMA_CH_TRIGSRC_EVSYS_CH0_gc;
      addr = (uint16_t) &(DACB.CH0DATA);
      DMA.CH0.DESTADDR0 = (addr % 256);
      DMA.CH0.DESTADDR1 = (addr >> 8);
      DMA.CH0.DESTADDR2 = 0;
      DMA.CH1.DESTADDR0 = (addr % 256);
      DMA.CH1.DESTADDR1 = (addr >> 8);
      DMA.CH1.DESTADDR2 = 0;
      addr = (uint16_t) gBuffers[0];
      DMA.CH0.SRCADDR0 = (addr % 256);
      DMA.CH0.SRCADDR1 = (addr >> 8);
      DMA.CH0.SRCADDR2 = 0;
      addr = (uint16_t) gBuffers[1];
      DMA.CH1.SRCADDR0 = (addr % 256);
      DMA.CH1.SRCADDR1 = (addr >> 8);
      DMA.CH1.SRCADDR2 = 0;
      break;

    case DMA_CFG_RECORD:
      DMA.CH0.ADDRCTRL = DMA_CH_SRCRELOAD_BURST_gc | DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTRELOAD_BLOCK_gc | DMA_CH_DESTDIR_INC_gc;
      DMA.CH1.ADDRCTRL = DMA_CH_SRCRELOAD_BURST_gc | DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTRELOAD_BLOCK_gc | DMA_CH_DESTDIR_INC_gc;
      DMA.CH0.TRIGSRC = DMA_CH_TRIGSRC_ADCA_CH1_gc;
      DMA.CH1.TRIGSRC = DMA_CH_TRIGSRC_ADCA_CH1_gc;
      addr = (uint16_t) gBuffers[0];
      DMA.CH0.DESTADDR0 = (addr % 256);
      DMA.CH0.DESTADDR1 = (addr >> 8);
      DMA.CH0.DESTADDR2 = 0;
      addr = (uint16_t) gBuffers[1];
      DMA.CH1.DESTADDR0 = (addr % 256);
      DMA.CH1.DESTADDR1 = (addr >> 8);
      DMA.CH1.DESTADDR2 = 0;
      addr = (uint16_t) &(ADCA.CH0RES);
      DMA.CH0.SRCADDR0 = (addr % 256);
      DMA.CH0.SRCADDR1 = (addr >> 8);
      DMA.CH0.SRCADDR2 = 0;
      DMA.CH1.SRCADDR0 = (addr % 256);
      DMA.CH1.SRCADDR1 = (addr >> 8);
      DMA.CH1.SRCADDR2 = 0;
      break;
  }

  DMA.CH0.TRFCNTL = (BUFFER_SIZE % 256); // Read/write 16-bit registers low-byte first
  DMA.CH0.TRFCNTH = (BUFFER_SIZE >> 8);
  DMA.CH1.TRFCNTL = (BUFFER_SIZE % 256); // Read/write 16-bit registers low-byte first
  DMA.CH1.TRFCNTH = (BUFFER_SIZE >> 8);
}

void dma_off(void)
{
  DMA.CH0.CTRLA = 0; // Disable DMA, but must wait until transfer buffer is done
  DMA.CH1.CTRLA = 0; // Disable DMA, but must wait until transfer buffer is done
  DMA.CTRL = 0; // Disable DMA controller
}

void dma_wait_for_disable(void)
{
  // Wait for transfer buffers to empty
  while (DMA.CH0.CTRLA & DMA_ENABLE_bm) /* NULL */ ;
  while (DMA.CH1.CTRLA & DMA_ENABLE_bm) /* NULL */ ;
  while (DMA.CTRL & DMA_ENABLE_bm) /* NULL */ ;
}

void dma_reset(void)
{
  // Reset the DMA channels and controller
  DMA.CH0.CTRLA = DMA_CH_RESET_bm;
  DMA.CH1.CTRLA = DMA_CH_RESET_bm;
  DMA.CTRL = DMA_CH_RESET_bm;
  gDMABufferDone = 0;
}

// Called when gActiveDMABuffer[0] is filled with recorded data
ISR(DMA_CH0_vect)
{
  gActiveDMABuffer = 1;
  gDMABufferDone = 1;
  DMA.CH0.CTRLB |= DMA_CH_TRNIF_bm; // TRNIF is NOT automatically cleared on interrupt

  switch (gState) {
    case STATE_PLAYING_FROM_SD: 
    case STATE_PLAYING_FROM_SPI:
      play_dma_ch0_isr(); 
      break;

    case STATE_RECORDING_TO_SPI:
      rec_dma_isr();
      break;

    default: 
      break;
  }
}

// Called when gActiveDMABuffer[1] is filled with recorded data
ISR(DMA_CH1_vect)
{
  gActiveDMABuffer = 0;
  gDMABufferDone = 1;
  DMA.CH1.CTRLB |= DMA_CH_TRNIF_bm; // TRNIF is NOT automatically cleared on interrupt

  switch (gState) {
    case STATE_PLAYING_FROM_SD: 
    case STATE_PLAYING_FROM_SPI:
      play_dma_ch1_isr(); 
      break;

    case STATE_RECORDING_TO_SPI:
      rec_dma_isr();
      break;

    default:
      break;
  }
}

// vim: ts=2 sw=2 ai expandtab cindent
