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

/* All SD card files are in the bootloader area...see ff.h.  Here we have a top-level function
   for initializing both the MMC and SD card drivers. */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "config.h"
#include "fail.h"
#include "diskio.h"
#include "ff.h"

static FATFS FSObject;

void sd_init(uint8_t prescale)
{
  if (0==disk_initialize(0)) {
    if (FR_OK == f_mount(0, &FSObject)) {
      // OK
      fail_nofail();
      //fail(FAIL_MOUNT, FAIL_MOUNT_SUCCESS);

      // Take a number from 0 to 7 and map it to bit 7, plus bits 2-1 so we
      // implement CLK2X and PRESCALER[1:0] bits.
      if (prescale & _BV(2)) prescale |= (uint8_t)0x80U;
      prescale &= 0x3;
      SPID.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | prescale;

    } else fail(FAIL_MOUNT, FAIL_MOUNT_NO_MOUNT);
  } else fail(FAIL_MOUNT, FAIL_MOUNT_NO_INIT);
}
// vim: expandtab ts=2 sw=2 ai cindent
