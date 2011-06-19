#include "awesome-lights.h" 

#include <stdlib.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "knobs.h"

volatile uint8_t knobs[3];
void (*adchandler)(void) = knobs_adchandler;

#define STRINGIFY(x) #x
#define S(x) STRINGIFY(x)
/*
overhead (cycles):
interrupt:	4
vector:		3
stub:		11 unitteruptible + 8 interruptible = 19
total:		18 + 8 = 26
*/
ISR(ADC_vect, ISR_NAKED)
{
	asm volatile(
		"push "S(ZH)"\n" //2
		"push "S(ZL)"\n" //2
		"lds "S(ZL)", adchandler\n"	//2
		"lds "S(ZH)", adchandler+1\n"	//2
		"icall\n"	//3
		"pop "S(ZL)"\n" //2
		"pop "S(ZH)"\n" //2
		//reti should be called by the real handler
		"ret" //4
		::
	);
}

void knobs_adchandler(void)
{
	static COLOR curcolor = RED;
	const COLOR nextcolors[3] = {GREEN, BLUE, RED};
	const COLOR nextnextcolors[3] = {BLUE, RED, GREEN};
	uint8_t nextnextadc;
	nextnextadc = nextnextcolors[curcolor]+1;
	ADMUX = (ADMUX & 0xF0) | nextnextadc;
	uint8_t val = ADCH;
	val = val > 127 ? 127 : val;
	knobs[curcolor] = val;
	curcolor = nextcolors[curcolor];
}

void knobs_init(void)
{
	//PC1,PC2,PC3 is input
	DDRC = DDRC & 0b11110001;
	//use AVcc, left adjust result, ADC1 selected
	ADMUX = 0b01100001;
	//deactivate digital input on PC1,PC2,PC3
	DIDR0 |= 0b00001110;
	//no analog comperator, free running
	ADCSRB = 0b00000000;
	//enable ADC, start conversion, auto trigger, interrupt, prescaler: 128
	ADCSRA = 0b11101111;
	//interrupts per second: F_CPU/(13*prescaler) = 4 807.69231
	//time between interrupts: 208us
	ADMUX = 0b01100011; //select ADC2 as next
}
