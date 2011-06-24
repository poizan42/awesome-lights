LIST      P=12f508
#include <p12f508.inc>
	__CONFIG _MCLRE_OFF & _CP_OFF & _WDT_OFF & _IntRC_OSC

#define IDX 0x00
#define BITLEN D'96'

TEST macro reg
	MOVF	reg,F
	endm

sel0a_pin EQU 0
#define sel0a GPIO,sel0a_pin
sel1a_pin EQU 1
#define sel1a GPIO,sel1a_pin
sel0b_pin EQU 2
#define sel0b GPIO,sel0b_pin
d_rxd_pin EQU 3
#define h_txd GPIO,d_rxd_pin
#define d_rxd h_txd
sel1b_pin EQU 4
#define sel1b GPIO,sel1b_pin
d_txd_pin EQU 5
#define h_rxd GPIO,d_txd_pin
#define d_txd h_rxd 
cfg_leds EQU 0x07
cfg_a EQU 0x07
cfg_red_a EQU 0x07
cfg_green_a EQU 0x08
cfg_blue_a EQU 0x09
cfg_b EQU 0x0A
cfg_red_b EQU 0x0A
cfg_green_b EQU 0x0B
cfg_blue_b EQU 0x0C
;idx EQU 0x0C
counter_a_on EQU 0x0D
counter_a_off EQU 0x0E
counter_b_on EQU 0x0F
counter_b_off EQU 0x10
a_color EQU 0x11
b_color EQU 0x12
last_TMR0 EQU 0x13
rx_flags EQU 0x14
#define rx_running rx_flags,0
#define rx_invalue rx_flags,2
rx_bitnum EQU 0x15
rx_cmd EQU 0x16
rx_data EQU 0x17
const7 EQU 0x18

;LED PWM'ing
;if counter_a_on > 0
;	counter_a_on--
;	if a_color,0
;		sel0a = 1
;	if ! a_color,0
;		sel0a = 0
;	if a_color,1
;		sel1a = 1
;	if ! a_color,1
;		sel1a = 0
;else if counter_a_off > 0
;	counter_a_off--
;	sel0a = 1
;	sel1a = 1
;	(pad with nops to match cycle count)
;else (load next color - only 1/255 of the time, so timing not important)
;	a_color--
;	if a_color,7 (wraparound, was 0 before)
;		a_color = 2 (blue)
;	counter_a_on = cfg_a[a_color]
;	counter_a_off = ~counter_a_on
;
;repeat for b

pwm_routine macro counter_on,counter_off,color,cfg,sel0,sel1
	local do_on,do_off,_end
	;if counter_on > 0
	TEST	counter_on		;1 Z = counter_on == 0
	BTFSS	STATUS,Z		;2 skip next if counter_on == 0
	GOTO	do_on			;*+1
	;else if counter_off > 0
	TEST	counter_off		;1
	BTFSS	STATUS,Z		;2
	GOTO	do_off			;*+1
	;else
	DECF	color,F 		;1 color--
	MOVLW	2				;1
	BTFSC	color,7			;2 wraparound?
	MOVWF	color			;* color = 2 (blue)
	MOVLW	cfg				;1 load address of cfg into W
	ADDWF	color,W			;1 W = &cfg + color
	MOVWF	FSR				;1 load &cfg[color]
	MOVF	INDF,W			;1 load cfg[color] into W
	MOVWF	counter_on		;1 counter_on = cfg[color]
	XORLW	0xFF			;1 W = ~counter_on
	MOVWF	counter_off		;1 counter_off = ~counter_on
	GOTO	_end			;2
	;cycles:
	;before: 6
	;body: 13
	;total: 19 (4 surplus)
do_on
	DECF	counter_on,F	;1
	BTFSC	color,0			;2
	BSF		GPIO,sel0		;*
	BTFSS	color,0			;2
	BCF		GPIO,sel1		;*
	BTFSC	color,1			;2
	BSF		GPIO,sel0		;*
	BTFSS	color,1			;2
	BCF		GPIO,sel1		;*
	GOTO _end				;2
	;cycles:
	;before: 4
	;body:   11
	;total:  15
do_off
	;cycles before: 7
	DECF	counter_off,F	;1
	BSF		GPIO,0			;1
	BSF		GPIO,1			;1
	NOP
	NOP
	NOP
	NOP
	NOP
	;cycles:
	;before: 7
	;body: 8
	;total: 15
_end
	;cycles:
	;min: 15 (255/256 iterations)
	;max: 19 (1/256 iterations)
	endm


	ORG	0 ;reset vector
