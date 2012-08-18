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
#ifndef _CONFIG_H_
#define _CONFIG_H_

// Serial baud rate for Arduino communication. Can be changed through user commands.
// Let's start with something simple though.
#define BAUD_RATE SIO_BAUD_38400

// Define RAM buffer size for each one of the two ping-pong buffers. Total RAM usage
// will be double this number (bytes), plus any other RAM usage for the rest of the program.
// It's important that this is closely related to the number 512 for good SD card performance.
// Do not allow total .bss usage to get too close to the limit else you will get stack overflows!
#define BUFFER_SIZE 1024

// Define how many bytes we receive/transmit when streaming to/from SPI. This is the number
// of bytes associated with the 'D' command. Must have (BUFFER_SIZE/SPI_STREAM_SIZE_BYTES)<=64
// AND must have BUFFER_SIZE=k*SPI_STREAM_SIZE_BYTES where k is an integer.
#define SPI_STREAM_SIZE_BYTES 128

// We're not compiling the bootloader
#define BOOTLOADER 0

// Maximum size of packet exchanged over SPI
#define MAX_SPI_BUF_SIZE (SPI_STREAM_SIZE_BYTES+1)

// Define to 1 to enable the SPI slave interface, else set to 0
#define WITH_SPI 1

// Define to 1 to enable the Serial interface, else set to 0
#define WITH_SIO 0

/* Define the size (in bytes) of the Serial receive buffer. The bigger the buffer,
 * the less chance there is of losing incoming characters.  */
#define SIO_RX_BUFSIZE  32

// Define the size (in bytes) of the Serial transmit buffer.
#define SIO_TX_BUFSIZE  32

// Enable the right-channel DAC for stereo output (left-channel DAC is always enabled)
#define WITH_DAC_RIGHT 1

// Set to 1 to enable debugging output over the serial port
#define WITH_DEBUG 1

#endif // _CONFIG_H_
// vim: expandtab ts=2 sw=2 ai cindent
