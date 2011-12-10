#include "avr-redeye.h"

#include <avr/io.h>
#include <avr/interrupt.h>

/* HP Redeye Protocol implemented on a AVR Mega328 using
   8-bit Timer/Counter 0 in FAST PWM-mode.

   == Redeye Protocol ========================================================

   The HP-Redeye protocol uses frames consisting of a
   start-sequence, 4 parity and 8 databits. These bits and
   start-sequences consist of bursts of 7 cycles of a 32768Hz IR signal:

          7 / (2^15 Hz) = 213.6 us
     |<--------------------------------------->|
      __    __    __    __    __    __    __       <- IR led on
     |  |  |  |  |  |  |  |  |  |  |  |  |  |
   __|  |__|  |__|  |__|  |__|  |__|  |__|  |___   <- IR led off

   Let's denote these bursts by "::" and a 213.6us idle period by __,
   then the three constituents for the protocol are:

   <start> = ::__::__::__  (total 1.28 ms)
   <one>   = ::______      (total 854 us)
   <zero>  = ____::__      (total 854 us)

   And one frame consists of <start> 12*(<one>|<zero>), the order for
   the 12 payload bits in a frame is:

      d c b a 7 6 5 4 3 2 1 0

   7..0 are data, a,b,c,d are parity bits, so that, if the frame payload
   is written as a 12-bit integer the following subsets of bits must have
   a even numbers of bits set:

   +---+---+---+---++---+---+---+---++---+---+---+---++--------+
   : d : c : b : a :: 7 : 6 : 5 : 4 :: 3 : 2 : 1 : 0 ::   Hex  :
   +---+---+---+---++---+---+---+---++---+---+---+---++--------+
   : X :   :   :   ::   : X : X : X :: X :   :   :   :: 0x0878 :
   +---+---+---+---++---+---+---+---++---+---+---+---++--------+
   :   : X :   :   :: X : X : X :   ::   : X : X :   :: 0x04e6 :
   +---+---+---+---++---+---+---+---++---+---+---+---++--------+
   :   :   : X :   :: X : X :   : X ::   : X :   : X :: 0x02d5 :
   +---+---+---+---++---+---+---+---++---+---+---+---++--------+
   :   :   :   : X :: X :   :   :   :: X :   : X : X :: 0x018b :
   +---+---+---+---++---+---+---+---++---+---+---+---++--------+
   
   == AVR implementation =====================================================

   The IR-led ought to be connected to the second PWM output of the 8-bit
   Timer/Counter 0, which is OC0B/PD5/Arduino digital output 5.

   Timer/Counter 0 is operated in Fast-PWM-Mode, with a prescaler of 8
   and a maximum value of 60: 16 MHz / 8 / 61 = 32786.8 Hz.

   Bit-timing is provided by 16-bit Timer/Counter 1 operating in CTC
   mode with a "TOP" value of 3415, so we get an interrupt every 7 pulses:
   (8*61=488 (== 32kHz), 488*7=3416).
*/


struct avr_redeye avr_redeye_data;

/* === Put data into Redeye Buffer ======================================== */

int8_t
avr_redeye_put(unsigned char c){
	volatile struct avr_redeye *d = &avr_redeye_data;

	uint8_t writep = d->writep;
	uint8_t readp = d->readp;

	d->buf[writep]=c;

	writep = (writep+1) % REDEYE_BUFSIZE;
	if(writep == readp) /* buffer is full! */
		return -1;
	d->writep = writep;
	return 0;
}

/* === Calculate Frame Payload inc. Parity Bits =========================== */

static char
parity(uint8_t byte){
	byte = byte ^ (byte >> 4); /* merge 7..4 -> 3..0 */
	byte = byte ^ (byte >> 2); /* merge 3..2 -> 1..0 */
	byte = byte ^ (byte >> 1); /* merge    1 ->    0 */
	return byte & 0x01;
}

