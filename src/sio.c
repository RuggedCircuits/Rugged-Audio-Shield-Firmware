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
 * Serial driver
 */
#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/atomic.h>
#include "config.h"
#include "sio.h"

#if WITH_SIO==1
/*******************************************************************
 *
 * This section contains parameters that may be modified by the
 * programmer.
 *
 *******************************************************************/

/* Indicate which USART this module should apply to. The suffix _C0, _C1, _E0, etc.
 * will be added to all function names. This module can be instantiated multiple
 * times (with different filenames of course) as long as each instantiation uses
 * a different USART module.
 *
 * SELECT ONLY ONE OF THE CHOICES BELOW!
 */
//#define __C0 // Use USART0 on Port C
//#define __C1 // Use USART1 on Port C
//#define __D0 // Use USART0 on Port D
//#define __D1 // Use USART1 on Port D
#define __E0 // Use USART0 on Port E

/*******************************************************************
 *
 * This is the end of the configuration parameters. You shouldn't
 * have to change anything below this point.
 *
 *******************************************************************/


#if   defined(__C0)
#  define BASE (&USARTC0)
#  define isinput   isinput_C0
#  define input     input_C0
#  define inchar    inchar_C0
#  define output    output_C0
#  define outstr    outstr_C0
#  define outstr_P  outstr_P_C0
#  define outshort  outshort_C0
#  define sio_init  sio_init_C0
#  define sio_flush sio_flush_C0
#  define sio_tx_enable sio_tx_enable_C0
#elif defined(__C1)
#  define BASE (&USARTC1)
#  define isinput   isinput_C1
#  define input     input_C1
#  define inchar    inchar_C1
#  define output    output_C1
#  define outstr    outstr_C1
#  define outstr_P  outstr_P_C1
#  define outshort  outshort_C1
#  define sio_init  sio_init_C1
#  define sio_flush sio_flush_C1
#  define sio_tx_enable sio_tx_enable_C1
#elif defined(__D0)
#  define BASE (&USARTD0)
#  define isinput   isinput_D0
#  define input     input_D0
#  define inchar    inchar_D0
#  define output    output_D0
#  define outstr    outstr_D0
#  define outstr_P  outstr_P_D0
#  define outshort  outshort_D0
#  define sio_init  sio_init_D0
#  define sio_flush sio_flush_D0
#  define sio_tx_enable sio_tx_enable_D0
#elif defined(__D1)
#  define BASE (&USARTD1)
#  define isinput   isinput_D1
#  define input     input_D1
#  define inchar    inchar_D1
#  define output    output_D1
#  define outstr    outstr_D1
#  define outstr_P  outstr_P_D1
#  define outshort  outshort_D1
#  define sio_init  sio_init_D1
#  define sio_flush sio_flush_D1
#  define sio_tx_enable sio_tx_enable_D1
#elif defined(__E0)
#  define BASE (&USARTE0)
#  define isinput   isinput_E0
#  define input     input_E0
#  define inchar    inchar_E0
#  define output    output_E0
#  define outstr    outstr_E0
#  define outstr_P  outstr_P_E0
#  define outshort  outshort_E0
#  define sio_init  sio_init_E0
#  define sio_flush sio_flush_E0
#  define sio_tx_enable sio_tx_enable_E0
#endif

static uint8_t _is_tx_enabled(void) {
    return BASE->CTRLB & USART_TXEN_bm;
}

static uint8_t *_rx_tail ;
static uint8_t * volatile _rx_head ;
static uint8_t _rx_buf_start[SIO_RX_BUFSIZE];
static uint8_t volatile _rx_flags ;
static uint16_t volatile _rx_chars ;
#define _rx_buf_limit (_rx_buf_start+sizeof(_rx_buf_start))

static uint8_t * volatile _tx_tail ;
static uint8_t *_tx_head ;
static uint8_t _tx_buf_start[SIO_TX_BUFSIZE];
//static uint8_t volatile _tx_flags ;
#define _tx_buf_limit (_tx_buf_start+sizeof(_tx_buf_start))

// Flags for _tx_flags
// None yet

// Flags for _rx_flags
#define RX_FLAG_SUSPENDED  0x01  // XOFF has been sent to host
#define RX_FLAG_OVERRUN    0x02  // Software overrun (rather than USART overrun)
#define RX_FLAG_FE         (USART_FERR_bm)   // framing error, same as FERR bit in AVR (0x10)
#define RX_FLAG_OR         (USART_BUFOVF_bm) // hardware overrun, same as BUFOVF bit in AVR (0x08)

