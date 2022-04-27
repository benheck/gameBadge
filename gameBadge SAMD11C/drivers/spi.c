#include "sam.h"
#include "spi.h"

void SPI_init(void) {
		
	//Enable SPI SERCOM clocks
	PM->APBCMASK.bit.SERCOM0_ = 1;				//SERCOM0 go!
	
	//Attach clocks
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_ID_SERCOM0_CORE;			//Use main 48MHz PLL clock
	while(GCLK->STATUS.bit.SYNCBUSY);	
	
	SERCOM_SPI_CTRLA_Type ctrla;				//Create structure then setup basic SPI parameters
	ctrla.bit.DORD = 0;							// MSB first
	ctrla.bit.CPHA = 0;							// Mode 0 clock phase
	ctrla.bit.CPOL = 0;							// Clock polarity low when idle
	ctrla.bit.FORM = 0;							// SPI frame
	ctrla.bit.DIPO = 2;							// MISO on PAD[2]
	ctrla.bit.DOPO = 0;							// MOSI on PAD[0], SCK on PAD[1]
	ctrla.bit.MODE = 3;							// Master mode (slave is 2)

	SERCOM0->SPI.CTRLA.reg = ctrla.reg;			//Copy values over to SERCOM0 CTRLA
	
	SERCOM_SPI_CTRLB_Type ctrlb;
	ctrlb.bit.RXEN = 1;							// RX enabled
	ctrlb.bit.CHSIZE = 0;						// 8-bit character size
	ctrlb.bit.MSSEN = 0;						// Disable the Chip Select line for the other SPI buses (we'll do it manually)
	SERCOM0->SPI.CTRLB.reg = ctrlb.reg;			//Copy modified values over to SERCOM0

	SERCOM0->SPI.BAUD.reg = (SPI_CLK_FREQ / (2 * SPI_BAUD)) - 1;	//Calculate BAUD value based off input clock and desired output

	PORT_WRCONFIG_Type wrconfig;				//Create config structure
	
	//Setup SERCOM0 on Peripheral Function C
	wrconfig.bit.HWSEL = 0;							//0 = Change the lower 16 pins of the PORT group
	wrconfig.bit.WRPINCFG = 1;						//Flag that we want to update pins
	wrconfig.bit.WRPMUX = 1;						//Flag that we want to update the Peripheral Multiplexing register
	wrconfig.bit.PMUXEN = 1;						//Enable pin multiplexing	
	wrconfig.bit.PMUX = MUX_PA14C_SERCOM0_PAD0;		//Set pin multiplexing type	(SERCOM0 Pad0 on PA14)
	wrconfig.bit.PINMASK = (uint32_t)(PORT_PA14 | PORT_PA15 | PORT_PA04);		//No need to bitshift since it's on the lower word

	PORT->Group[0].WRCONFIG.reg = wrconfig.reg;		//Send changes to SERCOM0 on first port (0=A)
	SERCOM0->SPI.CTRLA.bit.ENABLE = 1;				//Enable the SPI bus 1
	while(SERCOM0->SPI.SYNCBUSY.bit.ENABLE);		//Wait for it to be ready	
	
}

uint8_t spiSendIO(uint8_t data) {
	
	while(SERCOM0->SPI.INTFLAG.bit.DRE == 0);	
	SERCOM0->SPI.DATA.reg = data;	
	while(SERCOM0->SPI.INTFLAG.bit.RXC == 0);	
	return SERCOM0->SPI.DATA.reg;
	
}
