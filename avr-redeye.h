#ifndef AVR_REDEYE_H
#define AVR_REDEYE_H

#include <stdint.h>

#define REDEYE_BUFSIZE 256

struct avr_redeye {
	/* output state */
	uint16_t frame;
	uint8_t  state;
	/* buffer: */
	uint8_t  readp;
	uint8_t  writep;
	unsigned char buf[REDEYE_BUFSIZE];
};

extern struct avr_redeye avr_redeye_data;

extern void avr_redeye_init();
extern int8_t avr_redeye_put(unsigned char c);

#endif