static void sio_handle_rx(void)
{
  uint8_t scsr = BASE->STATUS & (USART_FERR_bm|USART_BUFOVF_bm);

  *_rx_head = BASE->DATA;
  _rx_flags |= scsr;

  if (scsr) {
    // Ignore data if we have overrun or framing error.
    return;
  }

  ++_rx_chars;
      
  // Finally, update head pointer to indicate received character
  if (++_rx_head >= _rx_buf_limit) {
    _rx_head = _rx_buf_start;
  }
  if (_rx_head == _rx_tail) {
    _rx_flags |= RX_FLAG_OVERRUN;
  }
}

static void sio_handle_tx(void)
{
  // Able to transmit, and is transmit interrupt enabled?
  // Why is this commented out?
  //if (! ( (UCSRA & _BV(UDRE)) && (UCSRB & _BV(UDRIE)) ) ) return;

  if ((_tx_tail != _tx_head) 
      ) {
    BASE->DATA = *_tx_tail++;
    if (_tx_tail >= _tx_buf_limit) {
        _tx_tail = _tx_buf_start;
    }
  } else {
    /* No more characters to transmit. Disable interrupt. */
      BASE->CTRLA &= ~USART_TXCINTLVL_gm;
  }
}

/* ISR(xxx) runs with interrupts disabled */
/* Interrupt handler for data reception. */
#if   defined(__C0)
ISR(USARTC0_RXC_vect)
#elif defined(__C1)
ISR(USARTC1_RXC_vect)
#elif defined(__D0)
ISR(USARTD0_RXC_vect)
#elif defined(__D1)
ISR(USARTD1_RXC_vect)
#elif defined(__E0)
ISR(USARTE0_RXC_vect)
#endif
{
  sio_handle_rx();
}
      
/* ISR(xxx) runs with interrupts disabled */
/* Interrupt handler for data transmission. */
#if   defined(__C0)
ISR(USARTC0_DRE_vect)
#elif defined(__C1)
ISR(USARTC1_DRE_vect)
#elif defined(__D0)
ISR(USARTD0_DRE_vect)
#elif defined(__D1)
ISR(USARTD1_DRE_vect)
#elif defined(__E0)
ISR(USARTE0_DRE_vect)
#endif
{
  sio_handle_tx();
}

uint8_t isinput(void)
{
  return ! (_rx_tail == _rx_head);
}

int16_t input(void)
{
  uint8_t c;

  /* Detect and clear overrun condition */
  if (_rx_flags & (RX_FLAG_OR|RX_FLAG_FE|RX_FLAG_OVERRUN)) { 
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
      _rx_flags &= ~(RX_FLAG_OR|RX_FLAG_FE|RX_FLAG_OVERRUN);
      _rx_tail = _rx_head;
      _rx_chars = 0;
    }
  }

  wdt_reset();

  if (_rx_tail == _rx_head) return -1;

  c = *_rx_tail++;

  ATOMIC_BLOCK(ATOMIC_FORCEON) {
    _rx_chars--;
  }

  if (_rx_tail >= _rx_buf_limit) {
    _rx_tail = _rx_buf_start;
  }

  return (int16_t)((uint16_t)c);
}

/*
 * This function waits until a character is received, then returns that
 * character.
 */
uint8_t inchar(void)
{
  int16_t c;

  while (((c=input()) == -1) 
      ) wdt_reset() ;

  return (uint8_t)c;
}

/*
 * This function writes a single character to the serial port.
 */
void output(uint8_t c)
{
  uint8_t *newHead = _tx_head+1;

  if (! _is_tx_enabled()) return;

  if (newHead >= _tx_buf_limit) {
    newHead = _tx_buf_start;
  }

  // Block if buffer is now full
  *_tx_head = c;
  do {
    wdt_reset(); // Reset the computer-operating-properly watchdog
  } while (newHead == _tx_tail);

  _tx_head = newHead;

  BASE->CTRLA |= USART_TXCINTLVL_MED_gc; // Enable medium priority TX interrupt
}

/*
 * This function writes a string to the serial port.
 */
void outstr(const char *s)
{
  while (*s) output(*s++);
}

/*
 * This function writes a constant string from the TEXT section to
 * the serial port.
 */
void outstr_P(PGM_P s)
{
  uint8_t c;

  while ((c=pgm_read_byte(s)) != 0) {
    output(c);
    s++;
  }
}

/*
 * This function writes a 16-bit integer LSB first
 */
