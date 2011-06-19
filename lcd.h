/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 *
 * Stdio demo, upper layer of LCD driver.
 *
 * $Id: lcd.h 1008 2005-12-28 21:38:59Z joerg_wunsch $
 */

/*
 * Initialize LCD controller.  Performs a software reset.
 */
void	lcd_init(void);

/*
 * Send one character to the LCD.
 */
void lcd_senddata(uint8_t c);
#define lcd_putchar(c) lcd_senddata(c)
void lcd_sendcmd(uint8_t cmd);
void lcd_puts(char* s);
void lcd_clr(void);
void lcd_goto(uint8_t l, uint8_t c);
void lcd_hex(uint8_t v);
void lcd_btop(uint8_t v);
void lcd_raw_writebmp(uint8_t idx, uint8_t* data);

void lcd_lr_fb_writebmp(uint8_t v, uint8_t chridx);
void lcd_lr_fb(uint8_t v, uint8_t chridx);
void lcd_writelrbmps(uint8_t idxstart);
void lcd_lr_indicator3(uint8_t v, uint8_t idxstart);
