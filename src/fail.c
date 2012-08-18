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
#include <inttypes.h>
#include "config.H"
#include "fail.h"

uint8_t gFailMajor, gFailMinor;

uint8_t fail(FailMajor_t major, FailMinor_t minor)
{
  gFailMajor = major;
  gFailMinor = minor;
  return 0;
}

uint8_t fail_major(FailMajor_t major)
{
  gFailMajor = major;
  gFailMinor = 0;
  return 0;
}

uint8_t fail_minor(FailMinor_t minor)
{
  gFailMinor = minor;
  return 0;
}

// vim: expandtab ts=2 sw=2 ai cindent
