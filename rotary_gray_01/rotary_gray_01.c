/**********************************************/
/* From http://embdev.net/articles/Rotary_Encoders */
/*  Reading rotary encoder                     */
/* one, two and four step encoders supported	*/
/*                                            */
/* Author: Peter Dannegger                    */
/*                                            */
/**********************************************/

#include <avr/io.h>
#include <avr/interrupt.h>

// target: ATmega16 //------------------------------------------------------------------------

#define	XTAL	8e6	// 8MHz

// define the two inputs that the encoder is connected to
#define PHASE_A	(PINB & 1<<PB1)
#define PHASE_B	(PINB & 1<<PB3)
#define LEDS_DDR DDRB
#define	LEDS PORTB	// LEDs against VCC

volatile int8_t enc_delta;	// -128 ... 127
static int8_t last;


void encode_init( void ) {
 int8_t new;
 new = 0;
 if( PHASE_A )
   new = 3;
 if( PHASE_B )
   new ^= 1;					// convert gray to binary
 last = new;					// power on state
 enc_delta = 0;
 TCCR0 = 1<<WGM01^1<<CS01^1<<CS00;		// CTC, XTAL / 64
 OCR0 = (uint8_t)(XTAL / 64.0 * 1e-3 - 0.5);	// 1ms
 TIMSK |= 1<<OCIE0;
}



// Timer interrupt handler. Should be executed periodically, e.g. once every 1ms (1kHz)
ISR( TIMER0_COMP_vect ) {

 int8_t new, diff;
 new = 0;
 if( PHASE_A )
   new = 3;
 if( PHASE_B )
   new ^= 1;					// convert gray to binary
 diff = last - new;				// difference last - new
 if( diff & 1 ){				// bit 0 = value (1)
   last = new;					// store new as next last
   enc_delta += (diff & 2) - 1;		// bit 1 = direction (+/-)
 }
}



int8_t encode_read1( void )	 { // read single step encoders
 int8_t val;
 cli();
 val = enc_delta;
 enc_delta = 0;
 sei();
 return val;					// counts since last call
}





int8_t encode_read2( void )	{ // read two step encoders
 int8_t val;
 cli();
 val = enc_delta;
 enc_delta = val & 1;
 sei();
 return val >> 1;
}


int8_t encode_read4( void )	 { // read four step encoders
 int8_t val;
 cli();
 val = enc_delta;
 enc_delta = val & 3;
 sei();
 return val >> 2;
}

// example main program
int main( void ) {
 int32_t val = 0;
 LEDS_DDR = 0xFF;
 encode_init();
 sei();
 for(;;){
   val += encode_read1();			// read a single step encoder
   LEDS = val;
 }
}


