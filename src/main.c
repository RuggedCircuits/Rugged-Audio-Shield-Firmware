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
 * This program is intended to reside on a Rugged Audio Shield, specifically
 * on an ATxmega32A4 initially clocked with a 2 MHz internal clock and
 * with no external crystal. The system is designed to run at 32 MHz
 * using the internal 32 MHz oscillator with DFLL enabled for higher accuracy.
 *
 * Global event channel usage:
 *
 *     Channel 0 : Timer TCC0 set up to trigger audio events at the desired sampling rate
 *     Channel 1 : not used
 *     Channel 2 : not used
 *     Channel 3 : not used
 *     Channel 4 : not used
 *     Channel 5 : not used
 *     Channel 6 : not used
 *     Channel 7 : not used
 *     Channel 8 : not used
 *
 * Timer usage:
 *
 *      TCC0 : Generates Event 0, sampling rate on TCC0 overflow
 *      TCC1 : not used
 *      TCD0 : not used
 *      TCD1 : not used
 *      TCE0 : not used
 *
 * Interrupt usage:
 *     * SPI from Arduino -- HIGH LEVEL (medium priority among HIGH LEVEL)
 *     * ADC for Line/Mic -- HIGH LEVEL (lowest priority among HIGH LEVEL)
 *     * DMA -- HIGH LEVEL (highest priority among HIGH LEVEL)
 *
 *     * USART -- MEDIUM LEVEL
 *       When WITH_SIO_INTERRUPTS is defined, generates interrupts on character
 *       reception/transmission (sio.c) 
 *
 *     * RTC -- LOW LEVEL
 *       Generates a 10ms timer tick (timer.c).
 *
 */

#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "sio.h"
#include "utils.h"
#include "timer.h"
#include "clocks.h"
#include "adc.h"
#include "dac.h"
#include "rec.h"
#include "buffers.h"
#include "state.h"
#include "play.h"
#include "spi_C_slave.h"
#include "i2c.h"
#include "diskio.h"
#include "fail.h"
#include "printf.h"

static void set_all_inputs(void)
{
// Configure all pins as inputs, input sense both edges, outputs are totem-pole when outputs and have pulldowns 
// when inputs
#if 0 // Already done in bootloader
  PORTA.DIR = 0;
  PORTCFG.MPCMASK = 0xFF; PORTA.PIN0CTRL = PORT_OPC_PULLDOWN_gc; 
  PORTB.DIR = 0;
  PORTCFG.MPCMASK = 0xFF; PORTB.PIN0CTRL = PORT_OPC_PULLDOWN_gc;
  PORTC.DIR = 0;
  PORTCFG.MPCMASK = 0xFF; PORTC.PIN0CTRL = PORT_OPC_PULLDOWN_gc;
  PORTD.DIR = 0;
  PORTCFG.MPCMASK = 0xFF; PORTD.PIN0CTRL = PORT_OPC_PULLDOWN_gc;
  PORTE.DIR = 0;
  PORTCFG.MPCMASK = 0xFF; PORTE.PIN0CTRL = PORT_OPC_PULLDOWN_gc;
#endif

  // Disable pulldowns on Port A analog inputs
  PORTA.PIN7CTRL = PORT_OPC_TOTEM_gc | PORT_ISC_INPUT_DISABLE_gc;
  PORTA.PIN6CTRL = PORT_OPC_TOTEM_gc | PORT_ISC_INPUT_DISABLE_gc;
  PORTA.PIN5CTRL = PORT_OPC_TOTEM_gc | PORT_ISC_INPUT_DISABLE_gc;

  // Enable microphone output by negating !SHDN
  // Only necessary on Rev. A hardware. Rev. B hardware has this always enabled (can shutdown over I2C)
#if 0
  PORTC.DIRSET = _BV(2);
  PORTC.OUTSET = _BV(2);
#endif
  
  // SD_!DET needs a pull-up since it's a dry contact to GND
#if 0 // Should already be done in bootloader
  PORTD.PIN3CTRL = PORT_OPC_PULLUP_gc;
#endif
}

