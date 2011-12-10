#ifndef SERIAL_UTIL_H
#define SERIAL_UTIL_H

#include <stdint.h>

extern void serial_putchar(unsigned char c);
extern void serial_putnibble(uint8_t v);
extern void serial_put_uint8_t(uint8_t v);
extern void serial_put_uint16_t(uint16_t v);
extern void serial_put_uint24_t(uint32_t v);
extern void serial_put_uint32_t(uint32_t v);

#endif
