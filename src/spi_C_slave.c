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
 * This module implements an SPI slave driver and handles all commands coming from the Arduino
 */

#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/atomic.h>
#include "config.h"
#include "i2c.h"
#include "version.c"
#include "sio.h"
#include "play.h"
#include "rec.h"
#include "fail.h"
#include "state.h"
#include "spi_C_slave.h"
#include "adc.h"
#include "pass.h"
#include "bootloader.h"
#include "wavwrite.h"

#if WITH_SPI==1

/*static*/ uint8_t volatile spiBuf[MAX_SPI_BUF_SIZE];
static volatile uint8_t *spiBufPtr; // Address of next location in spiBuf where incoming data will be stored
static volatile uint8_t spiBufCount; // Bytes to receive before transaction is complete
static volatile uint8_t spiExchangeUpdate; // Set to true in ISR if an entire SPI packet is done
static uint8_t spiCommand;  // Remember the command that was received while processing its data

static enum {  // Indicate what the latest SPI exchange is giving us, a 1-byte command or followup data
  SPI_IS_COMMAND,
  SPI_IS_DATA,
} spiExchangeType;

// spiBuf[] and spiBufCount must already have valid values at this point
static void _spiStartExchange(void)
{
  // Read any cruft bytes and clear interrupt flag
  if (SPIC.STATUS & SPI_IF_bm) (void)SPIC.DATA;

  // Write the first character in the buffer to the SPI
  SPIC.DATA = spiBuf[0];
  spiBufPtr = spiBuf;
}

static void _accept_command(void)
{
  spiBuf[0] = gState; //'*';
  spiBufCount = 1;
  spiExchangeType = SPI_IS_COMMAND;
  _spiStartExchange();
}

static void _accept_data(void)
{
  spiBufCount = spiBufPtr - spiBuf;
  spiExchangeType = SPI_IS_DATA;
  _spiStartExchange();
}

void SPI_C_Init(void)
{
  /*
   * Configuration as SPI slave:
   *      PC4 is !SSD input
   *      PC5 is MOSID input
   *      PC6 is MISOD output
   *      PC7 is SCKD input
   *
   * We're going to assume CPOL=CPHA=0.
   */
  PORTC.DIRCLR = _BV(4) | _BV(5) | _BV(7); // !SS, MOSI, SCK are inputs. Enable pullup on !SS.
  PORTC.DIRSET = _BV(6); // PD6 is MISO output -- will be held hi-Z by SPI module in slave mode
  // Pullup is unnecessary due to external hardware
  //PORTC.PIN4CTRL = (PORTC.PIN4CTRL & ~PORT_OPC_gm) | PORT_OPC_PULLUP_gc;

  // Enable SPI, MSB-first transfers, slave mode.
  SPIC.CTRL = SPI_ENABLE_bm;

  _accept_command();

  // Enable high-priority interrupts
  SPIC.INTCTRL = SPI_INTLVL_HI_gc;
}

static inline void _transmit_u8(uint8_t val)
{
  *spiBufPtr++ = val;
}

static void _transmit_u16(uint16_t val)
{
#if 0
  union {
    uint8_t byte[2];
    uint16_t hword;
  } val16;
  
  val16.hword = val;
  *spiBufPtr++ = (uint8_t) (val16.byte[0]);
  *spiBufPtr++ = (uint8_t) (val16.byte[1]);
#else
  memcpy((uint8_t *)spiBufPtr, &val, sizeof(val));
  spiBufPtr += sizeof(val);
#endif
}

#if 0
static void _transmit_u32(uint32_t val)
{
#if 0
  union {
    uint8_t byte[4];
    uint32_t lword;
  } val32;
  
  val32.lword = val;
  *spiBufPtr++ = (uint8_t) (val32.byte[0]);
  *spiBufPtr++ = (uint8_t) (val32.byte[1]);
  *spiBufPtr++ = (uint8_t) (val32.byte[2]);
  *spiBufPtr++ = (uint8_t) (val32.byte[3]);
#else
  memcpy(spiBufPtr, &val, sizeof(val));
  spiBufPtr += sizeof(val);
#endif
}
#endif

#if 0
static void _transmit_fill(uint8_t fillchar, uint8_t count)
{
#if 0
  while (count--) {
    *spiBufPtr++ = fillchar;
  }
#else
  memset(spiBufPtr, fillchar, count);
  spiBufPtr += count;
#endif
}
#endif

static void _transmit_empty(uint8_t count)
{
  spiBufPtr += count;
}

uint16_t _read_u16(void)
{
#if 0
  union {
    uint8_t byte[2];
    uint16_t word;
  } val16;
  val16.byte[0] = *spiBufPtr++;
  val16.byte[1] = *spiBufPtr++;
  return val16.word;
#else
  uint16_t word;

  word = *(uint16_t *)spiBufPtr;
  spiBufPtr += sizeof(word);
  return word;
#endif
}

