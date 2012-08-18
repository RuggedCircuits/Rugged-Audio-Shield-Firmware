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
 * This module implements a bit-banging I2C driver for communicating with the MAX9723D.
 */

#include <avr/io.h>
#include "config.h"
#include "timer.h"
#include "i2c.h"

static void _bit_delay(void) __attribute__((naked));
static void _bit_delay(void)
{
  asm volatile (
" ldi r24,105\n"           // 1 cycle
"_bit_delay1:\n"
"  dec r24\n"               // 1 cycle
"  brne _bit_delay1\n"      // 2 cycles if branch taken, 1 if not
"  ret\n"                   // 4 cycles
/* Total execution time is 5 + 3*(N-1) + 2 cycles (or 4+3N), where N is the value loaded into R24.
   To achive a bit time of 100 kHz (10us) at 32 MHz clock frequency we want the loop
   to take 10us*32MHz = 320 cycles = 4+3N --> N=105.
 */
  );
}

#define SDA_MASK _BV(0)
#define SCL_MASK _BV(1)

static uint8_t gLastByte;

static inline void SCL_LOW(void) { PORTC.DIRSET = SCL_MASK; }
static inline void SCL_HIGH(void) { PORTC.DIRCLR = SCL_MASK; }
static inline void SDA_LOW(void) { PORTC.DIRSET = SDA_MASK; }
static inline void SDA_HIGH(void) { PORTC.DIRCLR = SDA_MASK; }

static void _start_bit(void) 
{ 
  SDA_LOW(); 
  _bit_delay();
}
static void _stop_bit(void)
{
  SCL_LOW();
  _bit_delay();
  SDA_LOW();
  _bit_delay();
  SCL_HIGH();
  _bit_delay();
  SDA_HIGH();
}

static void _write_byte(uint8_t val)
{
  uint8_t bit;

  for (bit=0; bit < 8; bit++, val <<= 1) {
    SCL_LOW();
    if (val & (uint8_t)0x80U) {
      SDA_HIGH();
    } else {
      SDA_LOW();
    }
    _bit_delay();
    SCL_HIGH();
    _bit_delay();
  }

  // Now slave should be pulling SDA low when we release SDA to go high
  SCL_LOW();
  SDA_HIGH();
  _bit_delay();
  // Could check to see if SDA is low at this point, to indicate slave ACK
  SCL_HIGH();
  _bit_delay();

  // Both SCL and SDA are left high, as required
}

static void _update(void)
{
  _start_bit();
  _write_byte(0x9A); // Address 0b1001101_0 (with R/W bit set to 0)
  _write_byte(gLastByte);
  _stop_bit();
}

void I2C_init(void)
{
  /*
   * Configuration as I2C master:
   *      PC0 is SDA
   *      PC1 is SCL
   *
   * We set all pin drivers as 0 and make them inputs. To drive low, we enable the pin as an output.
   * We have an external pullup to keep SCA/SCL high.
   */
  PORTC.DIRCLR = SDA_MASK | SCL_MASK;
  PORTC.OUTCLR = SDA_MASK | SCL_MASK;

  gLastByte = 0xFFU; // Default on powerup
}

// Write 5 LSB's of MAX9723D command register to set the gain
void I2C_set_volume(uint8_t gain)
{
  gain &= (uint8_t)0x1FU;

  gLastByte = (gLastByte & (uint8_t)~0x1FU) | gain;
  _update();
}

static void _bit_control(uint8_t mask, uint8_t enable)
{
  if (enable) {
    gLastByte |= mask;
  } else {
    gLastByte &= ~mask;
  }
  _update();
}

void I2C_maxgain_enable(uint8_t enable)
{
  _bit_control(_BV(5), enable);
}

void I2C_bassmax_enable(uint8_t enable)
{
  _bit_control(_BV(6), enable);
}

void I2C_shutdown_enable(uint8_t enable)
{
  _bit_control(_BV(7), !enable);
}

void I2C_set_direct(uint8_t val)
{
  gLastByte = val;
  _update();
}
// vim: expandtab ts=2 ai sw=2 cindent