#if 0
void outshort(uint16_t val)
{
  output((uint8_t) val);
  output((uint8_t) (val>>8));
}
#endif

/*
 * This function initializes the serial port to the given baud rate assuming 32 MHz operation
 */

void sio_init(BaudRate_t baudrate)
{
  static struct {
    uint8_t clk2x;
    uint8_t baudctrla;
    uint8_t baudctrlb;
  } baudRegs[] = {
    { 0, 0x40, 0x03 },    // 2400 bps   (0.0% error)
    { 0, 0x9F, 0x01 },    // 4800 bps   (0.0% error)
    { 0, 0xCF, 0x00 },    // 9600 bps   (0.2% error)
    { 1, 0x14, 0x01 },    // 14400 bps  (0.3% error)
    { 0, 0x67, 0x00 },    // 19200 bps  (0.2% error)
    { 0, 0x44, 0x00 },    // 28800 bps  (0.6% error)
    { 0, 0x33, 0x00 },    // 38400 bps  (0.2% error)
    { 1, 0x44, 0x00 },    // 57600 bps  (0.6% error)
    { 0, 0x10, 0x00 },    // 115200 bps (2.1% error! Actual baud rate is 117647 bps)
    { 0, 0x0F, 0x00 },    // 125000 bps (0% error)
    { 1, 0x10, 0x00 },    // 230400 bps (2.1% error! Actual baud rate is 235294 bps)
    { 0, 0x07, 0x00 }     // 250000 bps (0% error)
  };

  // Turn off UART while configuring it
  BASE->CTRLA = BASE->CTRLB = 0;

  // Select asynchronous mode, no parity, 1 stop bit, 8-bit data words
  BASE->CTRLC = 3; // 8-bit words

  // Configure baud rate based on our best parameters obtained above
  BASE->BAUDCTRLA = baudRegs[baudrate].baudctrla;
  BASE->BAUDCTRLB = baudRegs[baudrate].baudctrlb;

  // Set CLK2X bit if so determined
  if (baudRegs[baudrate].clk2x) BASE->CTRLB = USART_CLK2X_bm;

  _rx_head = _rx_tail = _rx_buf_start;
  _rx_flags = 0;

  _tx_head = _tx_tail = _tx_buf_start;
  //_tx_flags = 0;

  _rx_chars = 0; // Number of characters in receive buffer

  // Enable receiver and transmitter
  BASE->CTRLB |= USART_RXEN_bm|USART_TXEN_bm;

  BASE->CTRLA |= USART_RXCINTLVL_MED_gc; /* Don't enable TX interrupt until characters are written */
}

/*
 * This function waits for the transmitter to be empty before quitting.
 */

void sio_flush(void)
{
  // Don't try to flush a disabled transmitter
  if (! (BASE->CTRLB & USART_TXEN_bm)) return;

  while (! (BASE->STATUS & USART_DREIF_bm)) wdt_reset();
}

/*
   This function enables or disables the transmitter (and also configures the direction of the Tx
   output pin) part of the serial port.
*/
void sio_tx_enable(uint8_t enable)
{
  if (enable) {
    BASE->CTRLB |= USART_TXEN_bm;
#if defined(__E0)
    PORTE.DIRSET = (1<<3);   // E0 serial port Tx line is PE3
#elif defined(__D1)
    PORTD.DIRSET = (1<<7);   // D1 serial port Tx line is PD7
#elif defined(__D0)
    PORTD.DIRSET = (1<<3);   // D0 serial port Tx line is PD3
#elif defined(__C1)
    PORTC.DIRSET = (1<<7);   // C1 serial port Tx line is PC7
#elif defined(__C0)
    PORTC.DIRSET = (1<<3);   // C0 serial port Tx line is PC3
#endif
  } else {
    BASE->CTRLB &= ~USART_TXEN_bm;
#if defined(__E0)
    PORTE.DIRCLR = (1<<3);   // E0 serial port Tx line is PE3
#elif defined(__D1)
    PORTD.DIRCLR = (1<<7);   // D1 serial port Tx line is PD7
#elif defined(__D0)
    PORTD.DIRCLR = (1<<3);   // D0 serial port Tx line is PD3
#elif defined(__C1)
    PORTC.DIRCLR = (1<<7);   // C1 serial port Tx line is PC7
#elif defined(__C0)
    PORTC.DIRCLR = (1<<3);   // C0 serial port Tx line is PC3
#endif
  }
}
#endif // WITH_SIO
// vim: expandtab ts=2 sw=2 ai cindent