int main(void) __attribute__((noreturn));
int main(void)
{
  uint8_t bootloader_code;

  // Turn off interrupts
#if 0 // Should already be true due to bootloader
  cli();
#endif
  bootloader_code = GPIOF;

  // Initialize static data of SD drivers. Ignore warning about "null argument where non-null required"
  memset((void *)0, 0, 16); // Zero out pseudo-bss section of SD card drivers
  GPIO0 = STA_NOINIT;       // The only initialized piece of data. MUST CO-ORDINATE WITH BOOTLOADER mmc.c!

  // Disable power to various components not used in this application.
  PR.PRGEN = PR_AES_bm; // Don't need AES
  PR.PRPA  = PR_AC_bm;  // Don't need analog comparator on Port A
  PR.PRPB  = PR_ADC_bm; // Don't need ADC on Port B

  // NOTE!!! Many timers are being disabled below! We currently don't need them.
  PR.PRPC  = PR_TC1_bm | PR_HIRES_bm | PR_USART0_bm | PR_USART1_bm | PR_TWI_bm; // Don't need TWI on Port C -- we just bit-bang it
  PR.PRPD  = PR_TC0_bm | PR_TC1_bm | PR_HIRES_bm | PR_USART0_bm | PR_USART1_bm | PR_TWI_bm;
  PR.PRPE  = PR_TC0_bm | PR_HIRES_bm | PR_TWI_bm
#if WITH_SIO==0
            | PR_USART0_bm
#endif
    ;

  // Set all I/O pins to be inputs with pulldowns
	set_all_inputs();

  // Port E pin 3 has to be an output since it is the TXD line for USARTE0
#if WITH_SIO==1
  sio_tx_enable_E0(0); // Should turn serial port transmitter OFF to not interfere with bootloading
#endif

  // Initialize the timer.
  TimerInit();

  // Enable interrupts, thus enable serial communication (if interrupt-driven)
  // Enable low-level interrupts, as timer is low-level.
  // Enable mid-level interrupts, as SIO is medium-level
  // Enable high-level interrupts as SPI and audio are high-level. Also ensure interrupt vector
  // table is in the application area.
  // Enable round-robin interrupts for low-priority interrupts (doesn't apply since we
  // only have 1 low-priority interrupt, but it can't hurt).
  CCP = CCP_IOREG_gc; // Enable configuration change to protected I/O registers
  PMIC.CTRL = PMIC_RREN_bm | PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
  sei();

#if 0 // Should already be true due to bootloader
  // Switch to 32 MHz. Also initializes the serial port for 19200bps operation (set by BAUD_RATE in config.h).
  // Already done by bootloader
  setClockSource(CLKSRC_32MHZ_INT);

  // Enable DFLL to improve clock accuracy
  DFLLRC32M.CTRL = 1;
#endif

  // Enable SPI slave interface to Arduino
#if WITH_SPI==1
  SPI_C_Init();
#endif

  // Enable Serial interface to Arduino
#if WITH_SIO==1
  sio_init_E0(BAUD_RATE);
#endif

  // Set up ADC to prepare for sampling analog inputs. Set up DAC for playing audio.
  adc_init();
  dac_init();
  buffers_init();
  I2C_init();

#if 0
  sio_tx_enable_E0(1);
  printf("<AD210>");
  sio_tx_enable_E0(0);
#endif

#if 0 // Now done in ff.c in sd_init(), on demand
  if (0==disk_initialize(0)) {
    if (FR_OK == f_mount(0, &FSObject)) {
      // OK
      fail_nofail();
      //fail(FAIL_MOUNT, FAIL_MOUNT_SUCCESS);
    } else fail(FAIL_MOUNT, FAIL_MOUNT_NO_MOUNT);
  } else fail(FAIL_MOUNT, FAIL_MOUNT_NO_INIT);
#endif
  
  // Mute headphones until specifically enabled
  I2C_shutdown_enable(1);
  I2C_maxgain_enable(0);
  I2C_bassmax_enable(1);

	while (1) {
#if WITH_SPI==1
    SPI_C_Update();
#endif

#if WITH_SIO==1
    ;
#endif    

    // Check for flushing buffers
    switch (gState) {
      case STATE_RECORDING_TO_SD:
        rec_flush_buffer();
        break;

      case STATE_PLAYING_FROM_SD:
        play_fill_buffer();
        break;

      case STATE_PLAYING_FROM_SPI:
        break;

      default:
        break;
    }
	}
}
// vim: cindent ts=2 sw=2 ai expandtab