static uint8_t inline _read_u8(void)
{
  return *spiBufPtr++;
}

/*
   Command reference:

   ! : Reboot and possibly load alternate program
   " :
   # :
   $ :
   % :
   & :
   ' :
   ( :
   ) :
   * : Synchronize SPI
   + :
   , :
   - :
   . :
   / :
   0 : Pass-through effect #0 (no effect)
   1 : Pass-through effect #1 (echo)
   2 : Pass-through effect #2 (flange)
   3 :
   4 :
   5 :
   6 :
   7 :
   8 :
   9 :
   : :
   ; :
   < :
   = :
   > :
   ? : Get current operating state
   @ : Create filesystem on SD card
   A : Set ADC line/mic gains
   B : Control headphone BassMax
   C : Stream SPI to headphones
   D : Stream SPI to headphones data packet
   E : Return last failure code (then clear failure code)
   F : Initialize the filesystem (initialize and mount)
   G : Set headphone gain boost on/off
   H : Headphone enable/disable
   I : Stream SPI from line/mic
   J : Receive stream SPI packet from line/mic
   K : Receive count of how many SPI packets are available for streaming to SPI from line/mic
   L :
   M :
   N :
   O :
   P : Play WAV file from SD card
   Q : Stop current activity and return to idle mode
   R : Record WAV file to SD card
   S : Presize file on SD card
   T : Serial Tx enable/disable
   U :
   V : Set headphone volume
   W :
   X :
   Y :
   Z : Get program version, SD card status, etc.
 */
static void _handleData(void)
{
  uint8_t line, mic;
  uint16_t Fs;
  uint8_t stereo, source;

  switch (spiCommand) {
    default:
      // We sent data, we're done. No data coming from Arduino.
      break;

    case '!':   // '!': Replace application through bootloader
      bootloader_reprogram((const uint8_t *)spiBufPtr); // Does not return

    case 'H':   // 'H': Enable/disable headphone output
      I2C_shutdown_enable(!_read_u8());
      break;

    case 'V':   // 'V': Set headphone volume
      I2C_set_volume(_read_u8());
      break;

    case 'F':   // 'F': Initialize SD card and set SPI bus frequency
      sd_init(_read_u8());
      break;

    case 'G':   // 'G': Set headphone gain boost
      I2C_maxgain_enable(_read_u8());
      break;

    case '@':   // '@': Make a new filesystem. Requires 32-bit verification code to prevent mistakes.
      fail(FAIL_MKFS, FAIL_MKFS_BAD_CODE);
      if (_read_u16() == 0x25E8) {
        if (_read_u16() == 0x9D3C) {
          if (FR_OK == f_mkfs(0,0,0)) {
            fail_nofail();
          } else {
            fail(FAIL_MKFS, FAIL_MKFS_FAILED);
          }
        }
      }
      break;

    case 'A':   // 'A': Set ADC line/mic gains
      line = _read_u8();
      mic = _read_u8();
      adc_set_gains(line, mic);
      break;

    case 'B':   // 'B': Set BassMax enable
      I2C_bassmax_enable(_read_u8());
      break;

    case 'T':   // 'T': Serial transmitter enable/disable
#if WITH_SIO==1
      sio_tx_enable_E0(_read_u8());
#endif
      break;

    case 'P':   // 'P': Play WAV file...specify 8.3 --> 13 characters including NULL
      play_wav_file((const uint8_t *)spiBuf);
      break;

    case 'C':   // 'C': Play stream from SPI...specify sampling rate and mono/stereo
    case 'I':   // 'I': Stream line/mic to SPI...specify sampling rate, mono/stereo, source
      Fs = _read_u16();
      stereo = _read_u8();
      if (spiCommand=='C') {
        play_from_SPI(Fs, stereo);
      } else {
        rec_to_SPI(Fs, stereo, _read_u8());
      }
      break;

    case '0':   // '0': Audio effects #0 through #9
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      Fs = _read_u16();
      stereo = _read_u8();
      source = _read_u8();
      pass_through(spiCommand, Fs, stereo, source);
      break;

    case 'D':   // 'D': Data packet for SPI stream to headphones
      play_SPI_add_buffer((const uint8_t *)spiBufPtr);
      break;

    case 'R':   // 'R': Record to WAV file
      Fs = _read_u16();
      stereo = _read_u8();
      source = _read_u8();
      record_wav_file(source, Fs, stereo, (const uint8_t *)spiBufPtr);
      break;

    case 'S':     // 'S': presize file on SD card. Parameter is number of MEGABYTES to presize.
      Fs = _read_u16();
      presize_wav_file((const char *)spiBufPtr, Fs);
      break;

  }
  _accept_command();
}

