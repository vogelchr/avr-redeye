#include <avr/io.h>
#include <avr/interrupt.h>

#include "avr-redeye.h"
#include "serial_util.h"

void
main(){
	/* -------------- serial port --------------------- */
	UCSR0B = _BV(RXEN0)|_BV(TXEN0);
	UCSR0C = _BV(UCSZ01)|_BV(UCSZ00);
#if 0 /* 1: 9600, 0: 115200 */
	UBRR0 = 103; /* 9k6 bps @ 16 MHz (U2X0=0) */
#else
	UBRR0 = 16; /* 115k2 bps @ 16 MHz (U2X0=1) */
	UCSR0A = _BV(U2X0);
#endif
       	UDR0 = '*'; /* say hello */

	DDRB |= _BV(5);  /* turn on LED */
	PORTB |= _BV(5);

	DDRD |= _BV(5);
	PORTD |= _BV(5);

	avr_redeye_init();
	sei();

	while(1){
		if(UCSR0A & _BV(RXC0)){
			avr_redeye_put(UDR0);
			serial_putchar('\r');
			serial_putchar('\n');
		}
		if((UCSR0A & _BV(UDRE0))){
			if(avr_redeye_data.state != 0){
				serial_putchar('<');
				serial_put_uint8_t(avr_redeye_data.state);
				serial_putchar(':');
				serial_put_uint16_t(avr_redeye_data.frame);
				serial_putchar('>');
			}
		}
	}
}
