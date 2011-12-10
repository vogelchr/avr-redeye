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


