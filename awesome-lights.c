#include "awesome-lights.h"

#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "awesome-lights.h"
#include "hd44780.h"
#include "lcd.h"
#include "knobs.h"
#include "samecolor.h"

volatile bool program_running = false;

void handle_diode_timer(void);

#define DIODES_DEBUG

#ifdef DIODES_DEBUG
#define DIODE_IDX(i,c) ((!i && c!=NONE) ? 1 << (c) : 0x00)
#define DIODES_OFF_VAL 0x00
#else
#define DIODE_IDX(i,c) (i<<2|c)
#define DIODES_OFF_VAL 0x7F
#endif

void set_diode(uint8_t idx, COLOR color)
{
	DIODE_PORT = DIODE_IDX(idx, color);
}

void diodes_off(void)
{
	DIODE_PORT = DIODES_OFF_VAL;
}

/*const uint8_t diode_src_config[20][3] = {
	{  0,  5, 10},{ 15, 20, 25},{ 30, 35, 40},{ 45, 50, 55},
	{ 60, 65, 70},{ 75, 80, 85},{ 90, 95,100},{105,110,115},
	{120,125,130},{135,140,145},{150,155,160},{165,170,175},
	{180,185,190},{195,200,205},{210,215,220},{225,230,235},
	{240,245,250},{255,225,200},{175,150,125},{100, 75, 50},
};*/

/*const uint8_t diode_src_config[20][3] = {
	{255,255,255},{255,255,255},{255,255,255},{255,255,255},
	{255,255,255},{255,255,255},{255,255,255},{255,255,255},
	{255,255,255},{255,255,255},{255,255,255},{255,255,255},
	{255,255,255},{255,255,255},{255,255,255},{255,255,255},
	{255,255,255},{255,255,255},{255,255,255},{255,255,255}
};*/

const uint8_t diode_src_config[20][3] = {
	{  0,255,  0},{  0,  0,  0},{  0,  0,  0},{  0,  0,  0},
	{  0,  0,  0},{  0,  0,  0},{  0,  0,  0},{  0,  0,  0},
	{  0,  0,  0},{  0,  0,  0},{  0,  0,  0},{  0,  0,  0},
	{  0,  0,  0},{  0,  0,  0},{  0,  0,  0},{  0,  0,  0},
	{  0,  0,  0},{  0,  0,  0},{  0,  0,  0},{  0,  0,  0},
};

/*const uint8_t diode_src_config[20][3] = {
	{  0,  0,  0},{255,  0,  0},{  0,  0,  0},{  0,  0,  0},
	{  0,  0,  0},{  0,  0,  0},{  0,  0,  0},{  0,  0,  0},
	{  0,  0,  0},{  0,  0,  0},{  0,  0,  0},{  0,  0,  0},
	{  0,  0,  0},{  0,  0,  0},{  0,  0,  0},{  0,  0,  0},
	{  0,  0,  0},{  0,  0,  0},{  0,  0,  0},{  0,  0,  0},
};*/

uint8_t diode_config[20][3];
#define diode_src_config_p ((uint8_t*)&diode_src_config)

ISR(TIMER0_OVF_vect)
{
	handle_diode_timer();
}

void handle_diode_timer(void)
{
	static uint8_t idx = 0;
	static COLOR color = RED;
	static bool active = true;
	uint8_t new_idx;
	if (active)
		new_idx = DIODE_IDX(idx, color);
	else
	{
		new_idx = DIODES_OFF_VAL;
		color++;
	}
	if (color == NONE)
	{
		color = RED;
		idx++;
}
	if (idx == LAMP_COUNT)
		idx = 0;

	//uint8_t scaledtime = diode_config[idx][color];
	uint8_t cfg_idx = idx*3;
	cfg_idx += color;
	//uint8_t scaledtime = diode_config_p[(uint8_t)(3*idx+color)]; //stupid compiler is stupid...
	uint8_t scaledtime = diode_config_p[cfg_idx]; //stupid compiler is stupid...
	scaledtime = active ? scaledtime : 255 - scaledtime;
	active = !active;
	if (scaledtime == 0)
	{
		handle_diode_timer();
		return;
	}
	else
		DIODE_PORT = (DIODE_PORT&0x80) | new_idx;
	TCNT0 = 256-scaledtime;
}

uint8_t _v = 5;

void show_banner(void)
{
	lcd_puts("*Awesome Lights*");
	lcd_goto(1,5);
	//"     V. M.N     "
	lcd_puts("V. 0.1");
	_delay_ms(1000); // lav vildt lys her!
	lcd_goto(1,0);
	lcd_puts("bit.ly/jctXkX   ");
	_delay_ms(1000);
}

__attribute__((OS_main)) int main(void)
{
	CLKPR = 1 << CLKPCE; //enable clock prescaler change
	CLKPR = 0; //clock prescaler = 1
	//Now running at 8MHz
	//Timer/Counter0 and ADC enabled
	PRR = 0b11001110;
	//PORTB = out
	DDRD = 0b01111111;
	//diode_config = malloc(20*3);
	for (uint8_t i=0; i<20*3; i++)
		for (COLOR c=RED; c <= BLUE; c++)
		{
			uint8_t idx = 3*i+c;
			diode_config_p[idx] = (TIMESLICE*diode_src_config_p[idx]) >> 8;
		}
	// Prescaler = FCPU/8
	TCCR0B |= 1<<CS01;
	//Initialize Counter
	handle_diode_timer();
	//Enable Overflow Interrupt Enable
	TIMSK0|=(1<<TOIE0);
	sei();
	lcd_init();
	show_banner();
	knobs_init();
	program_running = true;
	samecolor_main();
	//musiclights_main();
	//while (true) {}
}