static void _handleCommand(void)
{
  spiCommand = spiBuf[0];

  switch (spiCommand) {
    case 'B':     // 'B': Set headphone BassMax
    case 'F':     // 'F': Initialize the filesystem
    case 'G':     // 'G': Set headphone gain boost
    case 'H':     // 'H': Enable/disable headphones
    case 'T':     // 'T': Transmitter enable/disable
    case 'V':     // 'V': Set headphone volume
      _transmit_empty(1); // Wait for transmitter on/off parameter
      _accept_data();
      break;

    case 'A':     // 'A': Set ADC gains for line and mic
      _transmit_empty(2);
      _accept_data();
      break;

    case 'C':     // 'C': Stream audio over SPI to headphones
      _transmit_empty(3); // Specify sampling rate and mono/stereo
      _accept_data();
      break;

    case '@':     // '@': Make a new filesystem and erase the SD card (4-byte verification code required)
    case 'I':     // 'I': Stream audio from line/mic to SPI
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      _transmit_empty(4); // Specify sampling rate, mono/stereo, source for effects or 'I'
      _accept_data();
      break;

    case 'P':     // 'P': Play WAV from SD
    case '!':     // '!': Replace application with given filename through bootloader
      _transmit_empty(13); // Filename in 8.3 format, zero-padded
      _accept_data();
      break;

    case 'S':     // 'S': presize file on SD card. Parameter is number of MEGABYTES to presize.
      _transmit_empty(15); // 2 bytes for how many megabytes to presize, 13 chars for filename
      _accept_data();
      break;

    case 'R':     // 'R': Record WAV from SD
      _transmit_empty(17); // Source (line or mic), mono/stereo, sampling rate, then filename in 8.3 format, zero-padded
      _accept_data();
      break;

    case 'J':     // 'J': Request SPI stream packet from line/mic
      memcpy((uint8_t *)spiBufPtr, rec_SPI_get_buffer(), SPI_STREAM_SIZE_BYTES);
      spiBufPtr += SPI_STREAM_SIZE_BYTES;
      _accept_data();
      break;

    case 'D':     // 'D': Data packet for SPI. Return how many free buffers there are.
      _transmit_u8(play_SPI_get_free_buffers()-1); // This is how many buffers are left AFTER receiving this packet
      _transmit_empty(SPI_STREAM_SIZE_BYTES-1);
      _accept_data();
      break;

    case 'E':     // 'E': Return last failure codes
      _transmit_u8(gFailMajor);
      _transmit_u8(gFailMinor);
      gFailMajor = gFailMinor = 0;
      _accept_data();
      break;


    case 'K':     // 'K': Request count of how many SPI stream packets from line/mic are available
      _transmit_u8(rec_SPI_get_full_buffers());
      _accept_data();
      break;

    case '*':     // '*': Dummy command to restore synchronization
      _transmit_u8('_'); // Indicate we're synchronized
      _accept_data();
      break;

    case 'Q':     // 'Q': Stop whatever you're doing (playing, recording, etc.)
      switch (gState) {
        case STATE_RECORDING_TO_SD:
        case STATE_RECORDING_TO_SPI:
          rec_stop();
          break;

        case STATE_PLAYING_FROM_SD:
        case STATE_PLAYING_FROM_SPI:
          play_stop();
          break;

        case STATE_PASS_THROUGH:
          pass_stop();
          break;

        default:
          break;
      }
      _accept_command();
      break;

    case '?':     // '?': Request current operating state
      _accept_command(); // First returned byte in _accept_command() is gState, so nothing to do!
      break;

    case 'Z':     // 'Z': request administrative information
      _transmit_u8(APPVERSION_MAJOR);
      _transmit_u8(APPVERSION_MINOR);
      _transmit_u16(APPVERSION_BUILD);
      _transmit_u16(bootloader_version());
      _transmit_u8(SOCKINS());  // Is there an SD card plugged in?
      _accept_data();
      break;

    default:
      // ???
      _accept_command();
      break;
  }
}

// ISR for SPI incoming data.
ISR(SPIC_INT_vect)
{
  SPIC.DATA = spiBufPtr[1];
  if (spiBufCount) { // Important! Must not store data that has not yet been _accept'ed else it could overwrite a buffer being processed (the Arduino is overrunning our ability to process)
    *spiBufPtr++ = SPIC.DATA;

    if (--spiBufCount == 0) {
      SPIC.DATA = STATE_BUSY; // In case we get '?' queries right away
      spiBufPtr = spiBuf;
      spiExchangeUpdate = 1;
    }
  } else SPIC.DATA = STATE_BUSY;
}

void SPI_C_Update(void)
{
  if (spiExchangeUpdate) {
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
      spiExchangeUpdate=0;
    }
    if (spiExchangeType == SPI_IS_COMMAND) {
      _handleCommand();
    } else {
      _handleData();
    }
  }
}

#endif // WITH_SPI

// vim: expandtab ts=2 ai sw=2 cindent
