/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 *
 * General stdiodemo defines
 *
 * $Id: defines.h 2186 2010-09-22 10:25:15Z aboyapati $
 */

#include "awesome-lights.h"

/* CPU frequency */
//#define F_CPU 1000000UL

/* HD44780 LCD port connections */
#define HD44780_RS C, 5
#define HD44780_RW C, 4
#define HD44780_E  C, 3
/* The data bits have to be not only in ascending order but also consecutive. */
#define HD44780_D8 B

/* Whether to read the busy flag, or fall back to
   worst-time delays. */
#define USE_BUSY_BIT 1
