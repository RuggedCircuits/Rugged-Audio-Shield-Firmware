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
#ifndef _TIMER_H_
#define _TIMER_H_

#include <inttypes.h>
#include "config.h"

extern volatile uint16_t Tick10ms;

extern void TimerInit(void);
extern void delaycentiseconds(uint16_t centiseconds);

#if BOOTLOADER==0
extern uint8_t IsAlarm1(void);
extern void SetAlarm1(uint16_t centiseconds);
extern void rtc_interrupt(uint8_t ison);
#endif

#endif /* _TIMER_H_ */

// vim: ts=2 ai expandtab