;	GOTO init; - no interrupts
init
	;load factory programmed OSCCAL value
	MOVWF	OSCCAL
	;initialize OPTION register
	MOVLW	B'11001000'
	OPTION
	;d_rxd is always input. d_txd is input when we are not sending
	;(high impedance state, so we won't get a bus conflict)
	MOVLW	B'101000'
	TRIS	GPIO
	CLRF	GPIO
;	MOVLW	IDX_VALUE
;	MOVWF	idx
	MOVLW	0x07
	MOVWF	const7
	CLRF	counter_a_on
	CLRF	counter_a_off
	CLRF	counter_b_on
	CLRF	counter_b_off
	CLRF	a_color
	CLRF	b_color
	CLRF	rx_flags
	;initialize timer

main
;rx routine
;if !rx_running
;	if !d_rx (start bit - begin read)
;		TMR0 = 0
;		rx_running = 1
;		rx_data = 0
;		rx_bitnum = 0
;else
;	if TMR0 < BITLEN (new bit ready?)
;		goto rx_end
;	TMR0 = TMR0-BITLEN+adjustment
;	if !rx_bitnum,3 (rx_bitnum < 8 - data bit)
;		rx_data <<= 1
;		if d_rx
;			rx_data,0 = 1
;		rx_bitnum++
;	else (rx_bitnum=8 - stopbit expected)
;		rx_running = 0
;		if !d_rx (stop bit - framing error :( )
;			goto rx_end	
;		if !rx_invalue
;			if rx_data & 0xF0 != IDX << 4 (not to us)
;				goto rx_end
;			rx_cmd = rx_data & 0x0F
;			rx_invalue = 1
;		else (done)
;			rx_invalue = 0
;			cfg_leds[rx_cmd] = rx_data
;rx_end:

rx_start
	BTFSC	rx_running		;2
	GOTO	rx_is_running	;*+1 if rx_running - total 3
	;if !rx_running
	BTFSC	d_rxd			;2
	GOTO	rx_end			;*+1 if d_rx (no start bit) - total 5
	CLRF	TMR0			;1   TMR0 = 0
	BSF		rx_running		;1   rx_running = 1
	CLRF	rx_data			;1   rx_data = 0
	CLRF	rx_bitnum		;1   rx_bitnum = 0
	GOTO	rx_end			;2 - total 10
rx_is_running ;3 before
	MOVLW	BITLEN			;1
	SUBWF	TMR0,W			;1   W=TMR0-W=TMR0-BITLEN
	;C set if a borrow did not occour
	;borrow if BITLEN > TMR0, so !C=TMR0 < BITLEN
	BTFSS	STATUS,C		;2
	GOTO	rx_end			;*+1 TMR0 < BITLEN - total 8
;See fig. 6-2
;0=SUBWF, 1=BTFSS, 2=NOP (GOTO replacement), 3=ADDLW
;D=O-BITLEN
;           Q1  Q2  Q3  Q4| Q1  Q2  Q3  Q4| Q1  Q2  Q3  Q4| Q1  Q2  Q3  Q4| Q1  Q2  Q3  Q4| Q1  Q2  Q3  Q4| Q1  Q2  Q3  Q4| Q1  Q2  Q3  Q4| Q1  Q2  Q3  Q4
;fetch		SUBWF           BTFSS			NOP             ADDWF           MOVWF           *               *               *               *
;desired	O+0         O+1             O+2             O+3             O+4             O+5             D+6             D+7             D+8             D+9
;                               ^SUBWF
;TMR0       O+0         O+1             O+2             O+3             O+4             O+5             N+0                                             N+1
;S = O+1 <=> O = S-1
;N+1 = D+9 = O-BITLEN+9 = S-1-BITLEN+9 => N = S-1-BITLEN+9-1 = S-BITLEN+7
	ADDWF	const7,W		;1
	MOVWF	TMR0			;1
	BTFSC	rx_bitnum,3		;2 - total 11
	GOTO	rx_stopbit		;*+1 rx_bitnum == 8 - total 12
	;rx_bitnum < 8
	RLF		rx_data,F		;1   rx_data <<= 1
	BTFSC	d_rxd			;2
	BSF		rx_data,0		;*   rx_data |= 1
	INCF	rx_bitnum,F		;1
	GOTO	rx_end			;2 - total 17
rx_stopbit ; before: 12
	BCF		rx_running		;1   rx_running = 0
	BTFSS	d_rxd			;2
	GOTO	rx_end			;*+1 !d_rx - framing error - total 16
	BTFSC	rx_invalue		;2
	GOTO	rx_read_value	;*+1 if rx_invalue - total 18
	; if !rx_invalue
	MOVF	rx_data,W		;1
	ANDLW	0xF0			;1   W=rx_data & 0xF0
	XORLW	IDX << 4		;1   Z=rx_data & 0xF0 == IDX << 4
	BTFSS	STATUS,Z		;2
	GOTO	rx_end			;*+1 rx_data & 0xF0 != IDX << 4 - not to us - total 23
	MOVF	rx_data,W		;1
	ANDLW	0x0F			;1   W=rx_data & 0x0F
	MOVWF	rx_cmd			;1   rx_cmd=rx_data & 0x0F
	BSF		rx_invalue		;1   rx_invalue = 1
	GOTO	rx_end			;2   total 28
rx_read_value ; before: 18
	BCF		rx_invalue		;1
	MOVLW	cfg_leds		;1   W = &cfg_leds
	ADDWF	rx_cmd,W		;1   W = &cfg_leds[rx_cmd]
	MOVWF	FSR				;1   load &cfg_leds[rx_cmd]
	MOVF	rx_data,W		;1   W = rx_data
	MOVWF	INDF			;1   cfg_leds[rx_cmd] = rx_data
	;total: 24
rx_end
;cycles:
;min: 5
;max: 28
;

pwm_a
	pwm_routine counter_a_on,counter_a_off,a_color,cfg_a,sel0a_pin,sel1a_pin
pwm_b
	pwm_routine counter_b_on,counter_b_off,b_color,cfg_b,sel0b_pin,sel1b_pin
;pwm: min 30, max 38
	GOTO main				;2
;max cycles: 28+38+2 = 68

;	BTFSC	txd
;	BSF		sel0a
;	BTFSS	txd
;	BCF		sel0a
;	GOTO	loop

;	ORG		0x1FF ; RC cal
;	DATA 	0xC2A

END
