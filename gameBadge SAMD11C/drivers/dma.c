#include "sam.h"
#include "dma.h"
#include <string.h>

volatile dmacdescriptor wrb[6] __attribute__ ((aligned (16)));
dmacdescriptor descriptor_section[6] __attribute__ ((aligned (16)));
dmacdescriptor descriptor_chain[6] __attribute__ ((aligned (16)));
dmacdescriptor descriptor __attribute__ ((aligned (16)));

static uint32_t chnltx = 0;							// DMA channels
volatile uint32_t dmadone;

Sercom *sercomDMA0 = (Sercom   *)SERCOM0;  // SPI SERCOM0 used for OLED buffer

void dma_init(void *bufferStart) {

	bufferStart++;

	PM->AHBMASK.reg |= PM_AHBMASK_DMAC;
	PM->APBBMASK.reg |= PM_APBBMASK_DMAC;
	
	DMAC->BASEADDR.reg = (uint32_t)descriptor_section;
	DMAC->WRBADDR.reg = (uint32_t)wrb;
	DMAC->CTRL.reg = DMAC_CTRL_DMAENABLE | DMAC_CTRL_LVLEN(0xf);
	
	//Set up RGB transmit bank 0--------------------------------------------------------------------------------------------------------------------------------------	
	chnltx = 0;																//DMA channel ID
	
	DMAC->CHID.reg = DMAC_CHID_ID(chnltx);									//Set channel ID number that we want to modify
	DMAC->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;								//Disable channel so we can set things up
	DMAC->CHCTRLA.reg = DMAC_CHCTRLA_SWRST;									//Reset all registers to initial state - do we need to do this?	
	DMAC->SWTRIGCTRL.reg &= (uint32_t)(~(1 << chnltx));						//Set bit to enable software trigger of this DMA transfer start
	DMAC->CHCTRLB.reg = DMAC_CHCTRLB_LVL(0) | DMAC_CHCTRLB_TRIGSRC(SERCOM0_DMAC_ID_TX) | DMAC_CHCTRLB_TRIGACT_BEAT; //Copy contents of CHCTRLB and OR in some things.
	DMAC->CHINTENSET.reg = DMAC_CHINTENSET_MASK ;							//Enable all 3 interrupts Channel Suspend, Channel Transfer Complete and Channel Transfer Error

	//Setup transfer of 1024 bytes
	descriptor.descaddr = 0;												//There is only 1 DMA, so set this to 0 so it doesn't cascade to the next memory section
	descriptor.dstaddr = (uint32_t) &sercomDMA0->SPI.DATA.reg;				//DMA destination is the SPI bus (SERCOM0)
	descriptor.btcnt =  1023;												//Number of beats to transfer (1024)
	descriptor.srcaddr = (uint32_t)bufferStart;								//Where the DMA gets data from
	descriptor.btctrl =  DMAC_BTCTRL_VALID | DMAC_BTCTRL_SRCINC;			//Set this descriptor as valid and set DMA to increment the source address to scan across all RGB data
	descriptor.srcaddr += 1023;												//For some reason we have to set this to the end of data bank
	
	memcpy(&descriptor_section[chnltx], &descriptor, sizeof(dmacdescriptor));	//Copy values over into the actual RAM section that the DMA looks at. We only need to do this once because we're writing back to a separate area of RAM

}

void dmaStartTransfer() {

	DMAC->CHID.reg = DMAC_CHID_ID(0);			//Select channel
	DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;	//Start transfer						
	
}
