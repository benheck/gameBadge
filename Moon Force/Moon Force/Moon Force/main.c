#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define F_CPU 20000000UL
#include "drivers/gameBadge.h"
#include "game.h"

int main(void) {
	
	CCP = 0xD8;
	CLKCTRL.MCLKCTRLB = 0;							//Run CPU at full speed, disregarding speed considerings of the datasheet
	//CLKCTRL.MCLKCTRLB = 1;							//Enable prescaler for 10MHz / 2 (10MHz) required for 3.3v operation
	while(!(CLKCTRL.MCLKSTATUS & 0x10)) {}			//Sync

	//--Setup Real Time Counter (RTC) for clock use-----------------------------------------------

	CCP = 0xD8;
	CLKCTRL.XOSC32KCTRLA = CLKCTRL_ENABLE_bm;		//Enable the external crystal
	while (RTC.STATUS > 0) {} /* Wait for all register to be synchronized */
	
	RTC.CTRLA = RTC_PRESCALER_DIV1_gc   /* 1 */
	| 0 << RTC_RTCEN_bp     /* Enable: disabled */
	| 1 << RTC_RUNSTDBY_bp; /* Run In Standby: enabled */

	RTC.CLKSEL = RTC_CLKSEL_TOSC32K_gc; /* 32.768kHz External Crystal Oscillator (XOSC32K) */
	while (RTC.PITSTATUS > 0) {} /* Wait for all register to be synchronized */

	RTC.PITCTRLA = RTC_PERIOD_CYC32768_gc /* RTC Clock Cycles 32768 */
	| 1 << RTC_PITEN_bp;   /* Enable: enabled */

	RTC.PITINTCTRL = 1 << RTC_PI_bp; /* Periodic Interrupt: enabled */
	
	//--Setup IO direction--------------------------------------------------------------------------
	
	PORTA_DIR = 0b00011010;							//Set pin directions
	PORTB_DIR = 0b00110011;							//Speaker and LED out
	PORTC_DIR = 0b00000000;

	PORTC_PIN0CTRL = 0x88;							//Set pullups for control buttons, with inverse output (easier logic)
	PORTC_PIN1CTRL = 0x88;
	PORTC_PIN2CTRL = 0x88;
	PORTC_PIN3CTRL = 0x88;
	PORTA_PIN5CTRL = 0x88 | 0x01;					//Menu button, can also wake from sleep (ISR on either edge)
	PORTA_PIN6CTRL = 0x88;
	PORTA_PIN7CTRL = 0x88;

	//--Setup SPI bus and intialize OLED display----------------------------------------------------
	SPI0.CTRLA = SPI_MASTER_bm | SPI_ENABLE_bm | SPI_CLK2X_bm;	//Start SPI bus

	displayInit(0);									//Setup display
	
	SPI0_CTRLB = SPI_BUFEN_bm;						//Set high speed buffer SPI mode now that the OLED is set up

	//--Setup FRAMERATE TIMER (eat this PS5!)------------------------------------------------------
	TCB0.INTCTRL = TCB_CAPT_bm;						//Setup Timer B as compare capture mode that will trigger an interrupt
	TCB0_CCMP = 10000;								//CLK   (20MHz, 3.3 less safe mode) -> DIV2 prescaler = 10000 ticks/ms
	//TCB0_CCMP = 5000;								//CLK/2 (10MHz, 3.3 safe mode) -> DIV2 prescaler = 5000 ticks/ms
	TCB0_CTRLA = (1 << 1) | TCB_ENABLE_bm;

	//--Setup PIEZO SOUND (the new Dolby Atmos)---------------------------------------------------
	TCA0.SINGLE.CTRLB =
	TCA_SINGLE_CMP2EN_bm |							// Enables Compare Channel 2 (output port WO2 (alt pin per PORTMUX below) = PB5)
	TCA_SINGLE_WGMODE_FRQ_gc;						// FRQ Waveform Generation
	
	TCA0.SINGLE.CMP0 = 0;
	//TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV8_gc;	//10MHz mode
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc;		//20MHz mode
	
	PORTMUX_CTRLC = 0x04;							//Write this bit to '1' to select the alternative output pin for TCA0 waveform output 2.

	//--Setup ADC for random number generator-----------------------------------------------------
	
	ADC0.CTRLA = 0b00000001;						//Enable ADC0
	ADC0.CTRLC = 0b00010011;						//VDD reference, clk/16
	ADC0.MUXPOS = 0x02;								//ADC on AIN2 (PA2)
	
	ADC1.CTRLA = 0b00000001;						//Enable ADC1
	ADC1.CTRLC = 0b00010011;						//VDD reference, clk/16
	ADC1.MUXPOS = 0x06;								//ADC on AIN2 (PC0)
	
	//--Setup deep sleep mode---------------------------------------------------------------------

	SLPCTRL.CTRLA = (0x02 << 1) | 1;				//Set sleep mode (enabled, power-down mode, deepest sleep mode)

	//--Game Start and Main Loop------------------------------------------------------------------

	sei();							//Enable interrupts
	
	gameSetup();					//One-time user game setup code

	systemLoop();					//Jump to the main loop in the code

}
