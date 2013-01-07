#include "ht1632c.h"

#include <avr/io.h>
#include <stdio.h>

/* set this to the port the controller is connected to */
#define HT1632C_PORT		PORTB
#define HT1632C_DDR		DDRB
#define HT1632C_CS		_BV(3)
#define HT1632C_WRCLK		_BV(4)
#define HT1632C_DATA		_BV(5)

/*
#define BIT_SLEEP do { asm volatile ("nop;\n\tnop;\n\tnop;\n"); } while(0)
*/
#define BIT_SLEEP do { } while(0)

static void
ht1632c_start(void)
{
	BIT_SLEEP;
	HT1632C_PORT &= ~ HT1632C_CS;
}

static void
ht1632c_stop(void)
{
	BIT_SLEEP;
	HT1632C_PORT |= HT1632C_CS;
}

static void
ht1632c_bits(uint8_t bits, uint8_t n)
{
	uint8_t m;	/* bitmask */
	m = 1 << (n-1);	/* start with MSB */

	while ( m ) {	/* 8 bits, until we have shifted out the 1 */
		HT1632C_PORT &= ~ HT1632C_WRCLK;
		if ( bits & m )
			HT1632C_PORT |= HT1632C_DATA;
		else
			HT1632C_PORT &= ~ HT1632C_DATA;

		BIT_SLEEP;
		HT1632C_PORT |= HT1632C_WRCLK;
		BIT_SLEEP;

		m >>= 1;
	}
}

void
ht1632c_cmd(uint8_t cmd)
{
	ht1632c_start();
	ht1632c_bits(0x04, 3);	/* 1 0 0 */
	ht1632c_bits(cmd,  8);	/* ... command ... */
	ht1632c_bits(0,    1);	/* ... dummy? ... */
	ht1632c_stop();
}

void
ht1632c_bright(uint8_t val)
{
	ht1632c_cmd(0xa0 | (val & 0x0f));  /* 101X-vvvv-X */
}

void
ht1632c_onoff(uint8_t val){
	ht1632c_cmd(0x00 | !!val); /* 0000-0000-X and 0000-0001-X */
}

void
ht1632c_ledonoff(uint8_t val){
	ht1632c_cmd(0x02 | !!val); /* 0000-0010-X and 0000-0011-X */
}

void
ht1632c_blinkonoff(uint8_t val){
	ht1632c_cmd(0x08 | !!val); /* 0000-1000-X and 0000-1001-X */
}

void
ht1632c_slave(uint8_t val){
	val = val ? 0x04 : 0;
	ht1632c_cmd(0x10 | val ); /* 0001-00XX-X and 0001-01XX-X */
}

void
ht1632c_clock(uint8_t val){
	val = val ? 0x04 : 0;
	ht1632c_cmd(0x18 | val ); /* 0001-10XX-X and 0001-11XX-X */
}

void
ht1632c_opts(uint8_t val){
	val = (val & 0x03) << 2;
	ht1632c_cmd(0x20 | val ); /* 0010-abXX-X and 0001-11XX-X */
}

void
ht1632c_data4(uint8_t addr, uint8_t nibble)
{
	ht1632c_start();
	ht1632c_bits(0x05,  3 );  /* 1 0 1 */
	ht1632c_bits(addr,  7 );  /* ... command ... */
	ht1632c_bits(nibble,4 );  /* dataheet shows 4 dummy clock cycles? */
	ht1632c_stop();
}

void
ht1632c_data8(uint8_t addr, uint8_t byte)
{
	ht1632c_start();
	ht1632c_bits(0x05,  3 );  /* 1 0 1 */
	ht1632c_bits(addr,  7 );  /* ... command ... */
	ht1632c_bits(byte,  8 );  /* dataheet shows 4 dummy clock cycles? */
	ht1632c_stop();
}

void
ht1632c_flush_fb(uint8_t *fbmem)
{
	uint8_t addr=0;
	uint8_t fbbit=1;
	uint8_t byte, ledbit;

	ht1632c_start();
	ht1632c_bits(0x05,  3 );  /* 1 0 1 */
	ht1632c_bits(addr,  7 );  /* ... command ... */

	for(addr=0;addr < 64; addr+= 2){
		byte=0;

		/* for one 8-pixel-block from left to right
		 * the framebuffer (byte) address increases and
		 * the LED matrix controller bit number decreases, so...
		 */

		ledbit = 0x80; /* start MSB */
		while(ledbit){
			if(*fbmem & fbbit)
				byte |= ledbit;
			fbmem++;	/* next column in FB */
			ledbit >>= 1;	/* next column in LED controller */
		}
		fbmem -= 8;		/* move back FB memory pointer */

		ht1632c_bits(byte,  8 );  /* dataheet shows 4 dummy clock cycles? */

		fbbit <<= 1;		/* move to next row in FB */
		if(!fbbit){		/* reached bottom row?... */
			fbmem += 8;	/* move to next block */
			fbbit++;	/* start at 1 */
		}
	}

	ht1632c_stop();

}

void
ht1632c_init(void)
{
	uint8_t mask = HT1632C_WRCLK | HT1632C_CS | HT1632C_DATA;
	int i;

	HT1632C_PORT |= mask;
	HT1632C_DDR  |= mask;

	ht1632c_start();
	ht1632c_stop();

	ht1632c_onoff(0);
	ht1632c_onoff(1);
	ht1632c_slave(1); /* master mode */
	ht1632c_clock(0); /* internal RC clock */
	ht1632c_opts(0);  /* 0: 8 commons, n-mos outputs */
	ht1632c_bright(7);

	/* clear buffer memory */
	for(i=0;i<64;i++)
		ht1632c_data4(i,i);

	ht1632c_ledonoff(1); /* turn on */
}

