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
 * Handle interface back to bootloader
*/

#include <string.h>
#include <avr/io.h>
#include <util/crc16.h>
#include "config.h"
#include "bootloader.h"

// This whole mechanism must be kept in sync with the bootloader
typedef enum {
  BOOT_CMD_NO_COMMAND,
  BOOT_CMD_PROGRAM_FILE,      // Reprogram the application with filename in data[]

  BOOT_CMD_NUM_COMMANDS
} bootloadCommandCode;

typedef struct {
  bootloadCommandCode command;
  uint8_t data[13];
  uint16_t crc;
} bootloadCommandStruct;

static bootloadCommandStruct bootloadCommandArea __attribute__((section(".bss.low"))); // Section name MUST start with .bss

void bootloader_softreset(void)
{
  CCP = CCP_IOREG_gc; // Enable configuration change to protected I/O registers
  RST.CTRL = RST_SWRST_bm; // this causes a reset...no further code execution happens
}

static void _zero_cmd(void)
{
  memset(&bootloadCommandArea, 0, sizeof(bootloadCommandArea));
}

static void _compute_crc(void)
{
  uint8_t i;
  uint16_t crc=0xFFFFU;
  const uint8_t * p = (uint8_t *)&bootloadCommandArea;

  for (i=0; i < 14 /*first 14 bytes of 16-byte command area*/; i++, p++) {
    crc = _crc_ccitt_update(crc, *p);
  }

  bootloadCommandArea.crc = crc;
}

void bootloader_reprogram(const uint8_t *fname)
{
  _zero_cmd();
  bootloadCommandArea.command = BOOT_CMD_PROGRAM_FILE;
  strcpy((void *)&bootloadCommandArea.data, (const char *)fname);
  _compute_crc();
  bootloader_softreset();
  // does not return
}

// vim: expandtab ts=2 sw=2 ai cindent
