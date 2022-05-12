/*
 * lcd.h
 *
 * Created: 7/31/2021 6:22:10 PM
 *  Author: ben
 */ 

#ifndef GAMEBADGE_H_
#define GAMEBADGE_H_

#define SSD1306_LCDWIDTH				128
#define SSD1306_LCDHEIGHT				64
#define SSD1306_SETCONTRAST				0x81
#define SSD1306_DISPLAYALLON_RESUME		0xA4
#define SSD1306_DISPLAYALLON			0xA5
#define SSD1306_NORMALDISPLAY			0xA6
#define SSD1306_INVERTDISPLAY			0xA7
#define SSD1306_DISPLAYOFF				0xAE
#define SSD1306_DISPLAYON				0xAF
#define SSD1306_SETDISPLAYOFFSET		0xD3
#define SSD1306_SETCOMPINS				0xDA
#define SSD1306_SETVCOMDETECT			0xDB
#define SSD1306_SETDISPLAYCLOCKDIV		0xD5
#define SSD1306_SETPRECHARGE			0xD9
#define SSD1306_SETMULTIPLEX			0xA8
#define SSD1306_SETLOWCOLUMN			0x00
#define SSD1306_SETHIGHCOLUMN			0x10
#define SSD1306_SETSTARTLINE			0x40
#define SSD1306_MEMORYMODE				0x20
#define SSD1306_COLUMNADDR				0x21
#define SSD1306_PAGEADDR				0x22
#define SSD1306_COMSCANINC				0xC0
#define SSD1306_COMSCANDEC				0xC8
#define SSD1306_SEGREMAP				0xA0
#define SSD1306_CHARGEPUMP				0x8D
#define SSD1306_EXTERNALVCC				0x1
#define SSD1306_SWITCHCAPVCC			0x2

#define dUp			0x80		//Shift register MSB
#define dDown		0x40
#define dLeft		0x20
#define dRight		0x10		//-----------------------------
#define dB			0x08
#define dA			0x04		//------------
#define dCent		0x02		//------------
#define dExt		0x01		//Shift register LSB
#define dMenu		0x01		//Port A top 3 bits

#define horizontal		0x00		//Game scrolls horizontally
#define vertical		0x01		//Game scrolls vertically
#define mirrorSprite	-1				//Mirror the sprite
#define normalSprite	1				//Draw sprite normally

#define blank	0					//Used for single row tile drawing "clear" param. 0 = blank
#define draw	0xFF				//0xFF = draw pixels

void displayInit();
void tileLoad(uint16_t dataStart, uint8_t *data, uint16_t dataSize);
void screenLoad();
uint8_t dmaCheck();
void rowLoad(const char *tileData, uint8_t *rowRAM, uint8_t clear);
uint8_t getButtons();
void drawTiles(const char *tileData, uint8_t *tileMap);
void setWindow(uint8_t x, uint8_t y);
void setRowScroll(uint8_t x, uint8_t row);
void toneLogic();
void tone(uint16_t thePitch, uint16_t theTime, uint8_t thisPriority, int direction);
void ledState(uint8_t theState);
void addSeed(uint8_t theValue);
uint8_t getRandom(uint8_t seed);
void drawSprite(const char *bitmap, int8_t xPos, int8_t yPos, uint8_t frameNumber, int8_t mirror);
void setScrollDirection(uint8_t mirrorType);
void bufferMask(int8_t xPos, int16_t bufferPos, uint8_t data, uint8_t maskType);
void direct2buffer(const char *source, uint16_t dest, uint16_t length);
void displayOnOff(uint8_t whatState);
int setRotate(uint8_t which);
void setInvert(uint8_t state);
void writecommand(uint8_t c);
void writedata(uint8_t c);
void dcHigh();
void dcLow();
void csHigh();
void csLow();
void delayArgMS(double __ms);

#endif