static int16_t
calculate_frame_payload(uint8_t data){
	int16_t frame = data;
	if(parity(data & 0x78)) frame |= 0x800;
	if(parity(data & 0xe6)) frame |= 0x400;
	if(parity(data & 0xd5)) frame |= 0x200;
	if(parity(data & 0x8b)) frame |= 0x100;
	return frame;
}

/* === Interrupt Routine ================================================== */

/*
 * There's the global avr_redeye_data.state variable that holds our state,
 * i.e. the next thing we have to do in our interrout routine.
 *
 * State Description
 *
 *  0    Idle, we are waiting for the user to put something into our buffer.
 *       -> Either stays at 0, or computes next frame and goes to #
 *
 * >= 54 turn off LED (intra frame spacing!)
 *
 * from 53..48 : send start frame
 *   2*n+49 turn on LED unconditionally
 *   2*n+48 turn off LED
 *
 * from 47..0 : send frame payload
 *   4*n+3  turn on LED if one-bit   ( d->frame & (1<<n) == 1)
 *   4*n+2  turn off LED
 *   4*n+1  turn on LED if zero-bit  ( d->frame & (1<<n) == 0)
 *   4*n+0  turn off LED
 */

static void
turn_led_on(){
	/* Timer/Counter 0 in mode #7: Fast PWM, TOP=OCR0A */
	/* set OC0B on compare, clear OC0B on TOP */
	TCCR0A = _BV(COM0B1)|_BV(COM0B0)|_BV(WGM01)|_BV(WGM00);
	TCNT0 = 0;
	TCCR0B = _BV(CS01)|_BV(WGM02); /* clkIO / 8 */
}

static void
turn_led_off(){
	TCCR0A = 0;
	TCCR0B = 0;
	PORTD &= ~_BV(5);			\
}

ISR(TIMER1_OVF_vect){
	struct avr_redeye *d = &avr_redeye_data;
	uint8_t bitnum;
	uint8_t bitval;

	if(d->state == 0){ /* check for new data? */
		PORTB &= ~_BV(5); /* turn off arduino LED */
		turn_led_off();
		if(d->readp == d->writep)
			return;

		d->frame = calculate_frame_payload(d->buf[d->readp]);
		d->readp = (d->readp + 1) % REDEYE_BUFSIZE;
		d->state = 60;
		PORTB |= _BV(5); /* turn on arduino LED */
		return;
	}

	/* intra frame spacing (>=54) or every 2nd burst */
	if(d->state >= 54 || !(d->state & 1)){
		turn_led_off();
		d->state--;
		return;
	}

	if(d->state >= 48){ /* <start> */
		turn_led_on();
		d->state--;
		return;
	}

	bitnum = d->state/4;
	bitval = !!(d->frame & (1<<bitnum));
	if(((d->state % 4) == 3) && bitval) /* step 4*n+3: turn on for 1 bit */
		turn_led_on();
	if(((d->state % 4) == 1) && !bitval) /* step 4*n+1: turn on for 0 bit */
		turn_led_on();
	d->state--;
}

/* === Initialization ===================================================== */
void
avr_redeye_init(){

	/* PortD5 = LED output */
	DDRD |= _BV(5);

	/* TCCR0A, TCCR0B set in turn_led_on() / _off() */
	TIMSK0 = 0; /* no interrupts */
	OCR0A  = 60; /* = TOP */
	OCR0B  = 30; /* ~50% duty cycle */

	//	TURN_ON_LED();

	/* Timer/Counter 1 in CTC mode #14 without prescaler, CTC="Top"=3415 */
	/* clkIO without prescaler = 16 MHz */
	TCCR1A = _BV(WGM11);
	TCCR1B = _BV(CS10)|_BV(WGM12)|_BV(WGM13);
	TCCR1C = 0;
	TIMSK1 = _BV(TOIE1);
	ICR1 = 3415;

}

