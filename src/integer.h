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

   Copyright (C) 2011, ChaN, all right reserved.
 
  * The FatFs module is a free software and there is NO WARRANTY.
  * No restriction on use. You can use, modify and redistribute it for
    personal, non-profit or commercial product UNDER YOUR RESPONSIBILITY.
  * Redistributions of source code must retain the above copyright notice.

*/

/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/

#ifndef _INTEGER
#define _INTEGER

#ifdef _WIN32	/* FatFs development platform */

#include <windows.h>
#include <tchar.h>

#else			/* Embedded platform */

/* These types must be 16-bit, 32-bit or larger integer */
#include <inttypes.h>

//typedef int16_t	 INT; // Doesn't seem to be used
typedef uint16_t UINT;

/* These types must be 8-bit integer */
typedef int8_t  CHAR;
typedef uint8_t UCHAR;
typedef uint8_t BYTE;

/* These types must be 16-bit integer */
typedef int16_t	 SHORT;
typedef uint16_t USHORT;
typedef uint16_t WORD;
typedef uint16_t WCHAR;

/* These types must be 32-bit integer */
typedef int32_t	 LONG;
typedef uint32_t ULONG;
typedef uint32_t DWORD;

#endif

#endif
