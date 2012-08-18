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
#ifndef _PRINTF_H_
#define _PRINTF_H_

#include <stdarg.h>
#include "config.h"

#if WITH_DEBUG==1
extern void xvsnprintf(char *buf, unsigned maxsize, const char *fmt, va_list ap);
extern void xsnprintf(char *buf, unsigned maxsize, const char *fmt, ...);
extern void xprintf(const char *fmt, ...);
extern void xvprintf(const char *fmt, va_list ap);
extern void NL(void);
#else
static inline void xvsnprintf(char *buf, unsigned maxsize, const char *fmt, va_list ap) { (void)buf; (void) maxsize; (void) ap; }
static inline void xsnprintf(char *buf, unsigned maxsize, const char *fmt, ...) { (void)buf; (void)maxsize; (void)fmt; }
static inline void xprintf(const char *fmt, ...) { (void)fmt; }
static inline void xvprintf(const char *fmt, va_list ap) { (void)fmt; (void)ap; }
static inline void NL(void) {}
#endif

#define printf xprintf
#define vprintf xvprintf
#define snprintf xsnprintf
#define vsnprintf xvsnprintf

#endif // _PRINTF_H_
// vim: ts=2 sw=2 ai cindent expandtab
