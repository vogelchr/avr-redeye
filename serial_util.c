/*
 * HP Redeye Protocol implemented on a AVR Mega328 using
 *    8-bit Timer/Counter 0 in FAST PWM-mode.
 * Copyright (C) 2011 Christian Vogel <vogelchr@vogel.cx>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "serial_util.h"

#include <avr/io.h>

void
serial_putchar(unsigned char c){
	while(!(UCSR0A & _BV(UDRE0)));
	UDR0 = c;
}

void
serial_putnibble(uint8_t n){
	n &= 0x0f;
	n += '0';
	if( n > '9')
		n+= ('a'-'9'-1);
	serial_putchar(n);
}

void
serial_put_uint8_t(uint8_t v){
	serial_putnibble(v >> 4);
	serial_putnibble(v & 0x0f);
}

void
serial_put_uint16_t(uint16_t v){
	serial_putnibble(v >> 12);
	serial_putnibble(v >> 8);
	serial_putnibble(v >> 4);
	serial_putnibble(v);
}

void
serial_put_uint24_t(uint32_t v){
	serial_putnibble(v >> 20);
	serial_putnibble(v >> 16);
	serial_putnibble(v >> 12);
	serial_putnibble(v >>  8);
	serial_putnibble(v >>  4);
	serial_putnibble(v);
}

void
serial_put_uint32_t(uint32_t v){
	serial_putnibble(v >> 28);
	serial_putnibble(v >> 24);
	serial_putnibble(v >> 20);
	serial_putnibble(v >> 16);
	serial_putnibble(v >> 12);
	serial_putnibble(v >>  8);
	serial_putnibble(v >>  4);
	serial_putnibble(v);
}


