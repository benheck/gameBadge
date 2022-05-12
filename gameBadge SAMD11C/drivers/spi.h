//Defines for the SPI SERCOM peripherals

#ifndef SPI_H_
#define SPI_H_

	#define SPI_CLK_FREQ				48000000			//Base frequency of the CPU bus attached to SPI devices	

	#define SPI_BAUD					10000000			//10MHz for serial to OLED. Max speed to race ahead of the parallel tile driver

	void SPI_init(void);
	uint8_t spiSendIO(uint8_t data);

#endif /* SPI_H_ */