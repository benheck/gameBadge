#include "sam.h"
#include "drivers/clocks.h"						//Clock drivers
#include "drivers/spi.h"
#include "drivers/gameBadge.h"
#include "game.h"

int main(void) {
	
    /* Initialize the SAM system */
    SystemInit();
	clocksInit48Mhz();
	
	PORT->Group[0].DIRSET.reg = PORT_PA02;										//D/C as OUTPUT
	PORT->Group[0].OUTSET.reg = PORT_PA02;										//Normally HIGH
	PORT->Group[0].DIRSET.reg = PORT_PA09;										//OLED/RESET as OUTPUT
	PORT->Group[0].OUTSET.reg = PORT_PA09;										//Normally HIGH (not reset)
	
	PORT->Group[0].DIRSET.reg = PORT_PA24;										//OLED/RESET as OUTPUT
	PORT->Group[0].OUTSET.reg = PORT_PA24;										//Normally HIGH (not reset)

	PORT->Group[0].DIRSET.reg = PORT_PA08;										//OLED/RESET as OUTPUT
	PORT->Group[0].OUTCLR.reg = PORT_PA08;										//Normally HIGH (not reset)
	
	
	SPI_init();	
	displayInit();
	

	PM->APBCMASK.bit.TCC0_ = 1;					//Setup TCC timer 0
	
	// Set up TCC0 for PWM No prescaler
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC0;	//Attach clock 0 to TTC0

	TCC0->CTRLA.reg = 0;						//Disable register to set it up
	TCC0->PER.reg = 6000;						//Set max value
	TCC0->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;		//Normal waveform generation (for stepper/DC)
	TCC0->WEXCTRL.reg = TCC_WEXCTRL_OTMX(0);
	//TCC0->CTRLA.reg = TCC_CTRLA_ENABLE;			//Enable divide by 16	
	TCC0->CTRLA.reg = TCC_CTRLA_PRESCALER_DIV16 | TCC_CTRLA_PRESCSYNC_PRESC; // | TCC_CTRLA_ENABLE;			//Enable divide by 16
	
	PORT_WRCONFIG_Type	portConfig;	
	portConfig.reg = 0;
	portConfig.bit.WRPINCFG = 1;
	portConfig.bit.WRPMUX = 1;
	portConfig.bit.PMUX = MUX_PA05F_TCC0_WO1;
	portConfig.bit.INEN = 1;
	portConfig.bit.PMUXEN = 1;
	portConfig.bit.PINMASK = PORT_PA05;
	portConfig.bit.HWSEL = 0;	// lower 16 bits
	PORT->Group[0].WRCONFIG.reg = portConfig.reg;

	PORT->Group[0].DIRSET.reg = PORT_PA05;										// Assign the pin as OUTPUT
	
	TCC0->CC[1].reg = 0;														//Start OFF

	// Configure SysTick to trigger an ISR every 100us using a 48MHz CPU Clock------------------------------------------------------------------
	SysTick->CTRL = 0;							// Disable SysTick
	SysTick->LOAD = 48000UL; //47999UL;			// Set reload register for 1000us interrupts (1KHz)
	NVIC_SetPriority(SysTick_IRQn, 0);			// Set interrupt priority to high urgency //NVIC_SetPriority(SysTick_IRQn, 3);	// Set interrupt priority to least urgency
	SysTick->VAL = 0;							// Reset the SysTick counter value
	SysTick->CTRL = 0x00000007;					// Enable SysTick, Enable SysTick Exceptions, Use CPU Clock
	NVIC_EnableIRQ(SysTick_IRQn);				// Enable SysTick Interrupt


	REG_PM_APBCMASK |= PM_APBCMASK_ADC;														//Enable ADC Clock
	REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_ADC;	//Drive it with generic clock 3

	REG_ADC_REFCTRL = ADC_REFCTRL_REFSEL_AREFA;								// Select reference voltage (external 3.3V)
	REG_ADC_AVGCTRL |= ADC_AVGCTRL_SAMPLENUM_1;								// Average control 1 sample, no right-shift
	REG_ADC_SAMPCTRL = ADC_SAMPCTRL_SAMPLEN(0);								// Sampling time, no extra sampling half clock-cycles
	
	while (REG_ADC_STATUS & ADC_STATUS_SYNCBUSY);							// Wait for synchronization
	ADC->CTRLB.reg |= ADC_CTRLB_RESSEL_12BIT | ADC_CTRLB_PRESCALER_DIV4;	//Set divide factor from input clock
	while (REG_ADC_STATUS & ADC_STATUS_SYNCBUSY);							// Wait for synchronization
	ADC->CTRLA.reg |= ADC_CTRLA_ENABLE;										//Enable ADC
	while (ADC->STATUS.bit.SYNCBUSY);										// Wait for synchronization

	gameSetup();					//One-time user game setup code

	systemLoop();					//Jump to the main loop in the code

}




