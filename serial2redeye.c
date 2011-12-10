#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "avr-redeye.h"
#include "serial_util.h"

const char hello[] PROGMEM =\
	"This is serial2redeye.\r\n"
	"It sends received data to a HP82240A via Infrared.\r\n"
	"It doesn't care about flow control, be sure not to overflow\r\n"
	"either the AVR or the HP82240A buffer.";

int
main(){
	int i;

	/* -------------- serial port --------------------- */
	UCSR0B = _BV(RXEN0)|_BV(TXEN0);
	UCSR0C = _BV(UCSZ01)|_BV(UCSZ00);
	// UBRR0 = 103; /* 9k6 bps @ 16 MHz (U2X0=0) */
	// UCSR0A = 0;

	UBRR0 = 16; /* 115k2 bps @ 16 MHz (U2X0=1) */
	UCSR0A = _BV(U2X0);

	for(i=0;i<sizeof(hello);i++)
		serial_putchar(pgm_read_byte(&hello[i]));

	avr_redeye_init();
	sei();

	while(1){
		if(UCSR0A & _BV(RXC0)){ /* received a byte */
			avr_redeye_put(UDR0);
		}
	}
}
