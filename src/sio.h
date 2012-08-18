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
#ifndef _SIO_H_
#define _SIO_H_

#include <avr/pgmspace.h>
typedef enum {
  SIO_BAUD_2400,
  SIO_BAUD_4800,
  SIO_BAUD_9600,
  SIO_BAUD_14400,
  SIO_BAUD_19200,
  SIO_BAUD_28800,
  SIO_BAUD_38400,
  SIO_BAUD_57600,
  SIO_BAUD_115200,
  SIO_BAUD_125000,
  SIO_BAUD_230400,
  SIO_BAUD_250000
} BaudRate_t;

extern uint8_t gEnableResetOnBreak;

extern void sio_init_E0(BaudRate_t baudrate);
extern uint8_t isinput_E0(void);
extern int16_t input_E0(void);
extern uint8_t inchar_E0(void);
extern void output_E0(uint8_t c);
extern void outstr_E0(const char *s);
extern void outstr_P_E0(PGM_P s);
extern void outshort_E0(uint16_t val);
extern void sio_flush_E0(void);
extern void sio_tx_enable_E0(uint8_t enable);

#endif /* _SIO_H_ */
// vim: expandtab ts=2 sw=2 ai cindent
