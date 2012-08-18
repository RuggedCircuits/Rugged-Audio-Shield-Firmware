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
 * This module generates a 10ms timer tick using the RTC. Much finer resolution really isn't possible
 * given that the RTC clock domain only updates its registers once every millisecond. We could
 * probably get finer resolution using an actual timer peripheral. We can easily get 10ms down to 4ms
 * but 10ms is a nice round number and meshes well with the MMC low-level timeout code.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "timer.h"
#include "diskio.h"

#define interruptsOFF cli
#define interruptsON  sei

/*
 * For 16-bit read/writes:
 *
 *   - Write low before high to synchronize to RTC clock domain
 *   - The PERIOD register only synchronizes when another register is also written to
 *   - Check SYNCBUSY flag in STATUS register to see when synchronization is complete
 */

volatile uint16_t Tick10ms;
#if BOOTLOADER==0
static volatile uint8_t Alarm1Set;
static uint16_t Alarm1Match;
#endif

#if 0
ISR(RTC_OVF_vect) // Doesn't seem to work using the PER=0 approach. It's too slow by a factor of 3, probably due to clock domain synchronization.
#else
ISR(RTC_COMP_vect, ISR_NOBLOCK)
#endif
{
  uint16_t comp;

  comp = RTC.COMP + 10;
  RTC.COMPL = (uint8_t)comp;
  RTC.COMPH = (uint8_t)(comp>>8);
  
  // OVFIF automatically cleared on ISR
  // CMPIF automatically cleared on ISR
#if BOOTLOADER==0
  if (++Tick10ms == Alarm1Match) {
    Alarm1Set = 1;
  }
#endif

  // Kick MMC functions
  disk_timerproc();
}

#if BOOTLOADER==0
uint8_t IsAlarm1(void)
{
  return Alarm1Set;
}

void SetAlarm1(uint16_t centiseconds)
{
  Alarm1Match = Tick10ms + centiseconds + 1; // +1 to delay at least 10ms
  Alarm1Set = 0;
}
#endif

static void wait_for_sync(void)
{
  while (RTC.STATUS & RTC_SYNCBUSY_bm) /* NULL */ ;
}

void TimerInit(void)
{
        /* We use the internal 32 kHz oscillator (not the ultra-low-power one).
         * It has to first be enabled in the Oscillator CTRL register, then selected
         * as the RTC clock source in the RTCCTRL register.
         */

  OSC.CTRL |= OSC_RC32KEN_bm; // Enable internal 32kHz oscillator
  while (! (OSC.STATUS & OSC_RC32KRDY_bm)) /* NULL */ ; // Wait for it to be ready

  CLK.RTCCTRL |= (2 << CLK_RTCSRC_gp); // Select internal 32 kHz oscillator divided to 1 kHz
  CLK.RTCCTRL |= CLK_RTCEN_bm; // Enable RTC clock

#if 0 // This generates an interrupt every 3 RTC cycles, i.e., every 3 ms. Not what we want.
  RTC.PERL = 0;
  RTC.PERH = 0;
  wait_for_sync();
#endif

  RTC.COMPL = 50; // Give it 50ms to get stuff started so we don't miss the first compare
  RTC.COMPH = 0;
  wait_for_sync();

  RTC.CNTL = 0; // Write low first...
  RTC.CNTH = 0; // ...then write high to synchronize, and this should synchronize PERIOD too.
  wait_for_sync();

#if 0
  RTC.INTFLAGS = RTC_OVFIF_bm; // Clear overflow flag
  RTC.INTCTRL = RTC_OVFINTLVL_LO_gc; // Enable low-priority interrupt on overflow
#endif
  RTC.INTFLAGS = RTC_COMPIF_bm; // Clear compare flag
  RTC.INTCTRL = RTC_COMPINTLVL_LO_gc; // Enable low-priority interrupt on compare

  RTC.CTRL = 1; // Divide-by-1 prescale (1 kHz)
  wait_for_sync();
}

// Rough delay function that actually delays by 10ms chunks, and rounds up to the nearest
// 10ms chunk. E.g., delaycentiseconds(0) will delay for anywhere between 0ms and 10ms, delaycentiseconds(1)
// will delay for anywhere between 10ms and 20ms, etc.
void delaycentiseconds(uint16_t cs)
{
  uint16_t timeout = Tick10ms + cs + 1;

  while (Tick10ms != timeout) /* NULL */ ;
}

// Enable or disable the RTC interrupt
#if BOOTLOADER==0
void rtc_interrupt(uint8_t ison)
{
  if (ison) {
    RTC.INTCTRL = RTC_COMPINTLVL_LO_gc; // Enable low-priority interrupt on compare
  } else {
    RTC.INTCTRL = 0;
  }
}
#endif
// vim: expandtab ts=2 ai cindent sw=2
