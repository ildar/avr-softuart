/* 
	AVR Software-Uart Demo-Application 
	Version 0.4, 10/2010
	
	by Martin Thomas, Kaiserslautern, Germany
	<eversmith@heizung-thomas.de>
	http://www.siwawi.arubi.uni-kl.de/avr_projects
*/

/* 
Test environment/settings: 
- avr-gcc 4.3.3/avr-libc 1.6.7 (WinAVR 3/2010)
- Atmel ATmega324P @ 8MHz internal RC, ATtiny85 @ 1MHz internal RC
- 2400bps
*/

/*
AVR Memory Usage (-Os, no-inline small-functions, relax)
----------------
Device: atmega324p

Program:     926 bytes (2.8% Full)
(.text + .data + .bootloader)

Data:         52 bytes (2.5% Full)
(.data + .bss + .noinit)


AVR Memory Usage (-Os)
----------------
Device: attiny85

Program:     828 bytes (10.1% Full)
(.text + .data + .bootloader)

Data:         52 bytes (10.2% Full)
(.data + .bss + .noinit)

*/

#define WITH_STDIO_DEMO   0 /* 1: enable, 0: disable */

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#define LEDOUT PORTB5
#define LEDPORT PORTB
#define LEDDDR DDRB
#define LEDDDRPIN DDB5

#include "softuart.h"


#if WITH_STDIO_DEMO
#include <stdio.h>

// interface between avr-libc stdio and software-UART
static int my_stdio_putchar( char c, FILE *stream )
{
	if ( c == '\n' ) {
		softuart_putchar( '\r' );
	}
	softuart_putchar( c );

	return 0;
}

FILE suart_stream = FDEV_SETUP_STREAM( my_stdio_putchar, NULL, _FDEV_SETUP_WRITE );

static void stdio_demo_func( void ) 
{
	stdout = &suart_stream;
	printf( "This output done with printf\n" );
	printf_P( PSTR("This output done with printf_P\n") );
}
#endif /* WITH_STDIO_DEMO */

void delay_ms(unsigned int xms)
{
        while(xms){
                _delay_ms(0.96);
                xms--;
        }
}

int main(void)
{
	char c;
	static const char pstring[] PROGMEM = 
		"adapted for Atmel AVR and this demo by Martin Thomas\r\n";
	unsigned short cnt = 0;
#if (F_CPU > 4000000UL)
#define CNTHALLO (unsigned int)(0xFFFF)
#else 
#define CNTHALLO (unsigned int)(0xFFFF/3)
#endif

	/* enable pin as output */
	LEDDDR|= (1<<LEDDDRPIN);

	softuart_init();
	softuart_turn_rx_on(); /* redundant - on by default */
	
	sei();

	LEDPORT|= (1<<LEDOUT); // led on, pin=1
	softuart_puts_P( "\r\nSoftuart Demo-Application\r\n" );    // "implicit" PSTR
	softuart_puts_p( PSTR("generic softuart driver code by Colin Gittins\r\n") ); // explicit PSTR
	softuart_puts_p( pstring ); // pstring defined with PROGMEM
	softuart_puts( "--\r\n" );  // string "from RAM"
	LEDPORT &= ~(1<<LEDOUT); // led off, pin=0

#if WITH_STDIO_DEMO
	stdio_demo_func();
#endif
	
	while ( !softuart_kbhit() ) {
		LEDPORT|= (1<<LEDOUT); // led on, pin=1
		softuart_putchar( '.' );
		LEDPORT &= ~(1<<LEDOUT); // led off, pin=0
		delay_ms(500);
	}
	
	for (;;)
		if ( softuart_kbhit() ) {
			c = softuart_getchar();
			LEDPORT|= (1<<LEDOUT); // led on, pin=1
			softuart_putchar( '[' );
			softuart_putchar( c );
			softuart_putchar( ']' );
			LEDPORT &= ~(1<<LEDOUT); // led off, pin=0
		} else
			delay_ms(100);
	
	return 0; /* never reached */
}
