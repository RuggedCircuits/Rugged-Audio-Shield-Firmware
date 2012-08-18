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
 * This module handles clock switching.
 *
 * NOTE: The code supports 16 MHz and 32 MHz frequencies based on an EXTERNAL
 * clock crystal, as well as the internal 32 MHz oscillator mode. The AD210
 * board has no external crystal thus only the internal 2MHz and 32 MHz modes
 * are supported by this module.
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>
#include <avr/io.h>

#include "config.h"
#include "main.h"
#include "utils.h"
#include "clocks.h"

static ClockSource_t clockSource = CLKSRC_2MHZ; // Internal 2 MHz oscillator
#if WITH_SIO==1
//static uint32_t clockFreqs[] = { 2000000UL, 16000000UL, 32000000UL, 32000000UL };
#endif

// Turn the internal 2 MHz oscillator on or off, and wait for it to stabilize if turned on.
static void clock2M(uint8_t enable)
{
  if (enable) {
    OSC.CTRL |= OSC_RC2MEN_bm;
    while (! (OSC.STATUS & OSC_RC2MRDY_bm)) /* NULL */ ;
  } else {
    OSC.CTRL &= ~OSC_RC2MEN_bm;
  }
}

// Turn the internal 32 MHz oscillator on or off, and wait for it to stabilize if turned on.
static void clock32M(uint8_t enable)
{
  if (enable) {
    OSC.CTRL |= OSC_RC32MEN_bm;
    while (! (OSC.STATUS & OSC_RC32MRDY_bm)) /* NULL */ ;
  } else {
    OSC.CTRL &= ~OSC_RC32MEN_bm;
  }
}

// Turn the external oscillator on or off, and wait for it to stabilize if turned on.
static void clockXOSC(uint8_t enable)
{
#ifdef HAVE_EXTERNAL_OSC
  OSC.CTRL &= ~OSC_XOSCEN_bm; // Must disable XOSCEN to change its parameters
  if (enable) {
    OSC.XOSCCTRL = OSC_FRQRANGE_12TO16_gc | OSC_XOSCSEL_XTAL_16KCLK_gc;
    OSC.CTRL |= OSC_XOSCEN_bm; // Enable XOSCEN
    while (! (OSC.STATUS & OSC_XOSCRDY_bm)) /* NULL */ ; // Wait for it to stabilize
  }
#else
  (void) enable;
#endif
}

// Turn on the PLL to 32 MHz from the external 16 MHz oscillator. It is assumed that XOSC
// has already been enabled.
static void clockPLL(uint8_t enable)
{
#ifdef HAVE_EXTERNAL_OSC
  OSC.PLLCTRL = OSC_PLLSRC_XOSC_gc | 2; // Multiplication factor of 2
  if (enable) {
    OSC.CTRL |= OSC_PLLEN_bm; // Enable PLL
    while (! (OSC.STATUS & OSC_PLLRDY_bm)) /* NULL */ ; // Wait for lock
  } else {
    OSC.CTRL &= ~OSC_PLLEN_bm; // Disable PLL
  }
#else
  (void) enable;
#endif // HAVE_EXTERNAL_OSC
}

// Write to the CLK.CTRL register to switch the main clock source. It is assumed the clock
// source is already enabled and stable. At the end of this function the CPU frequency will have
// changed.
static void clockSwitchTo(ClockSource_t clk)
{
  switch (clk) {
    default:
    case CLKSRC_2MHZ: // Switch to 2 MHz internal clock
      CCP = CCP_IOREG_gc; // Enable configuration change to protected I/O registers
      CLK.CTRL = CLK_SCLKSEL_RC2M_gc; // Select internal 2 MHz clock
      break;

#ifdef HAVE_EXTERNAL_OSC
    case CLKSRC_16MHZ_EXT: // Switch to external oscillator
      CCP = CCP_IOREG_gc; // Enable configuration change to protected I/O registers
      CLK.CTRL = CLK_SCLKSEL_XOSC_gc; // Select external oscillator
      break;

    case CLKSRC_32MHZ_EXT: // Switch to PLL
      CCP = CCP_IOREG_gc; // Enable configuration change to protected I/O registers
      CLK.CTRL = CLK_SCLKSEL_PLL_gc; // Select PLL
      break;
#endif // HAVE_EXTERNAL_OSC

    case CLKSRC_32MHZ_INT:  // Switch to 32 MHz internal clock
      CCP = CCP_IOREG_gc; // Enable configuration change to protected I/O registers
      CLK.CTRL = CLK_SCLKSEL_RC32M_gc; // Select internal 32 MHz clock
      break;
  }
  nop(); // 2 cycles on old clock
  nop();
  nop(); // 2 cycles on new one
  nop();
}

// Co-ordinate all aspects of switching clock frequencies, including enabling clock sources,
// switching to the new clock, and disabling unneeded clock sources.
void setClockSource(ClockSource_t clk)
{
#if 0 // Do it anyways as it initializes the serial port
  if (clockSource == clk) return; // Nothing to do
#endif

  clockSource = clk;

  // Do configuration changes using internal 2 MHz clock
  clock2M(1); // Enable 2 MHz internal clock
  clockSwitchTo(CLKSRC_2MHZ); // Now we're running at 2 MHz

  switch (clk) {
    default:
      clockSource = CLKSRC_2MHZ; // In case an illegal clock source was specified
      // fall through

    case CLKSRC_2MHZ:   // Internal 2 MHz clock...already running from it
      clockPLL(0); // Disable PLL
      clockXOSC(0); // Disable external oscillator
      clock32M(0); // Disable 32 MHz internal oscillator, if it was running
      break;

#ifdef HAVE_EXTERNAL_OSC
    case CLKSRC_16MHZ_EXT: // External 16 MHz oscillator
      clockXOSC(1); // Enable XOSC
      clockSwitchTo(CLKSRC_16MHZ_EXT); // Now we're running at 16 MHz. We can turn 2 MHz and PLL off.
      clock2M(0); // Turn off internal 2 MHz oscillator
      clock32M(0); // Disable 32 MHz internal oscillator, if it was running
      clockPLL(0);
      break;

    case CLKSRC_32MHZ_EXT: // External 16 MHz oscillator, PLL up to 32 MHz
      clockXOSC(1); // Enable XOSC
      clockPLL(1); // Enable PLL
      clockSwitchTo(CLKSRC_32MHZ_EXT); // Switch to PLL for system clock. We can turn 2 MHz off.
      clock2M(0); // Turn off internal 2 MHz oscillator
      clock32M(0); // Disable 32 MHz internal oscillator, if it was running
      break;
#endif

    case CLKSRC_32MHZ_INT:  // Internal 32 MHz oscillator
      clock32M(1); // Enable 32 MHz internal oscillator
      clockPLL(0); // Disable PLL
      clockXOSC(0); // Disable external oscillator
      clockSwitchTo(CLKSRC_32MHZ_INT); // Switch to internal 32 MHz oscillator
      clock2M(0); // Turn off internal 2 MHz oscillator
      break;
  }
}

ClockSource_t getClockSource(void)
{
  return clockSource;
}

// vim: ts=2 sw=2 ai expandtab cindent
