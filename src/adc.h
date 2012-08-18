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
#ifndef _ADC_H_
#define _ADC_H_

#include "rec.h"

// These are basically values to write to the ADCA.CHx.CTRL register to control the gain
typedef enum {
  ADC_DIFF_GAIN_1X,
  ADC_DIFF_GAIN_2X,
  ADC_DIFF_GAIN_4X,
  ADC_DIFF_GAIN_8X
} ADCGain_t;

extern void adc_init(void);
extern void adc_start(RecType_t type);
extern void adc_stop(void);
extern void adc_set_gains(ADCGain_t line, ADCGain_t mic);

#endif // _ADC_H_
// vim: expandtab ts=2 ai sw=2 cindent
