#include "awesome-lights.h"

#include <inttypes.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>

#include "samecolor.h"
#include "knobs.h"
#include "lcd.h"

static void samecolor_init(void)
{
	lcd_clr();
	lcd_puts("SameColor");
	lcd_goto(1,0);

	//2, 8, 14
	lcd_puts("r    g    b");
	lcd_writelrbmps(0);
}

static const uint8_t color_pos[3] = {1,6,11};

static void samecolor_shutdown(void)
{
}
/*root(128, 127)^n-1
b=pow(128.0,1.0/127.0)

[int(round(pow(b,n)-1)) for n in range(0,128)]
	or
vals = [i*128/(128+11) for i in range(11,128+11)]
[int(round(pow(b,n)-1)) for n in vals]
*/
/*static uint8_t exptbl[128] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 9, 9, 9, 10, 10, 11, 11, 11, 12, 12, 13, 14, 14, 15, 15, 16, 17, 17, 18, 19, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 33, 34, 35, 37, 38, 40, 41, 43, 45, 46, 48, 50, 52, 54, 56, 59, 61, 63, 66, 68, 71, 74, 77, 80, 83, 86, 90, 93, 97, 101, 105, 109, 113, 118, 122, 127
};*/

static uint8_t exptbl[128] = {
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 17, 17, 18, 19, 19, 20, 21, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 33, 34, 34, 35, 37, 38, 40, 41, 43, 45, 46, 48, 50, 52, 52, 54, 56, 59, 61, 63, 66, 68, 71, 74, 77, 80, 83, 83, 86, 90, 93, 97, 101, 105, 109, 113, 118, 122, 127
};

void samecolor_main(void)
{
	samecolor_init();
	while (program_running)
	{
		for (COLOR c = RED; c < NONE; c++)
		{
			uint8_t val = knobs[c];
			lcd_goto(1, color_pos[c]);
			//lcd_btop(2*val);
			lcd_lr_indicator3(2*val, 0);
			for (uint8_t i=0; i < LAMP_COUNT; i++)
			{
				set_diode_cfg(i, c, 2*exptbl[val])
			}
		}
		_delay_ms(1);
	}
	samecolor_shutdown();
}
