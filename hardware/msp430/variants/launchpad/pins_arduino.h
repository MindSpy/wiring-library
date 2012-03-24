/*
  ************************************************************************
  *	pins_arduino.h
  *
  *	Arduino core files for MSP430
  *		Copyright (c) 2012 Robert Wessels. All right reserved.
  *
  *
  ***********************************************************************
  Derived from:
  pins_arduino.h - Pin definition functions for Arduino
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2007 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA
*/

#ifndef Pins_Arduino_h
#define Pins_Arduino_h
#ifndef BV
#define BV(x) (1 << (x))
#endif


static const uint8_t A0 = 0;
static const uint8_t A1 = 1;
static const uint8_t A2 = 2;
static const uint8_t A3 = 3;
static const uint8_t A4 = 4;
static const uint8_t A5 = 5;
static const uint8_t A6 = 6;
static const uint8_t A7 = 7;
static const uint8_t A10 = 10; // special. This is the internal temp sensor

#ifdef ARDUINO_MAIN

//                      +-\/-+
//               VCC   1|    |28  GND
//         (A0)  P1.0  2|    |27  XIN
//         (A1)  P1.1  3|    |26  XOUT
//         (A2)  P1.2  4|    |25  TEST
//         (A3)  P1.3  5|    |24  RST#
//         (A4)  P1.4  6|    |23  P1.7  (A7) (SCL)
//         (A5)  P1.5  7|    |22  P1.6  (A6) (SDA)
//               P2.0  8|    |21  P2.5
//               P2.1  9|    |20  P2.4
//               P2.2 10|    |19  P2.3
//                      +----+
//

const uint16_t port_to_dir[] = {
	NOT_A_PORT,
	(uint16_t) &P1DIR,
	(uint16_t) &P2DIR,
#ifdef __MSP430_HAS_PORT3_R__
	(uint16_t) &P3DIR,
#endif
};

const uint16_t port_to_input[] = {
	NOT_A_PORT,
	(uint16_t) &P1IN,
	(uint16_t) &P2IN,
#ifdef __MSP430_HAS_PORT3_R__
	(uint16_t) &P3IN,
#endif
};
const uint16_t port_to_output[] = {
	NOT_A_PORT,
	(uint16_t) &P1OUT,
	(uint16_t) &P2OUT,
#ifdef __MSP430_HAS_PORT3_R__
	(uint16_t) &P3OUT,
#endif
};

const uint8_t digital_pin_to_port[] = {
	NOT_A_PIN, /* 0 */
	NOT_A_PIN, /* 1 */
	P1, /* 2 */
	P1, /* 3 */
	P1, /* 4 */
	P1, /* 5 */
	P1, /* 6 */
	P1, /* 7 */
	P2, /* 8 */
	P2, /* 9 */
	P2, /* 10 */
	P2, /* 11 */
	P2, /* 12 */
	P2, /* 13 */
	P1, /* 14 */
	P1, /* 15 */
};

const uint8_t digital_pin_to_timer[] = {
	NOT_A_PIN, /* 0 */
	NOT_A_PIN, /* 1 */
	P1, /* 2 */
	P1, /* 3 */
	P1, /* 4 */
	P1, /* 5 */
	P1, /* 6 */
	P1, /* 7 */
	P2, /* 8 */
	P2, /* 9 */
	P2, /* 10 */
	P2, /* 11 */
	P2, /* 12 */
	P2, /* 13 */
	P1, /* 14 */
	P1, /* 15 */
};

const uint8_t digital_pin_to_bit_mask[] = {
	NOT_A_PIN, /* 0,  pin count starts at 1 */
	NOT_A_PIN, /* 1,  VCC */
	BV(0),     /* 2,  port P1.0 */
	BV(1),     /* 3,  port P1.1 */
	BV(2),     /* 4,  port P1.2 */
	BV(3),     /* 5,  port P1.3*/
	BV(4),     /* 6,  port P1.4 */
	BV(5),     /* 7,  port P1.5 */
	BV(0),     /* 8,  port P2.0 */
	BV(1),     /* 9,  port P2.1 */
	BV(2),     /* 10, port P2.2 */
	BV(3),     /* 11, port P2.3 */
	BV(4),     /* 12, port P2.4 */
	BV(5),     /* 13, port P2.5 */
	BV(6),     /* 14, port P1.6 */
	BV(7),     /* 15, port P1.7 */
	NOT_A_PIN, /* 16, RST */
	NOT_A_PIN, /* 17, TEST */
	NOT_A_PIN, /* 18, XOUT */
	NOT_A_PIN, /* 19, XIN */
	NOT_A_PIN, /* 20, GND */
};
#endif
#endif
