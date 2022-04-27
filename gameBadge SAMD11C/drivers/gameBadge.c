#include "sam.h"
#include "gameBadge.h"
#include "spi.h"
#include "dma.h"

uint8_t buffer[1024];

uint8_t scrollDirection = horizontal; //vertical_mirror; //

uint8_t winX[8];
uint8_t winXfine[8];
uint8_t winY = 0;
uint8_t winYfine = 0;

uint16_t tonePitch;
uint16_t toneTimer = 0;
uint8_t tonePriority = 0;		//10 = highest (sfx) 0 = lowest (music)
int toneDir = 0;

uint8_t displaySleep = 0;
uint16_t seed;

uint8_t rotate180 = 0;
uint8_t inverted = 0;

uint8_t buttons;

void displayInit() {

	csLow();
	PORT->Group[0].OUTCLR.reg = PORT_PA09;										//OLED reset LOW
	PORT->Group[0].OUTSET.reg = PORT_PA09;										//OLED reset HIGH		
	csHigh();

    // Init sequence for 128x64 OLED module
    writecommand(SSD1306_DISPLAYOFF);                    // 0xAE
	
    writecommand(SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
    writecommand(0x80);									// the suggested ratio 0x80
    
    writecommand(SSD1306_SETMULTIPLEX);                  // 0xA8
    writecommand(0x3F);
    
    writecommand(SSD1306_SETDISPLAYOFFSET);              // 0xD3
    writecommand(0x0);                                   // no offset
    
    writecommand(SSD1306_SETSTARTLINE);// | 0x0);        // line #0
    
    writecommand(SSD1306_CHARGEPUMP);                    // 0x8D
    writecommand(0x14);  // using internal VCC
    
    writecommand(SSD1306_MEMORYMODE);                    // 0x20
    //writecommand(0x01);									// 0x00 vertical addressing
    writecommand(0x00);									// 0x00 horizontal addressing

	setRotate(rotate180);								//Set as normal rotate to start
    
    writecommand(SSD1306_SETCOMPINS);                    // 0xDA
    writecommand(0x12);
    
    writecommand(SSD1306_SETCONTRAST);                   // 0x81
    writecommand(0x8F);
    
    writecommand(SSD1306_SETPRECHARGE);                  // 0xd9
    writecommand(0xF1);
    
    writecommand(SSD1306_SETVCOMDETECT);                 // 0xDB
    writecommand(0x40);
    
    writecommand(SSD1306_DISPLAYALLON_RESUME);           // 0xA4
 
	setInvert(inverted);
     
    writecommand(SSD1306_DISPLAYON);                     //switch on OLED

	dma_init(buffer);					//Setup DMA transfer

}

void screenLoad() {						//This should be called beginning of each frame. Takes just under 1ms

	PORT->Group[0].OUTCLR.reg = PORT_PA08;		//Load shift register
	PORT->Group[0].OUTSET.reg = PORT_PA08;

	buttons = spiSendIO(buffer[0]);				//Send out the first byte of the OLED and get back the controller shift register value		

	dmaStartTransfer();							//Now DMA out the remaining 1023 bytes to OLED (Hackalicious)

}

void rowLoad(const char *tileData, uint8_t *rowRAM, uint8_t clear) {		//Loads a row of 16 tiles directly into the OLED. Useful for status screens/pause displays without destroying the main buffer
	
	for (int col = 0 ; col < 16 ; col++) {
		uint16_t tilePointer = rowRAM[col] * 8;								//Get the tile and * 8 to create a pointer
		
		for (uint8_t colB = 0 ; colB < 8 ; colB++) {						//Draw the 8 pixel wide tile
			
			//SPI0.DATA = pgm_read_byte(tileData + tilePointer++) & clear;	//Send the column of pixels directly to OLED. Clear 0 = blank Clear 0xFF = show pixels
			//while(!(SPI0.INTFLAGS & SPI_DREIF_bm)) {}
			
		}
		
	}
	
}

uint8_t getButtons() {
	
	//uint8_t buttons = PORTC_IN & 0x0F;					//Grab lower nibble of C (d-pad)
	//buttons |= PORTA_IN & 0xE0;							//OR in upper 3 bits of Port A

	return buttons;									//Return inverse for easy logic
	
}

void toneLogic() {			//Called at 1KHz

	if (toneDir) {
		//TCA0.SINGLE.CMP0 += toneDir;
	}

	if (toneTimer) {
		if (--toneTimer == 0) {
			//TCA0.SINGLE.CTRLA &= 0xFE;				//Mask out enable bit (sound off)
			tonePriority = 0;						//Priority set to 0 (anything can now play)
		}
	}
	
}

void tone(uint16_t thePitch, uint16_t theTime, uint8_t thisPriority, int direction) {

	if (thisPriority < tonePriority) {					//A "more important" sound is playing? (some sounds are more equal than others)
		return;											//Abort
	}

	toneDir = direction;
	tonePriority = thisPriority;						//Playing? Set as new priority
	
	//TCA0.SINGLE.CMP0 = thePitch;
	//TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; // Start
	
	toneTimer = theTime;
	
}

void ledState(uint8_t theState) {
	
	if (theState) {
		//PORTB_OUTSET = 0x10;
	}
	else {
		//PORTB_OUTCLR = 0x10;
	}
	
}

uint8_t getRandom(uint8_t mask) {
	
	//ADC0.COMMAND = 0x01;					//Start ADC0 (the bare pads)
	//while(!ADC0.INTFLAGS) {}				//Wait for it
	//return (ADC0.RES + seed) & mask;		//Add the SEED value from start of frame and return the masked value
	
}

void drawTiles(const char *tileData, uint8_t *tileMapPointer) {

	uint16_t bufPoint = 0;

	csLow();

	if (scrollDirection) {					//Vertical?
		
		uint8_t winYfineINV = 8 - winYfine;				//Get remainder of Y fine scrolling to OR in next line of pixels
		uint8_t coarseY = winY * 16;                           //Get the course Y value for top line of visible screen
		
		for (uint8_t row = 0 ; row < 8 ; row++) {               //Draw the 8 tile row high display

			uint8_t finePointer = winXfine[row];						//Copy the fine scrolling amount so we can use it as a byte pointer when scanning in graphics
			uint8_t coarseXpointerU = (tileMapPointer[coarseY] * 8) + finePointer;
			uint16_t coarseXpointerL; // = (tileMapPointer[coarseY + 16] * 8) + finePointer;
			if (coarseY == 240) {
				coarseXpointerL = (tileMapPointer[0] * 8) + finePointer;
			}
			else {
				coarseXpointerL = (tileMapPointer[coarseY + 16] * 8) + finePointer;
			}
			
			for (uint8_t colB = 0 ; colB < 128 ; colB++) {         //Draw 16 column wide display (8 pixels / 1 char at a time)

				uint8_t upper = tileData[coarseXpointerU++] >> winYfine;
				uint8_t lower = tileData[coarseXpointerL++] << winYfineINV;
				
				buffer[bufPoint++] = upper | lower;
				
				if (++finePointer == 8) {								//Done drawing this tile?
					finePointer = 0;                                //Reset fine scroll counter
					coarseXpointerU = tileMapPointer[coarseY] * 8;		//Get new tile pointer from memory
					if (coarseY == 240) {
						coarseXpointerL = tileMapPointer[0] * 8;
					}
					else {
						coarseXpointerL = tileMapPointer[coarseY + 16] * 8;
					}
				}
			}

			coarseY += 16;                                            //Increment coarse pointer to next row
			
		}
	}
	else {								//Horizontal?
		
		uint8_t coarseY = 0;                           //Get the course Y value for top line of visible screen
		
		for (uint8_t row = 0 ; row < 8 ; row++) {               //Draw the 8 tile row high display

			uint8_t finePointer = winXfine[row];						//Copy the fine scrolling amount so we can use it as a byte pointer when scanning in graphics
			uint8_t coarseX = winX[row];								//Find the current coarseX position for this line
			uint16_t coarseXpointer = (tileMapPointer[coarseX + coarseY] * 8) + finePointer;

			for (uint8_t colB = 0 ; colB < 128 ; colB++) {         //Draw 16 column wide display (8 pixels / 1 char at a time)

				buffer[bufPoint++] = tileData[coarseXpointer++];
				
				if (++finePointer == 8) {									//Done drawing this tile?
					finePointer = 0;										//Reset fine scroll counter
					if (++coarseX > 31) {
						coarseX = 0;
					}
					coarseXpointer = tileMapPointer[coarseX + coarseY] * 8;		//Get new tile pointer from memory
				}
				
			}

			coarseY += 32;                                            //Increment coarse pointer to next row
			
		}
	}

	csHigh();

}

void setWindow(uint8_t x, uint8_t y) {

	for (int row = 0 ; row < 8 ; row++) {
		winX[row] = x >> 3;
		winXfine[row] = x & 0x07;
		winY = y >> 3;
		winYfine = y & 0x07;
	}
	
}

void setRowScroll(uint8_t x, uint8_t row) {

	winX[row] = x >> 3;
	winXfine[row] = x & 0x07;

}

void drawSprite(const char *bitmap, int8_t xPos, int8_t yPos, uint8_t frameNumber, int8_t mirror) {

	uint8_t xSize = 0; //pgm_read_byte(bitmap++);				//Get bitmap width from flash
	uint8_t ySize = 0; //pgm_read_byte(bitmap++) >> 3;			//Get bitmap height from flash, convert to bytes (8 pixels per byte)

	uint16_t sizeInBytes = xSize * ySize;					//Get total sprite size

	uint8_t offset = yPos & 0x07;
	uint8_t offsetInv = 8 - offset;
	
	int16_t bPointer = ((yPos >> 3) * 128) + xPos;			//Where the screen buffer pointer starts

	bitmap += frameNumber * (sizeInBytes * 2);				//Advance to frame pointer in sprite char memory

	if (mirror == -1) {										//If mirror draw sprites from right to left
		bitmap += xSize - 1;
	}
	
	uint8_t yOffset = 0;
	
	if (offset) {											//If sprite is on a byte boundry we can save a lot of time, so set flag if so
		yOffset = 1;
	}

	for (int y = 0 ; y < (ySize + yOffset) ; y++) {			//Do all rows of a sprite+1 if offset (since lowest row will go past a byte boundry) or if no offset, just do ySize rows

		int8_t xTemp = xPos;

		for (int16_t x = bPointer ; x < xSize + bPointer ; x++) {					//Select column

			if (!(x & 0xFC00) && !(xTemp & 0x80)) {									//Don't fill bytes outside of the screen buffer or past the left and right edges of the screen (xTemp)
				
				uint8_t buildMask = 0;
				uint8_t buildPixels = 0;
				
				if (offset) {														//Get offset pixels from adjacent rows
					if (y) {
						buildMask = pgm_read_byte(bitmap - xSize) >> offsetInv;
						buildPixels = pgm_read_byte((bitmap + sizeInBytes) - xSize) >> offsetInv;
					}
					if (y < ySize) {
						buildMask |= pgm_read_byte(bitmap) << offset;
						buildPixels |= pgm_read_byte(bitmap + sizeInBytes) << offset;
					}
				}
				else {																//Falls on byte boundry - simple and fast!
					buildMask = pgm_read_byte(bitmap);
					buildPixels = pgm_read_byte(bitmap + sizeInBytes);
				}
				
				buffer[x] &= ~buildMask;											//AND in mask
				buffer[x] |= buildPixels;											//OR in pixels
			}

			bitmap += mirror;															//Advance bitmap pointer and xTemp counter
			xTemp++;
			
		}
		
		bPointer += 128;														//Increment buffer line buffer by 1 byte-boundary line

		if (mirror == -1) {														//Since mirrored = going backwards in memory we have to skip 2 row widths to get to the right-hand start of the next one
			bitmap += (xSize * 2); // - 1;
		}

	}

}

void setScrollDirection(uint8_t mirrorType) {

	scrollDirection = mirrorType;
	
}

void displayOnOff(uint8_t whatState) {
	
	//SPI0_CTRLB = 0;											//Disable SPI buffer mode since we are sending single byte commands

	if (whatState) {
		displayInit();
		displaySleep = 0;									//Reset OLED to wake it back up
	}
	else {
		writecommand(SSD1306_DISPLAYOFF);                   //Sleep the OLED
		displaySleep = 1;
	}
	
	//SPI0_CTRLB = SPI_BUFEN_bm;								//Resume SPI buffer mode
	
}


void direct2buffer(const char *source, uint16_t dest, uint16_t length) {

	uint16_t pointer = 0;

	while(length--) {
		buffer[dest++] = source[pointer++];
	}
	
}

int setRotate(uint8_t which) {

	if (which == 0xFF) {							//Toggle flag? user mode
		rotate180 = !rotate180 & 1;					//Invert rotation	
	}
	else {
		rotate180 = which;							//0 or 1? Restore last rotation (such as from a wake)
	}

	if (rotate180) {
		writecommand(SSD1306_SEGREMAP);				// rotate screen 180
		writecommand(SSD1306_COMSCANINC);			// rotate screen 180		
	}
	else {
		writecommand(SSD1306_SEGREMAP | 0x01);		//Normal
		writecommand(SSD1306_COMSCANDEC);	
	}
	
	return rotate180;
	
}

void setInvert(uint8_t state) {

	if (state == 0xFF) {							//Toggle flag? user mode
		inverted = !inverted & 1;					//Invert... inversion
	}
	else {
		inverted = state;							//0 or 1? Restore last invert state (such as from a wake)
	}

	if (inverted) {
		writecommand(SSD1306_INVERTDISPLAY);                 // 0xA7	
	}	
	else {
		writecommand(SSD1306_NORMALDISPLAY);                 // 0xA6		    
	}
	
}

void writecommand(uint8_t c) {

	dcLow();
	csLow();
	spiSendIO(c);
	csHigh();
	dcHigh();
		
}

void writedata(uint8_t c) {

	dcHigh();
	csLow();
	spiSendIO(c);
	csHigh();

}

void dcHigh() {

	PORT->Group[0].OUTSET.reg = PORT_PA02;										//D/C HIGH (data, default)
	
}

void dcLow() {

	PORT->Group[0].OUTCLR.reg = PORT_PA02;										//D/C LOW (command)
		
}

void csHigh() {

	//PORT->Group[0].OUTSET.reg = PORT_PA08;										//Normally HIGH (not reset)	
	
}

void csLow() {

	//PORT->Group[0].OUTCLR.reg = PORT_PA08;										//Normally HIGH (not reset)	
	
}

