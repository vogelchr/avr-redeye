DEVICE_CC = atmega328p
DEVICE_DUDE = m328p

# PROGRAMMER_DUDE = -Pusb -c dragon_isp
PROGRAMMER_DUDE = -P/dev/ttyUSB0 -c stk500v1 -b 57600

AVRDUDE=avrdude
OBJCOPY=avr-objcopy
OBJDUMP=avr-objdump
CC=avr-gcc
LD=avr-gcc

LDFLAGS=-Wall -g -mmcu=$(DEVICE_CC)
CPPFLAGS=
CFLAGS=-mmcu=$(DEVICE_CC) -Os -Wall -g

MYNAME=serial2redeye

OBJS=$(MYNAME).o avr-redeye.o serial_util.o

all : $(MYNAME).hex $(MYNAME).lst

$(MYNAME).bin : $(OBJS)

%.hex : %.bin
	$(OBJCOPY) -j .text -j .data -O ihex $^ $@ || (rm -f $@ ; false )

%.lst : %.bin
	$(OBJDUMP) -S $^ >$@ || (rm -f $@ ; false )

%.bin : %.o
	$(LD) $(LDFLAGS) -o $@ $^

include $(OBJS:.o=.d)

%.d : %.c
	$(CC) -o $@ -MM $^

.PHONY : clean burn
burn : $(MYNAME).hex
	$(AVRDUDE) $(PROGRAMMER_DUDE) -p $(DEVICE_DUDE) -U flash:w:$^
clean :
	rm -f *.bak *~ *.bin *.hex *.lst *.o *.d