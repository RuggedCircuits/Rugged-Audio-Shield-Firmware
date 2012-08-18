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
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "printf.h"
#include "sio.h"

// Change for particular serial port being used
#define outstr outstr_E0
#define outstr_P outstr_P_E0

#if WITH_DEBUG==1
void NL(void) { outstr_P(PSTR("\r\n")); }

// Uncomment to use signaling assert
//#define WITH_ASSERT

/////////////////////////////////////////////////////

#ifdef WITH_ASSERT
#  include "assert.h"
#endif

#ifdef TEST
void output(unsigned char c)
{
  putchar(c);
}
void outstr(const char *s)
{
  while (*s) output(*s++);
}
void _abort(assertCode_t code)
{
  exit(code);
}
#endif

#define PRINTF_BUF_SIZE 128

static char _buf[PRINTF_BUF_SIZE+1];

#if 0 // Use built-in to string.h -- it's smaller
char * strrev(char *s)
{
  unsigned i,j;

  j = 0;
  i = strlen((char *)s)-1;
  while (j < i) {
    unsigned char c;

    c = s[i];
    s[i] = s[j];
    s[j] = c;

    j++;
    i--;
  }
  return s;
}
#endif

static int padWidth = 0;
static unsigned char padChar = ' ';

static void _uprint(char **buf, unsigned *maxsize, unsigned val, unsigned base)
{
  char tmp[33];
  int i;

  memset(tmp, 0, sizeof(tmp));

  if (val==0) {
    tmp[0]='0';
  } else {
    for (i=0; val && (i < sizeof(tmp)-1); i++, val /= base) {
      tmp[i] = '0' + (val % base);
      if (tmp[i] > '9') tmp[i] += ('A'-'9'-1);
    }
    strrev(tmp);
  }

  i = strlen((char *)tmp);
  if (i < padWidth) {
    if (padWidth > *maxsize) padWidth = *maxsize;
    if (i < padWidth) {
      padWidth -= i;
      memset((char *)(*buf), padChar, padWidth);
      *buf += padWidth;
      *maxsize -= padWidth;
    }
  }

  if (i > *maxsize) i = *maxsize;

  strncpy((char *)(*buf), (char *)tmp, i);
  *maxsize -= i;
  *buf += i;

  padChar = ' ';
  padWidth = 0;
}

void vsnprintf(char *buf, unsigned maxsize, const char *fmt, va_list ap)
{
  unsigned char c=0;
  unsigned char f;
  int i;

  if (maxsize==0) return;
  --maxsize; // Allow room for terminating null

  while (maxsize && *fmt) {
    if (c=='%') {
      c=0;
      switch ( (f = *fmt++) ) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          if (f=='0' && padWidth==0) {
            padChar = '0';
          } else {
            padWidth = padWidth*10 + (f-'0');
          }
          c = '%';
          break;

        case '%':
          *buf++ = '%'; maxsize--; break;

        case 'u':
          _uprint(&buf, &maxsize, va_arg(ap, unsigned), 10);
          break;

        case 'X': case 'x':
          _uprint(&buf, &maxsize, va_arg(ap, unsigned), 16);
          break;

        case 'd':
          {
            int val;

            val = va_arg(ap, int);
            if (val < 0) { *buf++ = '-'; maxsize--; val = -val; } // Doesn't work for MIN_INT
            _uprint(&buf, &maxsize, val, 10);
          }
          break;

        case 'c':
          {
            int val;

            val = va_arg(ap, int);
            if (maxsize) {
              *buf++ = val;
              maxsize--;
            }
          }
          break;

        case 's':
          {
            const char *s;

            s = va_arg(ap, const char *);
            i = strlen(s);
            if (i < padWidth) {
              padWidth -= i;
              while (maxsize && padWidth) {
                *buf++ = ' ';
                padWidth--;
                maxsize--;
              }
            }
            while (maxsize && *s) {
              *buf++ = *s++;
              maxsize--;
            }
            padWidth=0;
          }
          break;

        default:
          break;
      }
    } else {
      c = (unsigned char) (*fmt++);
      if (c!='%') {
        *buf++ = c;
        maxsize--;
      }
    }
  }

  *buf = 0;
}

void snprintf(char *buf, unsigned maxsize, const char *fmt, ...)
{
  va_list ap;

#ifdef WITH_ASSERT
  assert(buf, ASSERT_SNPRINTF_BUF);
  assert(fmt, ASSERT_SNPRINTF_FMT);
#endif

  va_start(ap, fmt);
  vsnprintf(buf, maxsize, fmt, ap);
  va_end(ap);
}

void vprintf(const char *fmt, va_list ap)
{
#ifdef WITH_ASSERT
  assert(fmt, ASSERT_VPRINTF_FMT);
#endif

  vsnprintf(_buf, PRINTF_BUF_SIZE, fmt, ap);
  outstr((char *)_buf);
}

void printf(const char *fmt, ...)
{
  va_list ap;

#ifdef WITH_ASSERT
  assert(fmt, ASSERT_PRINTF_FMT);
#endif

  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
}

#endif // WITH_DEBUG
// vim: expandtab sw=2 ts=2 ai cindent
