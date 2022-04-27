#ifndef DMA_H_
#define DMA_H_

	typedef struct {
		uint16_t btctrl;
		uint16_t btcnt;
		uint32_t srcaddr;
		uint32_t dstaddr;
		uint32_t descaddr;
	} dmacdescriptor ;

	void dma_init(void *bufferStart);
	void dmaStartTransfer();

#endif /* DMA_H_ */