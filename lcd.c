/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 *
 * Modifications and improvements (custom characters) by Kasper F. Brandt
 * <poizan@poizan.dk>. Modifications are made available under the terms of
 * The Beer-Ware Licence as well :)
 *
 * Stdio demo, upper layer of LCD driver.
 *
 * $Id: lcd.c 1008 2005-12-28 21:38:59Z joerg_wunsch $
 */

#include "awesome-lights.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <avr/io.h>

#include <util/delay.h>

#include "hd44780.h"
#include "lcd.h"

/*
 * Setup the LCD controller.  First, call the hardware initialization
 * function, then adjust the display attributes we want.
 */
void lcd_init()
{

  hd44780_init();

  /*
   * Clear the display.
   */
  lcd_sendcmd(HD44780_CLR);

  /*
   * Entry mode: auto-increment address counter, no display shift in
   * effect.
   */
  lcd_sendcmd(HD44780_ENTMODE(1, 0));

  /*
   * Enable display, activate non-blinking cursor.
   */
  lcd_sendcmd(HD44780_DISPCTL(1, 0, 0));
}

/*
 * Send character c to the LCD display.  After a '\n' has been seen,
 * the next character will first clear the display.
 */
void lcd_senddata(uint8_t c)
{
#if 0
  static bool nl_seen;
  if (nl_seen && c != '\n')
    {
      /*
       * First character after newline, clear display and home cursor.
       */
      hd44780_wait_ready(0);
      hd44780_outcmd(HD44780_CLR);
      hd44780_wait_ready(0);
      hd44780_outcmd(HD44780_HOME);
      hd44780_wait_ready(0);
      hd44780_outcmd(HD44780_DDADDR(0));

      nl_seen = false;
    }
  if (c == '\n')
    {
      nl_seen = true;
    }
  else
    {
#endif
      hd44780_wait_ready(0);
      hd44780_outdata(c);
//    }
}

void lcd_sendcmd(uint8_t cmd)
{
	hd44780_wait_ready(0);
	hd44780_outcmd(cmd);
}

void lcd_clr(void)
{
	lcd_sendcmd(HD44780_CLR);
	lcd_sendcmd(HD44780_HOME);
}

void lcd_goto(uint8_t l, uint8_t c)
{
	lcd_sendcmd(HD44780_DDADDR(0x40*l|c));
}

void lcd_puts(char* s)
{
	for (; *s != '\0'; s++)
	{
		lcd_putchar(*s);
	}
}

static const char* hexdigits = "0123456789ABCDEF";

void lcd_hex(uint8_t v)
{
	lcd_putchar(hexdigits[v>>4]);
	lcd_putchar(hexdigits[v&7]);
}

//show byte scaled as 0..99
void lcd_btop(uint8_t v)
{
	uint8_t p = v*100 >> 8;
	lcd_putchar((p/10)+'0');
	lcd_putchar((p%10)+'0');
}

void lcd_raw_writebmp(uint8_t idx, uint8_t* data)
{
	lcd_sendcmd(HD44780_CGADDR(idx*8));
	for (uint8_t i=0; i < 8; i++)
	{
		lcd_senddata(data[i]);
	}
}

void lcd_lr_fb_writebmp(uint8_t v, uint8_t chridx)
{
	uint8_t row = ~((uint8_t)(0b11111 << (v+3)) >> (v+3)) & 0b11111;
	lcd_sendcmd(HD44780_CGADDR(chridx*8));
	for (uint8_t i=0; i < 8; i++)
	{
		lcd_senddata(row);
	}
}

//write block filled left-to-right with 0-5 columns of black pixels
void lcd_lr_fb(uint8_t v, uint8_t chridx)
{
	if (v == 0)
	{
		lcd_putchar(' ');
		return;
	}
	uint8_t oldaddr = hd44780_incmd() & 0x7F;
	lcd_lr_fb_writebmp(v, chridx);
	lcd_sendcmd(HD44780_DDADDR(oldaddr));
	lcd_putchar(chridx);
}

void lcd_writelrbmps(uint8_t idxstart)
{
	for (uint8_t v = 1; v <= 5; v++)
		lcd_lr_fb_writebmp(v, idxstart+v-1);
}

void lcd_lr_indicator3(uint8_t v, uint8_t idxstart)
{
	if (v == 0)
	{
		lcd_puts("   ");
		return;
	}
	uint8_t v1;
	uint8_t v2 = 0;
	uint8_t v3 = 0;
	if (v <= 85) // 85=255/3
	{
		// 5/85 = 1/17.
		// 8/17 ~= 0.5
		// so (v+8)/17 ~= floor(8/17+0.5) = round(8/17)
		v1 = (v+8)/17; 
	}
	else
	{
		v1 = 5;
		v -= 85;
		if (v <= 85)
		{
			v2 = (v+8)/17;
		}
		else
		{
			v2 = 5;
			v -= 85;
			v3 = (v+8)/17;
		}
	}
	lcd_putchar(v1 == 0 ? ' ' : idxstart+v1-1);
	lcd_putchar(v2 == 0 ? ' ' : idxstart+v2-1);
	lcd_putchar(v3 == 0 ? ' ' : idxstart+v3-1);
}
