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
 * This configures the ADC to read channels 6 and 7 for the line-in Left and Right channels, respectively.
 * It can also configure channel 5 for reading the microphone.
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
#include "state.h"
#include "adc.h"

// Control the amount of differential gain
static uint8_t gGainVal[] = {
  ADC_CH_GAIN_1X_gc | ADC_CH_INPUTMODE_DIFFWGAIN_gc,     // No gain
  ADC_CH_GAIN_2X_gc | ADC_CH_INPUTMODE_DIFFWGAIN_gc,
  ADC_CH_GAIN_4X_gc | ADC_CH_INPUTMODE_DIFFWGAIN_gc,
  ADC_CH_GAIN_8X_gc | ADC_CH_INPUTMODE_DIFFWGAIN_gc
};

static uint8_t gGainLINE, gGainMIC;

void adc_set_gains(ADCGain_t line, ADCGain_t mic)
{
  gGainLINE = gGainVal[line];
  gGainMIC = gGainVal[mic];
}

// Read calibration byte
static uint8_t read_cal_byte( uint8_t index )
{
  uint8_t result;

  /* Load the NVM Command register to read the calibration row. */
  NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
  result = pgm_read_byte(index);

  /* Clean up NVM Command register. */
  NVM_CMD = NVM_CMD_NO_OPERATION_gc;

  return result;
}

void adc_init(void)
{
  // Set default gains. Can always be changed by user.
  gGainLINE = gGainMIC = gGainVal[0]; // 1X gain

  // We have a 1.25V reference on ADC0 and a 1.25V reference on ADC1, as well as a 1.25V reference
  // on ADC4. AREFA is ADC0, hence 1.25V. Since the line-in analog inputs are biased to 1.25V, we
  // configure the ADC for differential mode with ADC1 as the negative terminal, and ADC0 as the voltage
  // reference (presumably the reference is cleaner if it is not also an input, but this is not confirmed).
  // App note AVR1300 says wait to enable until everything is configured
  //ADCA.CTRLA = ADC_ENABLE_bm; // Enable the converter

#if 0
  // Configure for non-free-running mode, signed mode (CONMODE), 12-bit right-adjusted mode (automatically sign-extended)
  ADCA.CTRLB = ADC_CONMODE_bm | ADC_RESOLUTION_12BIT_gc;
#else
  // Configure for non-free-running mode, signed mode (CONMODE), 12-bit left-adjusted mode
  ADCA.CTRLB = ADC_CONMODE_bm | ADC_RESOLUTION_LEFT12BIT_gc;
#endif

  // Use ADC0 (1.25V) as the reference
  ADCA.REFCTRL = ADC_REFSEL_AREFA_gc;

  /* Prescaler frequency divides 32 MHz clock to get ADC clock. Due to pipelining, we simply
     have the the ADC clock frequency must be simply higher than the sampling rate (not sampling
     rate multiplied by the number of bits!). However, we need to (in stereo mode) sample two
     sets of samples per clock so the ADC clock must be higher than twice the sampling rate.
     Assuming we occasionally want to sample at 44.1 kHz we have Fadc >= 88.2kHz and the only
     way to get that is with a prescale of 256 so that 32 MHz/256 --> 125 kHz.
     */
  ADCA.PRESCALER = ADC_PRESCALER_DIV256_gc;

  // Set the event control register to include channels 0 and 1 in a channel sweep when triggered.
  // The sweep will be synchronized with the event, and will be on event channel 0, which we configure
  // to be a timer set up for the sampling rate.
  //
  // "Synchronized sweep" DOES NOT WORK!!! Sounds like a nice idea but an event simply does not 
  // trigger a conversion with sync-sweep.
  ADCA.EVCTRL = ADC_SWEEP_01_gc | ADC_EVSEL_0123_gc | ADC_EVACT_SWEEP_gc;

  // Clear interrupt flags
  ADCA.INTFLAGS = 0xF;

  // NOTE: Calibration should be done BEFORE enabling the converter! (cf. app note AVR1300 Section 3.12 Calibration)

  // Load calibration bytes into ADC
  ADCA.CALL = read_cal_byte( offsetof(NVM_PROD_SIGNATURES_t, ADCACAL0) );
  ADCA.CALH = read_cal_byte( offsetof(NVM_PROD_SIGNATURES_t, ADCACAL1) );
}

void adc_start(RecType_t type)
{
  uint8_t muxctrl;
  uint8_t gain;

  // Channel 0 : PA6 (LINE_L) or PA5 (MIC) depending on MIC/LINE
  ADCA.CH0.CTRL = gain = (type==REC_LINE) ? gGainLINE : gGainMIC;
  //if (gain == ADC_DIFF_GAIN_1X) {
  //  muxctrl = ADC_CH_MUXNEG_PIN1_gc;
  //} else {
    muxctrl = 0; // Pin 4 -- header file define for pin 4 is wrong
  //}

  // Negative input will be ADC1 for single-ended (1X) gain, ADC4 for differential gain
  // Select PA6 as positive input for Channel 0 (or PA5 for mic recording), ADC4 (1.25V) as negative input.
  // NOTE: Negative input can only be ADC0-ADC3 for differential input without
  // gain, and only ADC4-ADC7 for differential input with gain! Our configuration ALWAYS uses
  // some type of differential gain so ADC4 will ALWAYS be the negative reference.
  switch (type) {
    case REC_LINE: default:
      ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN6_gc | muxctrl;
      break;

    case REC_MIC:
      ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN5_gc | muxctrl;
      break;
  }

  ADCA.CH1.CTRL = gGainLINE;
  //if (gGainLINE == ADC_DIFF_GAIN_1X) { // This line is no longer right now that ADCGain_t is not the actual value to write to the CTRL register
  //  muxctrl = ADC_CH_MUXNEG_PIN1_gc;
  //} else {
    muxctrl = 0; // Pin 4 -- header file define for pin 4 is wrong
  //}
  switch (type) {
    case REC_LINE: default:
      ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN7_gc | muxctrl;
      break;

    case REC_MIC:
      ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN6_gc | muxctrl;
      break;
  }

  // Only CH1 will trigger interrupts. An interrupt on CH1 indicates both CH0 and CH1 have samples.
  // MUST ENABLE interrupt for DMA source (Channel 1) since it appears that it is the INTFLAG which
  // triggers the DMA, and if the flag is not cleared (by virtue of servicing the interrupt) then
  // the DMA is just repeatedly triggered without waiting for conversions to complete.
  ADCA.CH1.INTFLAGS = 1;
  // Configure interrupt for high priority, conversion complete
  ADCA.CH1.INTCTRL = ADC_CH_INTMODE_COMPLETE_gc | ADC_CH_INTLVL_HI_gc;

  // Now start the conversions
  ADCA.CTRLA = ADC_ENABLE_bm; // Enable the converter
  ADCA.CTRLA = ADC_CH0START_bm | ADC_CH1START_bm | ADC_ENABLE_bm; // ...and start Channel 0 conversions and Channel 1 conversions
}

void adc_stop(void)
{
  ADCA.CH1.INTCTRL = 0; // Stop generating interrupts
  ADCA.CTRLA = 0; // Disable A/D's
}

// vim: expandtab ts=2 ai sw=2 cindent
