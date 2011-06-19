#include "awesome-lights.h"

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
//#include <util/delay.h>

#include "musiclights.h"
#include "knobs.h"
#include "lcd.h"
#include "ffft.h"

#define ADC_vector ((void*)0x54)

int16_t capture_buffer[FFT_N];
complex_t bfly_buff[FFT_N];     /* FFT buffer */

extern void waveform_capture_handler(void);

static void musiclights_init(void)
{
	//PC4,PC5 is input
	DDRC = DDRC & 0b11001111;
	//dactivate digital input on PC4,PC5
	DIDR0 |= 0b00110000;
	//prescaler = 32
	ADCSRA = (ADCSRA & 0b11111000) | 101;
	//F_CPU/(13*prescaler) = 19 230.7692
	//2CH, sampling at 9 615.38462 Hz
	//replace vector
	cli();
	adchandler = waveform_capture_handler;
	//right adjust
	ADMUX = (ADMUX & ~((1<<ADLAR)|0x0F)) | 4;
	sei();
}

static void musiclights_shutdown(void)
{
	/* it is possible that the next knob value will be
	   from the wrong adc - we don't care about that
	*/
	//restore ISR
	cli();
	adchandler = knobs_adchandler;
	ADMUX |= (1<<ADLAR);
	//prescaler = 128
	ADCSRA = (ADCSRA & 0b11111000) | 111;
	sei();
}

#if 0
static void waveform_capture_handler(void)
{
	/* 13*32 = 416 clockcycles between calls. */
	static uint8_t curch = 0; 
	static uint8_t nextnextadc;
	static COLOR curcolor = BLUE;
	static uint8_t count = 0;

	//capture knobs values 1 of 85 times
	//this will sample knobs at a rate of ~ 75 Hz
	static uint8_t kcount = 3;
	if (kcount == 85)
	{
		kcount = 255; //=-1
		//schedule read of next knob
		curcolor = (curcolor+1)%3;
		nextnextadc = curcolor+1;
	}
	else if (kcount == 2) //color value
	{
		knobs[curcolor] = ADCH;
		goto exit;
	}
	else
	{
		//schedule next read of *this* channel
		nextnextadc = 4 + curch;//ADC4 or ADC5
		curch = !curch;
	}
	capture_buffer[count] = (ADCH << 8) | ADCL;
	ADMUX = (ADMUX & 0xF0) | nextnextadc;
	if (count == FFT_N)
	{
		
		sei();
		count = 255;//-1
	}
	exit:
		count++;
		kcount++;
		return;
}
#endif

void musiclights_main(void)
{
	musiclights_init();
	while (program_running)
	{
	}
	musiclights_shutdown();
}
