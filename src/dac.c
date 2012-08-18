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
 * This configures the ADC to output audio data on left channel (DAC0) and right channel (DAC1)
 */

#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "config.h"
#include "rec.h"
#include "timer.h"
#include "sio.h"
#include "utils.h"
#include "dac.h"

void dac_init(void)
{
#if WITH_DAC_RIGHT==1
  // Configure for no internal DAC re-route, enable DAC 0 and 1 to output pin, enable,
  // sample-and-hold mode, 1.25V external reference, left-adjusted, refresh every 1 us.
  DACB.CTRLA = DAC_CH0EN_bm | DAC_CH1EN_bm | DAC_ENABLE_bm;
  DACB.CTRLB = DAC_CHSEL_DUAL_gc;
  DACB.CTRLC = DAC_REFSEL_AREFA_gc | DAC_LEFTADJ_bm; // 1.25V reference on AREF
  DACB.TIMCTRL = DAC_CONINTVAL_128CLK_gc  // 192 clocks @ 32 MHz --> 6 microseconds settling time
                | DAC_REFRESH_32CLK_gc ; // 32 clocks is the default startup value
#else
  // Configure for no internal DAC re-route, enable DAC 0 to output pin, enable,
  // single-channel mode (not sample-and-hold), 1.25V external reference, left adjusted
  // DAC1 is currently off (right audio channel).
  DACB.CTRLA = DAC_CH0EN_bm | DAC_ENABLE_bm;
  DACB.CTRLB = DAC_CHSEL_SINGLE_gc; // DAC0 left channel only for now
  DACB.CTRLC = DAC_REFSEL_AREFA_gc | DAC_LEFTADJ_bm; // 1.25V reference on AREF
  DACB.TIMCTRL = DAC_CONINTVAL_128CLK_gc | DAC_REFRESH_OFF_gc; // 128 clocks @ 32 MHz --> 4 microseconds settling time, no sample-and-hold refresh
#endif
}

// vim: expandtab ts=2 ai sw=2 cindent
