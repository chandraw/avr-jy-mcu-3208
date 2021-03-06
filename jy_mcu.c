#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "serial.h"
#include "ht1632c.h"
#include "fonts.h"


static const PROGMEM char hello_str[]="\n\n"
	"\t****************************************************\n"
	"\t**  JY-MCU 3208 Lattice Clock                     **\n"
	"\t**  (c) 2013 Christian Vogel <vogelchr@vogel.cx>  **\n"
	"\t****************************************************\n\n";

static uint8_t bright = 0;	/* current LED brightness */
static uint8_t fb[32];		/* framebuffer */
static char buf[8]; /* general purpose */

static void
led_msg(const char *msg)
{
	ht1632c_clear_fb(fb);
	font_puts_RAM(FONT4X5, msg, fb, 0, HT1632C_WIDTH, 0, FONT_STAMP_NORM);
	ht1632c_flush_fb(fb);
}

int
main(void)
{
	char c;

	serial_init();
	ht1632c_init();

	puts_P(hello_str);

	/* fill random text pattern */
	for(c=0;c<32;c++)
		fb[(int)c]=c;
	ht1632c_flush_fb(fb);

	for(;;){
		if(! serial_status()) /* wait for charater to be input */
			continue;
		c = getchar();

		if(c == '-' || c == '+'){
			if(c == '+' && bright < 15)
				bright++;
			if(c == '-' && bright > 0)
				bright--;

			printf_P(PSTR("Brightness set to %d.\n"),bright);

			sprintf_P(buf,PSTR("BR %d"),bright);
			puts(buf);
			led_msg(buf);

			ht1632c_bright(bright);
		}

		if(c == 'o' || c == 'f')
			ht1632c_onoff(c == 'o');

		if(c == 'B' || c == 'b')
			ht1632c_blinkonoff(c == 'B');

		if(c == '?'){
			puts_P(hello_str);
		}

		if(c == 't'){
			ht1632c_clear_fb(fb);
			font_puts_P(FONT6X8, PSTR("0123"), fb, 0, HT1632C_WIDTH, 0, FONT_STAMP_NORM);
			ht1632c_flush_fb(fb);
		}

		if(c == 'T'){
			ht1632c_clear_fb(fb);
			font_puts_P(FONT4X5, PSTR("HELLO@"), fb, 0, HT1632C_WIDTH, 0, FONT_STAMP_NORM);
			ht1632c_flush_fb(fb);
		}
	}
}
