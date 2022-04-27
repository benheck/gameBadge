#include "sam.h"
#include "clocks.h"

void clocksInit48Mhz() {

	//Remove default clock divider on OSC8M
	SYSCTRL->OSC8M.bit.PRESC = SYSCTRL_OSC8M_PRESC_0_Val;
	SYSCTRL->OSC8M.bit.ONDEMAND = 0;

	/* configuration of the NVM CTRLB register and set the flash wait states */
	NVMCTRL->CTRLB.bit.RWS = 1;

	/* Put OSC8M as source for Generic Clock Generator 1  and divide by 250 (32KHz)*/
	GCLK->GENDIV.reg = GCLK_GENDIV_ID(1) | GCLK_GENDIV_DIV(250); // Generic Clock Generator 3

	/* Write Generic Clock Generator 1 configuration */
	GCLK->GENCTRL.reg = GCLK_GENDIV_ID(1) |			// Generic Clock Generator 1
	GCLK_GENCTRL_SRC_OSC8M |						//Sourced from OSC8M
	GCLK_GENCTRL_IDC |
	GCLK_GENCTRL_GENEN ;		

	while (GCLK->STATUS.bit.SYNCBUSY);				// wait for synchronization to complete

	//Attach Generic clock 1 to DFLL	
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(0) |		// DFLL48MReference
	GCLK_CLKCTRL_GEN_GCLK1 |						// Generic Clock Generator 1 is source
	GCLK_CLKCTRL_CLKEN ;
	
	/* Workaround for errata 9905 */
	SYSCTRL->DFLLCTRL.bit.ONDEMAND = 0;
	
	/* wait for the DFLL clock to be ready */
	while (SYSCTRL->PCLKSR.bit.DFLLRDY == 0);

	SYSCTRL->DFLLCTRL.reg = SYSCTRL_DFLLCTRL_MODE |				//Set DFLL control
	SYSCTRL_DFLLCTRL_QLDIS;
		
	while (SYSCTRL->PCLKSR.bit.DFLLRDY == 0);					//wait for the DFLL clock to be ready

	/* get the coarse and fine values stored in NVM */
	uint32_t coarse = (*(uint32_t *)(0x806024) >> 26);
	uint32_t fine = (*(uint32_t *)(0x806028) & 0x3FF);

	SYSCTRL_DFLLVAL_Type dfllval = {
		.bit.COARSE = coarse,
		.bit.FINE = fine,
	};
	SYSCTRL->DFLLVAL.reg = dfllval.reg;

	SYSCTRL_DFLLMUL_Type dfllmul = {
		.bit.MUL = 1500, /* 32KHz * 1500 = 48MHz */
		.bit.CSTEP = (coarse >> 1), /* must be 50% or less of coarse value */
		.bit.FSTEP = (fine >> 1), /* must be 50% or less of fine value */
	};
	SYSCTRL->DFLLMUL.reg = dfllmul.reg;

	/* enable DFLL */
	SYSCTRL->DFLLCTRL.bit.ENABLE = 1;
	
	while (SYSCTRL->PCLKSR.bit.DFLLLCKF == 0);				//wait for DFLL closed loop lock

	/* Configure GCLK 0 (CPU clock) to run from the DFLL */
	GCLK->GENDIV.reg = GCLK_GENDIV_ID(0);
	GCLK->GENCTRL.reg = GCLK_GENDIV_ID(0) | // Generic Clock Generator 0
	GCLK_GENCTRL_SRC_DFLL48M |
	GCLK_GENCTRL_IDC |
	GCLK_GENCTRL_GENEN;
	
	while (GCLK->STATUS.bit.SYNCBUSY);						//wait for synchronization to complete

}
