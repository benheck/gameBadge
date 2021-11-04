#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

//#define F_CPU 20000000UL
//#include <util/delay.h>
#include "drivers/gameBadge.h"
#include "game.h"
#include <string.h>

uint8_t tileMap[256];							//Tile map ram (2 screens of 16x8 tiles)
static volatile uint8_t sleepState = 0;			//0 = normal 1 = asleep
static volatile uint8_t frameFlag;				//ms Frame counter to decide when a frame should be drawn

static volatile uint8_t secFlag = 0;			//This flag is set when a seconds transition occurs
//static volatile uint8_t day = 0;				//0 = Sunday... 7 - Saturday
//static volatile uint8_t amPM = 0;				//am = 0 pm = 1
static volatile uint8_t hours = 6;				//The hours
static volatile uint8_t minutes = 30;			//The minutes
static volatile uint8_t seconds = 0;			//The seconds

//Your flash game data here--------------------------------------------------------------------------------

#define maxEnemies			16

const char PROGMEM tileData[] = {
0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0x13, 0x0b, 0x05, 0x02, 0x00, 0x00, 0x01, 0x03, 0x01, 0x00, 0x04, 0x00, 0x01, 0x01, 0x01, 0x12, 0x02, 0x02, 0x04, 0x04, 0x04, 0x48, 0x08, 0x08, 0x88, 0x90, 0x90, 0xb0, 0xb0, 0x98, 0x98, 0x98, 0x58, 0x4c, 0x2c, 0x26, 0x16, 0x13, 0x03, 0x07, 0x07, 0x03, 0x06, 0x0a, 0x02, 0x02, 0x14, 0x04, 0x44, 0x4c, 0x4c, 0x2c, 0x26, 0x16, 0x13, 0x0b, 0x09, 0x05, 0x04, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x09, 0x01, 0x02, 0x06, 0x16, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x13, 0x13, 0x0b, 0x09, 0x09, 0x09, 0x09, 0x05, 0x04, 0x04, 0x04, 0x04, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0xc0, 0x80, 0x80, 0xc0, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xc0, 0xc0, 0x60, 0x30, 0x70, 0x78, 0x30, 0x30, 0xa0, 0x20, 0x40, 0x40, 0x80, 0x80, 0x80, 0x80, 0x80, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x60, 0x60, 0x60, 0x60, 0x30, 0x30, 0x30, 0xb0, 0x98, 0x58, 0x4c, 0x4c, 0x0c, 0x0e, 0x0e, 0x0c, 0x08, 0x28, 0x08, 0x08, 0x08, 0x50, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x40, 0x40, 0x80, 0x80, 0x00, 0x00, 0x40, 0x50, 0xb8, 0x2c, 0x7c, 0xfe, 0xfe, 0xf7, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe, 0xfc, 0xfc, 0xf8, 0xf0, 0xc0, 0x00, 0x00, 0x01, 0x00, 0x15, 0x02, 0x49, 0x12, 0x0a, 0x0a, 0x24, 0x15, 0x15, 0x09, 0x0a, 0x23, 0x0b, 0x8b, 0x0b, 0x23, 0x0a, 0x23, 0x0b, 0x23, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xde, 0xde, 0xde, 0x00, 0x00, 0x00, 0x1e, 0x1e, 0x00, 0x00, 0x1e, 0x1e, 0x00, 0x00, 0x6c, 0xfe, 0xfe, 0x6c, 0xfe, 0xfe, 0x6c, 0x00, 0x48, 0x5c, 0x54, 0xfe, 0x54, 0x74, 0x24, 0x30, 0x72, 0xbc, 0xb0, 0xfc, 0x30, 0xfc, 0x32, 0x00, 0x60, 0xf4, 0x9e, 0xba, 0x6e, 0xe4, 0xa0, 0x30, 0x70, 0xbc, 0xb2, 0xf8, 0x32, 0xfc, 0x30, 0x00, 0x00, 0x00, 0x7c, 0xfe, 0xc6, 0x82, 0x00, 0x00, 0x00, 0x82, 0xc6, 0xfe, 0x7c, 0x00, 0x00, 0x10, 0x54, 0x7c, 0x38, 0x38, 0x7c, 0x54, 0x10, 0x00, 0x18, 0x18, 0x7e, 0x7e, 0x18, 0x18, 0x00, 0x00, 0x00, 0x80, 0xe0, 0xe0, 0x60, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x04, 0x00, 0x00, 0x7c, 0xfe, 0xfe, 0xfe, 0x82, 0xfe, 0x7c, 0x00, 0x00, 0x0c, 0xfc, 0xfe, 0xfe, 0xfe, 0x00, 0x00, 0xc4, 0xe6, 0xf2, 0xde, 0xde, 0xde, 0xcc, 0x00, 0x44, 0xd6, 0x92, 0xfe, 0xfe, 0xfe, 0x7c, 0x00, 0x3e, 0x3e, 0x30, 0x3e, 0xfe, 0xfe, 0x30, 0x00, 0x5e, 0xde, 0xda, 0xda, 0xda, 0xfa, 0x72, 0x00, 0x7c, 0xfe, 0x8a, 0xfa, 0xfa, 0xfa, 0x72, 0x00, 0x02, 0x02, 0xf2, 0xfa, 0xfe, 0x0e, 0x06, 0x00, 0x6c, 0xfe, 0x92, 0xfe, 0xfe, 0xfe, 0x6c, 0x00, 0x1c, 0xbe, 0xa2, 0xfe, 0xfe, 0xfe, 0x7c, 0x00, 0x00, 0x00, 0x6c, 0x6c, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x80, 0xec, 0xec, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x10, 0x38, 0x7c, 0xfe, 0x00, 0x00, 0x00, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x00, 0x00, 0x00, 0xfe, 0x7c, 0x38, 0x10, 0x00, 0x00, 0x00, 0x0c, 0x06, 0xd2, 0xda, 0xde, 0x0e, 0x04, 0x00, 0x7c, 0xfe, 0xc6, 0xba, 0xaa, 0xb6, 0xbc, 0x00, 0xfc, 0xfe, 0x12, 0xfe, 0xfe, 0xfe, 0xfc, 0x00, 0xfe, 0xfe, 0xfe, 0xfe, 0x92, 0xfe, 0x6c, 0x00, 0x7c, 0xfe, 0xfe, 0xfe, 0x82, 0xc6, 0x44, 0x00, 0xfe, 0xfe, 0xfe, 0xfe, 0x82, 0xfe, 0x7c, 0x00, 0xfe, 0xfe, 0xfe, 0xfe, 0x92, 0x92, 0x82, 0x00, 0xfe, 0xfe, 0xfe, 0xfe, 0x12, 0x12, 0x02, 0x00, 0x7c, 0xfe, 0xfe, 0xfe, 0x82, 0xa6, 0xe4, 0x00, 0xfe, 0xfe, 0x10, 0xfe, 0xfe, 0xfe, 0xfe, 0x00, 0x82, 0xfe, 0xfe, 0xfe, 0xfe, 0x82, 0x00, 0x00, 0x40, 0xc2, 0x82, 0xfe, 0xfe, 0x7e, 0x02, 0x00, 0xfe, 0xfe, 0xfe, 0xfe, 0x38, 0xee, 0xc6, 0x00, 0xfe, 0xfe, 0xfe, 0xfe, 0x80, 0x80, 0x80, 0x00, 0xfe, 0xfe, 0x1e, 0xfc, 0x1e, 0xfe, 0xfe, 0x00, 0xfe, 0xfe, 0xfe, 0x1c, 0x38, 0xfe, 0xfe, 0x00, 0x7c, 0xfe, 0x82, 0xfe, 0xfe, 0xfe, 0x7c, 0x00, 0xfe, 0xfe, 0xfe, 0xfe, 0x22, 0x3e, 0x1c, 0x00, 0x7c, 0xfe, 0xa2, 0xfe, 0xfe, 0xfe, 0x7c, 0x00, 0xfe, 0xfe, 0xfe, 0xfe, 0x22, 0xfe, 0xdc, 0x00, 0x4c, 0xde, 0x9e, 0xfe, 0xf2, 0xf6, 0x64, 0x00, 0x02, 0x02, 0xfe, 0xfe, 0xfe, 0x02, 0x02, 0x00, 0x7e, 0xfe, 0x80, 0xfe, 0xfe, 0xfe, 0x7e, 0x00, 0x3e, 0x7e, 0xe0, 0xfe, 0xfe, 0x7e, 0x3e, 0x00, 0x7e, 0xfe, 0xe0, 0x7c, 0xe0, 0xfe, 0x7e, 0x00, 0xc6, 0xee, 0x7c, 0x38, 0x7c, 0xee, 0xc6, 0x00, 0x0e, 0x1e, 0xfe, 0xf8, 0xfe, 0x1e, 0x0e, 0x00, 0xe2, 0xf2, 0xfa, 0xfe, 0xbe, 0x9e, 0x8e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x30, 0x38, 0x3c, 0x38, 0x30, 0x20, 0x00, 0x10, 0x30, 0x70, 0xf0, 0x70, 0x30, 0x10, 0x00, 0x30, 0x78, 0xfc, 0xfc, 0x78, 0x30, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0xc0, 0xe0, 0xa0, 0xfc, 0xb8, 0xb8, 0xff, 0xb0, 0xa0, 0xf4, 0xba, 0xa1, 0xa1, 0xa1, 0xa5, 0xae, 0xb4, 0xe0, 0xc0, 0x18, 0x14, 0x34, 0xff, 0x1e, 0xd6, 0x16, 0x1e, 0x1f, 0x17, 0x17, 0x1e, 0x1e, 0xfe, 0xff, 0x3c, 0x00, 0x80, 0xe0, 0xd0, 0xd8, 0xfc, 0xde, 0xd0, 0xf0, 0xe0, 0x80, 0xe0, 0xc0, 0xff, 0xc0, 0x80, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x38, 0x74, 0x56, 0xde, 0x76, 0xd6, 0x5c, 0x7c, 0x38, 0x00, 0x00, 0x90, 0xf0, 0x90, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0xfe, 0x58, 0x58, 0x5f, 0x7a, 0x5e, 0x5a, 0x5e, 0x7e, 0x7f, 0x7f, 0xe0, 0xc0, 0x00, 0x00, 0x06, 0x0f, 0x0f, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x7f, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7f, 0x7f, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7f, 0x7f, 0x00, 0x00, 0x00, 0x80, 0xc0, 0x20, 0x9f, 0x50, 0x51, 0x52, 0x52, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x1c, 0x14, 0x16, 0x23, 0xc1, 0x80, 0x80, 0xfe, 0x01, 0xfc, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xfc, 0xc1, 0x22, 0x17, 0x8a, 0x43, 0x22, 0x13, 0x02, 0x83, 0x42, 0x23, 0x33, 0x3a, 0x3f, 0x77, 0xe3, 0xc0, 0x20, 0x1f, 0x90, 0x48, 0x28, 0x28, 0x08, 0x0c, 0x07, 0x04, 0x07, 0x0c, 0x08, 0x08, 0x08, 0x18, 0x10, 0x3f, 0xe0, 0xc0, 0x80, 0x80, 0x80, 0xff, 0xf0, 0x10, 0x09, 0xc5, 0x23, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x1e, 0xf3, 0xe1, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x18, 0x7e, 0x18, 0x1e, 0xda, 0x1a, 0x7c, 0x18, 0x40, 0x40, 0x40, 0x60, 0x60, 0x40, 0x40, 0x40, 0x0c, 0xcf, 0x0c, 0x0f, 0x3d, 0x0d, 0xce, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xe0, 0x50, 0xa0, 0xc0, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x38, 0x16, 0x68, 0x28, 0x50, 0x60, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x70, 0x28, 0xd0, 0xe0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0xa0, 0x30, 0x38, 0x16, 0x41, 0x22, 0x04, 0x08, 0x70, 0x80, 0x00, 0x80, 0x80, 0x40, 0xa0, 0x40, 0x80, 0x40, 0xb0, 0x08, 0x28, 0x17, 0x4a, 0x3c, 0x9a, 0x34, 0x38, 0x60, 0xc0, 0x00, 0x80, 0x40, 0x01, 0x05, 0x0a, 0x1c, 0x9a, 0x22, 0x11, 0x00, 0x00, 0x05, 0x0a, 0x0f, 0x14, 0x08, 0x10, 0x50, 0x28, 0x94, 0x0a, 0x25, 0x01, 0x0d, 0x02, 0x08, 0x05, 0x01, 0x03, 0x27, 0x06, 0x0d, 0x0e, 0x08, 0x1e, 0x45, 0x1a, 0x0d, 0x10, 0x0a, 0x00, 0x00, 0x00, 0x01, 0x12, 0x07, 0x85, 0x0a, 0x04, 0x08, 0x10, 0x50, 0x28, 0x94, 0x0a, 0x25, 0x11, 0x1e, 0x28, 0x70, 0x50, 0xd0, 0x2c, 0x82, 0x43, 0x01, 0x04, 0x02, 0x40, 0x01, 0x0b, 0x16, 0x3c, 0x70, 0x50, 0xd0, 0x28, 0x94, 0x44, 0x22, 0x01, 0x02, 0x0a, 0x14, 0x38, 0x70, 0x50, 0x88, 0x04, 0x08, 0x28, 0x14, 0x44, 0x34, 0x0a, 0x25, 0x02, 0x09, 0x84, 0x00, 0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x01, 0x84, 0x03, 0x23, 0x03, 0x04, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x82, 0x07, 0x06, 0x09, 0x7f, 0x49, 0x36, 0x00, 0x3e, 0x41, 0x3e, 0x00, 0x46, 0x49, 0x31, 0x00, 0x46, 0x49, 0x31, 0x00, 0x08, 0x08, 0x08, 0x49, 0x2a, 0x1c, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00,
};

const char PROGMEM wheel[] = {
8,8,	
0x00, 0x18, 0x3c, 0x7e, 0x7e, 0x3c, 0x18, 0x00, 0x00, 0x00, 0x18, 0x2c, 0x34, 0x18, 0x00, 0x00
};

const char PROGMEM buggie[] = {
24, 8,
0x78, 0xf8, 0xf8, 0xf8, 0xff, 0xff, 0xfe, 0xfc, 0xf8, 0xf8, 0xf8, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfc, 0xfc, 0xf8, 0x70, 0x00, 0x70, 0x70, 0x70, 0x70, 0x7e, 0x74, 0x78, 0x70, 0x70, 0x70, 0x70, 0x7e, 0x7e, 0x72, 0x72, 0x72, 0x72, 0x72, 0x74, 0x78, 0x78, 0x70, 0x00, 	
};

const char PROGMEM blaster[] = {
	
16,8,
0x18, 0x00, 0x42, 0x5a, 0x18, 0x5a, 0x5a, 0x5a, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x18, 0x18, 0x00, 0x42, 0x18, 0x00, 0x18, 0x42, 0x18, 0x5a, 0x7e, 0x76, 0x7e, 0x7a, 0x34, 0x18, 0x00, 0x42, 0x42, 0x18, 0x5a, 0x5a, 0x5a, 0x18, 0x7e, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x18, 0x42, 0x00, 0x00, 0x18, 0x42, 0x18, 0x00, 0x24, 0x7e, 0x6e, 0x7e, 0x7e, 0x5e, 0x2c, 0x18, 0x00, 
};

const char PROGMEM dualShot[] = {

8,8,
0x06, 0x7f, 0xff, 0x7f, 0x7f, 0xff, 0x7f, 0x06, 0x00, 0x06, 0xaf, 0x00, 0x00, 0xaf, 0x06, 0x00, 
	
};

const char PROGMEM smallRock[] = {
	
8,16,	
0x80, 0xe0, 0xfc, 0xfe, 0xfe, 0xfc, 0xf8, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0xe0, 0xbc, 0xea, 0xfe, 0xfc, 0xf8, 0x80, 0xfb, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xf7, 0xff, 	
};

const char PROGMEM largeRock[] = {	
16,16,
0xc0, 0xe0, 0xf8, 0xfc, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0xe0, 0xb8, 0xd4, 0xfa, 0xfe, 0xfe, 0xfe, 0xbe, 0xfa, 0xfc, 0xd8, 0xb0, 0xe0, 0xc0, 0x80, 0xf7, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xf7, 
};

const char PROGMEM smallPit[] = {
16,8,	
0x01, 0x03, 0x03, 0x07, 0x07, 0x0f, 0x1f, 0x7f, 0xff, 0xff, 0x3f, 0x1f, 0x0f, 0x03, 0x01, 0x01, 0xfe, 0xfc, 0xfc, 0xf8, 0xf8, 0xf0, 0xe0, 0x80, 0x00, 0x00, 0xc0, 0xe0, 0xf0, 0xfc, 0xfe, 0xfe, 	
};

const char PROGMEM largePit[] = {
24, 8,	
0x01, 0x03, 0x03, 0x07, 0x07, 0x0f, 0x0f, 0x0f, 0x1f, 0x3f, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 0x01, 0xfe, 0xfc, 0xfc, 0xf8, 0xf8, 0xf0, 0xf0, 0xf0, 0xe0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xfe, 	
};

const char PROGMEM ufo[] = {
16, 8,	
0x38, 0x7c, 0x7c, 0x7c, 0x7e, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x7e, 0x7c, 0x7c, 0x7c, 0x38, 0x00, 0x28, 0x28, 0x28, 0x24, 0x20, 0x68, 0x6c, 0x6e, 0x4e, 0x2c, 0x2c, 0x28, 0x28, 0x28, 0x00, 
};

const char PROGMEM balls[] = {
16, 16,
0x00, 0x1c, 0x3e, 0x7f, 0x7f, 0x7f, 0xfe, 0xfc, 0xc0, 0xe0, 0xf0, 0xf0, 0xf0, 0xe0, 0xc0, 0x00, 0x00, 0x00, 0x38, 0x7c, 0xfe, 0xfe, 0xff, 0x7f, 0x3f, 0x03, 0x07, 0x07, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x1c, 0x3a, 0x3e, 0x3e, 0x5c, 0x80, 0x00, 0xc0, 0xe0, 0xa0, 0xe0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x7c, 0x7c, 0x5c, 0x3a, 0x00, 0x01, 0x03, 0x03, 0x03, 0x01, 0x00, 0x00, 
};

const char PROGMEM evilWave[] = {
8,8,
0x0f, 0x7f, 0xfc, 0xff, 0xff, 0xfc, 0x7f, 0x0f, 0x07, 0x0c, 0x70, 0xc6, 0xc6, 0x70, 0x0c, 0x07, 
};

const char PROGMEM evilBomb[] = {
8,8,
0x30, 0x78, 0xfe, 0xff, 0xff, 0xff, 0x7b, 0x31, 0x00, 0x30, 0x68, 0xf6, 0xfd, 0x79, 0x31, 0x00, 
};

const char PROGMEM bulletMan[] = {
16, 8,
0x18, 0x3c, 0x7e, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xa5, 0x81, 0x81, 0x00, 0x18, 0x34, 0x3c, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x24, 0x24, 0x00, 0x24, 0x00, 0x00, 0x00, 		
};

const char PROGMEM vertExplode[] = {
16, 24,

0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xf8, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0c, 0xfc, 0xf0, 0xff, 0xff, 0xff, 0xe0, 0xe0, 0xe8, 0x18, 0x00, 0x00, 0x80, 0x70, 0x70, 0xf1, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0x7f, 0xfe, 0xef, 0xe3, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x04, 0x30, 0xe0, 0x80, 0xff, 0xe0, 0x80, 0xc0, 0x60, 0x08, 0x00, 0x00, 0x80, 0x20, 0x20, 0xa0, 0x52, 0x74, 0x7d, 0xff, 0x37, 0x3f, 0x7d, 0xbf, 0x3e, 0x6c, 0x42, 0x40, 0x00, 0x00, 0x40, 0xc0, 0x84, 0x7c, 0xf8, 0xc0, 0xfe, 0xfe, 0xf0, 0x7e, 0x8e, 0xf8, 0xfe, 0x06, 0x00, 0x00, 0x10, 0x71, 0xff, 0x9e, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xfc, 0xcf, 0xf7, 0x3c, 0x0e, 0x02, 0x00, 0x0c, 0x38, 0x71, 0x7f, 0x7f, 0xff, 0x03, 0x07, 0x03, 0xff, 0xff, 0x13, 0x08, 0x04, 0x04, 0x00, 0x40, 0x80, 0x04, 0x78, 0xc0, 0x00, 0x7e, 0x80, 0x00, 0x70, 0x0e, 0x80, 0xf8, 0x06, 0x00, 0x00, 0x10, 0x61, 0x9e, 0x00, 0x93, 0xe0, 0x7c, 0xef, 0xfc, 0xc0, 0x08, 0xc7, 0x30, 0x0c, 0x02, 0x00, 0x08, 0x30, 0x01, 0x06, 0x77, 0x00, 0x00, 0x03, 0x00, 0x01, 0xfe, 0x03, 0x08, 0x00, 0x04, 0x06, 0x0f, 0x23, 0x33, 0xff, 0xee, 0x06, 0x67, 0x83, 0xfb, 0xe2, 0x6e, 0xe7, 0xc3, 0x1f, 0x3e, 0x00, 0x02, 0x33, 0x1b, 0x73, 0xc6, 0x1c, 0x7c, 0x00, 0xff, 0xff, 0x00, 0x78, 0x0c, 0x86, 0x80, 0x00, 0x04, 0x0c, 0x00, 0x00, 0x01, 0x06, 0x00, 0x00, 0x18, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x06, 0x01, 0x21, 0x13, 0x36, 0xe0, 0x02, 0x27, 0x03, 0x7b, 0x22, 0x20, 0x26, 0xc3, 0x07, 0x1e, 0x00, 0x02, 0x11, 0x09, 0x32, 0xc4, 0x18, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x38, 0x04, 0x02, 0x80, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x7f, 0x67, 0xe3, 0xc0, 0x00, 0x04, 0x08, 0x20, 0x10, 0x41, 0xe1, 0xe7, 0xff, 0xfe, 0x7c, 0x00, 0x07, 0x0f, 0x0c, 0x08, 0x40, 0x82, 0x04, 0x00, 0x00, 0x40, 0x20, 0x09, 0x0f, 0x0e, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x06, 0x06, 0x3c, 0x66, 0x43, 0xc1, 0x80, 0x00, 0x04, 0x00, 0x00, 0x10, 0x00, 0x41, 0xc1, 0xe6, 0x7c, 0x18, 0x00, 0x07, 0x0c, 0x08, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x08, 0x06, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x04, 	

};

const char PROGMEM mediumExplode[] = {
16,16,
0x00, 0xc6, 0xdf, 0xfe, 0xfc, 0xf8, 0xf0, 0xf0, 0xfe, 0xf0, 0xe0, 0xf0, 0x78, 0x4c, 0x46, 0x00, 0x01, 0x03, 0x47, 0xff, 0x7f, 0x3f, 0x1f, 0x1f, 0x3f, 0x7f, 0x3f, 0x1f, 0x0f, 0x1e, 0x1c, 0x20, 0x00, 0x00, 0x46, 0x1c, 0xb8, 0xb0, 0xc0, 0xc0, 0xf4, 0xc0, 0x60, 0xb0, 0x08, 0x44, 0x00, 0x00, 0x00, 0x01, 0x01, 0x45, 0x31, 0x1f, 0x0f, 0x05, 0x1f, 0x3f, 0x03, 0x16, 0x06, 0x0c, 0x10, 0x00, 0x0f, 0xdf, 0xe3, 0x2b, 0x12, 0x38, 0x9c, 0x1c, 0x3b, 0xc0, 0xc4, 0x96, 0x27, 0x73, 0x3f, 0x1e, 0x70, 0x79, 0xfb, 0xfa, 0x64, 0x41, 0x0c, 0x30, 0x72, 0xe3, 0xfb, 0x71, 0x58, 0x99, 0x3f, 0x1e, 0x06, 0x0b, 0xc1, 0x2a, 0x00, 0x18, 0x0c, 0x0c, 0x01, 0x00, 0xc0, 0x92, 0x03, 0x21, 0x31, 0x1e, 0x60, 0x70, 0xd9, 0xca, 0x40, 0x01, 0x08, 0x20, 0x60, 0xc2, 0xf3, 0x61, 0x18, 0x90, 0x31, 0x1e, 0x46, 0x47, 0x01, 0x02, 0x24, 0x40, 0x00, 0x0c, 0x00, 0x00, 0x12, 0x32, 0xa8, 0x89, 0x23, 0x26, 0x70, 0xf2, 0xc2, 0x94, 0xb4, 0x40, 0x10, 0x08, 0x00, 0x60, 0x31, 0x02, 0x98, 0x80, 0xf0, 0x78, 0x46, 0x01, 0x00, 0x02, 0x20, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x12, 0x20, 0x88, 0x01, 0x22, 0x70, 0xc2, 0x80, 0x14, 0xa0, 0x00, 0x10, 0x00, 0x00, 0x40, 0x20, 0x02, 0x10, 0x80, 0x80, 0x70, 	
};

const char PROGMEM superUFO[] = {
32,16,
0xc0, 0xe0, 0xf0, 0xf0, 0xf8, 0xf8, 0xfc, 0xfc, 0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe, 0xfc, 0xfc, 0xf8, 0xf8, 0xf0, 0xf0, 0xe0, 0xc0, 0x07, 0x0f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x3f, 0x3f, 0x7f, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x3f, 0x3f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x0f, 0x07, 0x00, 0x40, 0x60, 0x60, 0x70, 0x50, 0x48, 0x48, 0x64, 0x64, 0x72, 0x72, 0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 0x7a, 0x7e, 0x7e, 0x7e, 0x7c, 0x7c, 0x78, 0x78, 0x70, 0x70, 0x60, 0x60, 0x40, 0x00, 0x00, 0x03, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x08, 0x08, 0x08, 0x08, 0x1b, 0x1b, 0x3b, 0x7b, 0x7b, 0x7b, 0x6b, 0x2b, 0x1b, 0x1b, 0x08, 0x08, 0x08, 0x08, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x03, 0x00, };

const int jumpVelocity[] = {-3, -2, -1, -1, 0, -1, 0, 0, 0, 0, 1, 0, 1, 1, 2, 3};

const int makeDigits[] = {

	0b00011111,
	0b00010001,	
	0b00011111,
	
	0b00000000,	
	0b00011111,
	0b00000000,	
		
	0b00011101,
	0b00010101,
	0b00010111,	

	0b00010001,
	0b00010101,
	0b00011111,

	0b00000111,
	0b00000100,
	0b00011111,
	
	0b00010111,
	0b00010101,
	0b00011101,

	0b00011111,
	0b00010101,
	0b00011101,

	0b00000001,
	0b00000001,
	0b00011111,

	0b00011111,
	0b00010101,
	0b00011111,

	0b00000111,
	0b00000101,
	0b00011111,

	0b00001010,
	0b00000000,	
	0b00000000,
			
};

const int8_t sineWave[] = {
16,17,18,18,19,20,21,21,
22,23,24,24,25,26,26,27,
27,28,28,29,29,30,30,30,
31,31,31,32,32,32,32,32,
32,32,32,32,32,32,31,31,
31,30,30,30,29,29,28,28,
27,27,26,26,25,24,24,23,
22,21,21,20,19,18,18,17,
16,15,14,14,13,12,11,11,
10,9,8,8,7,6,6,5,
5,4,4,3,3,2,2,2,
1,1,1,0,0,0,0,0,
0,0,0,0,0,0,1,1,
1,2,2,2,3,3,4,4,
5,5,6,6,7,8,8,9,
10,11,11,12,13,14,14,15,
};

uint8_t wavePointer = 0;

//Your game variables (RAM) here-------------------------------------------------------------------------------
uint8_t gamePad;				//Gets the value from buttons
uint8_t buttonRepeatFlag;		//Flag to make button single-press
uint8_t buttonRepeatMask;		//Mask of what buttons are single press (default = none)

uint8_t buttonsPressed = 0;		//		

uint8_t buttonDebounceMask[] = {dUp, dDown, dLeft, dRight, dMenu, dB, dA};	//Saves FLASH when doing debounce tests

uint8_t sideOfMoon = 0;						//Which side of the moon you are on (rotates LCD, reverses left/right control)

uint8_t changeTime = 0;						//0 = time, 1 = change hour, 2 = change minute

uint8_t attackSpeed = 150;					//Frames between each enemy attacking player. Lower = harder

uint16_t xWindow = 0;
int16_t xWindowCoarse = 0;
uint16_t frameCounter = 0;
uint8_t bgScroll = 0;

int8_t bugXfine = 0;
uint8_t bugX = 16;
uint8_t bugY = 51;
uint8_t wheelXcoarseTile[] = {227, 228, 229};
int8_t wheelSpin = 1;

uint8_t jump = 0;

uint8_t playerState = 0;
uint16_t playerTimer = 0;
int playerFrame = 0;

uint8_t spawnFineCounter = 0;
uint8_t spawnPointer = 224 + 20;

uint8_t groundSlope = 1;					//The next block of terrain to be drawn 0 = full, 1 = one pixel lower, 2 = 2 pixels lower

uint8_t speed = 1;

uint8_t stage;
uint8_t stageWin = 0;
uint16_t eventTimer = 0;					//Timer used for stage action
uint16_t eventGoal = 0;						//""
uint8_t numberSpawned;
uint8_t spawnPerWave;							//How many enemies to spawn on this wave
uint8_t numberOfWaves;
uint8_t killedPerWave;
uint8_t skyEnemies;
uint8_t distance;							//How many screens we have traversed in this checkpoint
uint8_t distanceGoal;						//How many screens (256 pixels, so really 2 screens) to the next goal
uint8_t byDistance;							//Value to auto-increment distance by when windows pass. 0 = must clear enemies not just scroll
uint8_t checkPoint;							//Which goal you've reached (0=A, 5=F)
uint32_t score = 0;
uint8_t lives;
uint8_t stagePhase = phaseNone;				//Stage phase (none)
uint16_t jumpScore;							//Set to points if a jump over an object is successful

uint16_t messageFlash = 0xFFFF;

struct shotData {					//Vertical player and enemy shots
	int x;
	int y;	
};

struct shotData heroShots[2];
uint8_t shotPointer = 0;

struct enemyData {
	const char *gfx;
	int16_t x;
	int8_t y;
	uint8_t type;
	int8_t mirror;
	int8_t dir;
	uint8_t state;
	uint8_t frame;
	uint8_t count;
	uint8_t grounded;
	uint16_t timer;	
};

struct enemyData enemy[maxEnemies];
uint8_t enemyPointer = 0;
const int16_t enemySpawnSide[2] = {-16, 128};

uint8_t xCollide;													//X and Y collision flags for enemy-player
uint8_t yCollide;

#define bossStartingHealth		52
uint8_t bossHealth = bossStartingHealth;							
uint8_t bossBattle = 0;								//Flag if we've in a boss battle
uint8_t bossPhase = 0;								//How many loops the boss has done

int8_t blasterX = spriteOff;						//Disabled state
uint8_t blasterY = 0;

uint8_t gameState = stateTitle; //stateRunning;

uint8_t isDrawn;

uint8_t obstacleTimer = 10;								//Spacing (in frames) between the spawning of obstacles. Lower= harder
uint8_t lastObstacleSpawned;							

char name[] = {'A', 'B', 'C'};							//Used for high score name entry
char nameShift[3];										//Used for sorting high scores

uint8_t scoreRanking = 1;

void gameSetup() {							//This function is called once per boot/program

	uint8_t temp = eepromReadByte(0);		//See if any names entered

	if (temp < 64 || temp > 90) {			//ASCII alphabet not found? Must be blank EEPROM. Write defaults
		writeDefaultHighScores();
	}

	setScrollDirection(horizontal);
		
}

void systemLoop() {

    while (1) {
		if (sleepState) {
			sleep_cpu();
		}				
		else {
			if (frameFlag == 16) {		//Time to draw a frame? (60-ish Hz)
				frameFlag = 0;			//Reset ms counter
				ledState(1);			//LED is "on" for the frame
				gameFrame();			//New frame of game logic
				ledState(0);			//connect scope to this line to see frame time
			}	
		}
    }
	
}

void gameFrame() {							//This function is called at 60-ish Hz

	gamePad = getButtons();					//Get controls into local var

	buttonsPressed = 0;						//Universal button check for all functions

	for (int x = 0 ; x < 7 ; x++) {			//Scan through debounce buttons (Up, down, menu, B and A)

		uint8_t temp = buttonDebounceMask[x];		//Scan RAM scans

		if (gamePad & temp) {						//Button pressed?
			if (temp & buttonRepeatMask) {			//Repeats allowed?		
				buttonsPressed |= temp;				//Set bit				
			}
			else {									//Repeat restricted?
				if (buttonRepeatFlag & temp) {		//Check if released first
					buttonRepeatFlag &= !temp;
					buttonsPressed |=temp;
				}			
			}
		}
		else {
			buttonRepeatFlag |= temp;
		}

	}

	if ((buttonsPressed & dMenu) && gameState & canPauseMask) {		//MENU to pause/unpause?
		if (gameState == statePaused) {	//Change state
			gameState = stateRunning;
		}
		else {
			gameState = statePaused;
			frameCounter = 0x20;			//Clean transition to blinking PAUSED (pedantic, I know)
		}
	}
	
	if (buttonsPressed & dRight && (gameState & canSwitchScreens)) {
		//cls();
		playerTimer = 300; 		
		isDrawn = 0;
		if (++gameState == 0x0C) {		//Loop around?			
			gameState = stateTitle;			
		}
	}	
	
	if (buttonsPressed & dLeft && (gameState & canSwitchScreens)) {
		//cls();
		isDrawn = 0;
		playerTimer = 300; 
		if (--gameState == 0x07) {		//Loop around?
			gameState = stateOptions;
		}
	}
	
	switch(gameState) {

		case stateTitle:
			if (!isDrawn) {							//First time here? Draw the title screen
				drawTitle();
			}			
			sendTiles();
			
			if (++frameCounter & 1) {
				xWindow++;						
			}						
			setRowScroll(xWindow >> 1, 5);			//WE SCROLL THE MOON!
			setRowScroll(xWindow >> 1, 6);			//BECAUSE IT IS CLOSE TO US!
			setRowScroll(xWindow, 7);				//WE SCROLL THE MOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOON!
			
			if (buttonsPressed & dA) {
				startNewGame();			
			}						
		break;
		
		case stateGetReady:
			gameStatus(1);						//Draw status display
			if (frameCounter == 120) {			//2 seconds of GET READY splash
				gameState = stateRunning;
				drawMoonscape();
			}
		break;
		
		case statePaused:
			gameStatus(0);
		break;
		
		case stateRunning:
			gameAction();
		break;
		
		case stateGameOver:
			gameStatus(2);						//Draw status display with GAME OVER
			if (frameCounter == 180) {			//2 seconds of GET READY splash
				gameState = stateEntry;			//See if we made a high score, else jumps to title screen
				isDrawn = 0;
			}		
		break;
		
		case stateShowTime:
			sendTiles();
			drawTime();						
			if (buttonsPressed & dB) {
				playerTimer = 300;
				if (++changeTime == 3) {	//Cycle through hours, minutes and done with each press
					changeTime = 0;
				}
			}	
			if (buttonsPressed & dA) {
				playerTimer = 300;
				SPI0_CTRLB = 0;							//Disable SPI buffer mode since we are sending single byte commands			
				sideOfMoon = setRotate(0xFF);			//Toggle screen rotate and store local copy
				SPI0_CTRLB = SPI_BUFEN_bm;				//Resume SPI buffer mode					
			}				
		break;
		
		case stateHighScores:
			if (!isDrawn) {							//First time here? Draw the title screen
				drawHighScores();
			}		
			sendTiles();	
		break;
		
		case stateOptions:
			sendTiles();	
			drawOptions();
		break;	
		
		case stateEntry:
			if (!isDrawn) {							//First time here? Draw the title screen
				drawNameEntryScreen();
			}
			
			drawCharacterRow();
			
			uint8_t character = tileMap[167];

			for (int x = 0 ; x < 3 ; x++) {
				tileMap[96 + x] = name[x];
			}

			if (playerTimer) {
				if (++playerTimer == 90) {
					updateHighScores();	
				}
			}
			else {
				if (frameCounter & 0x08) {
					name[bugY] = 124;
				}
				else {
					name[bugY] = character;
				}

				if (buttonsPressed & dB) {
					if (bugY) {
						name[bugY--] = 124;
					}
				}

				if (buttonsPressed & dA) {
					name[bugY] = character;
					if (++bugY == 3) {
						playerTimer = 1;
					}
				}

				if (buttonsPressed & dRight) {
					if (++bugX > 90) {
						bugX = 65;
					}
				}
				if (buttonsPressed & dLeft) {
					if (--bugX < 65) {
						bugX = 90;
					}
				}			
			}
			sendTiles();
			frameCounter++;
		break;
	}

}

void gameAction() {

	sendTiles();
	frameCounter++;							//Increment counter

	uint8_t distanceCheck = 0;				//Flag if we should check for checkpoint distance (allows 2 ways of triggering a checkpoint, event or # of window scrolls)
	
	playerLogic();
	enemies();

	if (messageFlash == 0xFFFF) {			//Can spawn stuff?
		
		if (stagePhase & phaseSkyMask) {	//Sky enemies active?

			if (numberSpawned != spawnPerWave) {		//Haven't spawned them all yet?
				if (eventTimer) {						//Remove?
					if (--eventTimer == 0) {			//Time to spawn
						numberSpawned++;
						
						switch(stagePhase & phaseSkyMask) {
							
							case phaseUFOs:
								spawnEnemy(enemySpawnSide[getRandom(0x01)], getRandom(0x01) * 8, 1, enemyUFO);
							break;
							
							case phaseBalls:
								spawnEnemy(enemySpawnSide[getRandom(0x01)], getRandom(0x01) * 8, 1, enemyBalls);
							break;
							
							case phaseUFOs | phaseBalls:;				//Spawn both at once, different sides of screen
								uint8_t whichSide = getRandom(0x01);
								spawnEnemy(enemySpawnSide[whichSide], getRandom(0x01) * 8, 1, enemyUFO);
								spawnEnemy(enemySpawnSide[!whichSide & 0x01], getRandom(0x01) * 8, 1, enemyBalls);
								skyEnemies += 1;
							break;
							
						}
						//spawnEnemy(enemySpawnSide[getRandom(0x01)], getRandom(0x03) * 8, 1, enemyUFO); 0x03 makes them low enough to hit you in a jump, should save that for later levels
						eventTimer = eventGoal + (getRandom(0x07) * 3);
						skyEnemies += 1;
					}
				}
			}
			else {										//OK so they have all spawned
				if (!skyEnemies) {					//Did we spawn them all and player killed them all? Next wave
					numberSpawned = 0;				//Clear the spawned count
					if (--numberOfWaves) {
						eventTimer = 80;			//Frames before next wave
					}
					else {
						distance = distanceGoal;		//Set goal as REACHED
						distanceCheck = 1;				//Set flag for check
					}
				}
			}

			//drawDecimal(eventTimer, tileMap + 8);
			//drawDecimal(skyEnemies, tileMap);
			//drawDecimal(numberOfWaves, tileMap + 32);
			//drawDecimal(numberSpawned, tileMap + 4);
		}

		
		if (stagePhase & phaseGroundMask) {			//An obstacle phase, and we can spawn them?
			
			uint8_t whichItem = getRandom(0x03);							//Get random 0-3
			
			uint8_t speedModifier = 60;
			
			if (stage > 1 && checkPoint == 0) {
				speedModifier = 40;
			}
			
			if (!(stagePhase & phasePits)) {		//Just rocks?
				whichItem &= 0x01;					//Remove pits from rando
			}

			if (--obstacleTimer == 0) {											//Time to spawn?
							
				obstacleTimer = (speedModifier - (stage * 4)) + getRandom(0x0F);		//Reset spawn timer + random (work on scaling difficulty)
				
				if (whichItem > 1) {				//Rando for a pit? Make sure there's enough space to land after jumping
					obstacleTimer += 20;			//Add some space
				}
				spawnObstacle(127, 1 << whichItem);							//Spawn obstacle
			}
			//drawDecimal(obstacleTimer, tileMap);
		}
	}
	
	xWindow += speed;

	if (messageFlash != 0xFFFF) {			//Checkpoint message active?
		setRowScroll(messageFlash, 6);		//Scroll past it
		messageFlash += speed;
		
		if (messageFlash > 255) {			//Disable message
			phaseStart();					//Decide what to do in this phase
			messageFlash = 0xFFFF;			//This can act as a flag to allow enemy spawn
		}
	}
	
	if ((xWindow & 0x07) == 0x07) {
		spawnFineCounter = 0;
		if (++spawnPointer == 0) {
			spawnPointer = 224;
		}
		tileMap[spawnPointer] = getGround();

	}
	
	bgScroll += (frameCounter & 1);

	if (bossBattle) {							//Boss active? Draw their health bar across top of screen over everything
		fillTiles(16, 160, 3);									//Draw "Boss->"
		for (int g = 19 ; g < 32 ; g++) {		//Draw all black squares first
			tileMap[g] = 163;
		}
		
		uint8_t drawEdge = (31 - (bossHealth >> 2));			//How many full 4 unit health bars to draw (can be 0)
		
		for (int g = 31 ; g > drawEdge ; g--) {
			tileMap[g] = 167;
		}
		
		if (bossHealth & 0x03) {
			tileMap[drawEdge] = 163 + (bossHealth & 0x03);			//Draw sub health (1-3 pixels)
		}
		
		if (frameCounter & 1) {					//Flicker between sky graphics and the Boss Health Bar
			setRowScroll(0, 0);
		}
		else {
			setRowScroll(128, 0);
		}
		
	}
	else {
		setRowScroll(0, 0);
	}

	setRowScroll(0, 1);
	setRowScroll(bgScroll >> 1, 2);
	setRowScroll(bgScroll >> 1, 3);
	setRowScroll(bgScroll, 4);
	setRowScroll(bgScroll, 5);
	//setRowScroll(xWindow, 6);
	setRowScroll(xWindow, 7);
	
	if (xWindow > 255) {
		xWindow -= 256;
		
		if (messageFlash == 0xFFFF) {
		
			distanceCheck = 1;
		
			//drawDecimal(distance, tileMap);
			//drawDecimal(distanceGoal, tileMap + 32);
		}
				
	}

	if (distanceCheck) {
		
		distance += byDistance;					//Does simply scrolling the screen advance you?
		
		if (distance >= distanceGoal) {
			distance = 0;
			speed = 1;							//Back to normal speed... for NOW
			checkPoint++;
			//drawText("CHECKPOINT", tileMap + 192 + 18);
			tileMap[192 + 30] = checkPoint + 65;
			messageFlash = 0;
		}
		
	}
	
}

void playerLogic() {

	//drawDecimal(score, tileMap + 32);			//BJH
	
	xWindowCoarse = (xWindow + bugX) >> 3;
	
	uint8_t suspension = 0;

	switch(playerState) {
		
		case playerDriving:
			for (int x = 0 ; x < 3 ; x++) {
				wheelXcoarseTile[x] = 225 + xWindowCoarse + x;
				if (wheelXcoarseTile[x] < 225) {
					wheelXcoarseTile[x] -= 32;
				}
				suspension += tileMap[wheelXcoarseTile[x]];
			}
		
			if (frameCounter & 1) {
				wheelSpin = -1;
			}
			else {
				wheelSpin = 1;
			}

			drawSprite(buggie, bugX, bugY - (6 - (suspension >> 1)), 0, 1);
			drawSprite(wheel, bugX, bugY + tileMap[wheelXcoarseTile[0]], 0, wheelSpin);
			drawSprite(wheel, bugX + 8, bugY + tileMap[wheelXcoarseTile[1]], 0, wheelSpin);
			drawSprite(wheel, bugX + 16, bugY + tileMap[wheelXcoarseTile[2]], 0, wheelSpin);
		break;
		
		case playerJumping:
			drawSprite(buggie, bugX, bugY, 0, 1);
			uint8_t wheelOffset = 6;
			if (jump > 24) {
				wheelOffset = 4;
			}
			drawSprite(wheel, bugX, bugY + wheelOffset, 0, 1);
			drawSprite(wheel, bugX + 8, bugY + wheelOffset, 0, 1);
			drawSprite(wheel, bugX + 16, bugY + wheelOffset, 0, 1);
		break;

		case playerExploding:
			bugX -= speed;
			drawSprite(vertExplode, bugX + 4, bugY - 16, playerFrame, 1);
			tone(playerTimer * 300, 20, 10, 5);
			if (--playerTimer == 0) {
				playerTimer = 5;
				if (++playerFrame == 4) {
					playerState = playerDead;
					playerTimer = 60;
				}
			}
		break;
		
		case playerDead:
			if (--playerTimer == 0) {
				//if (lives == 255) {				//Rolls under from to 255?
				if (--lives == 255) {				//Rolls under from to 255?
					gameState = stateGameOver;
					frameCounter = 0;
				}
				else {
					startNewLife();
				}
			}
		break;
	}

	shotLogic();
	
	if (blasterX != spriteOff) {							//Blaster shot active? (the front shot)
		drawSprite(blaster, blasterX, blasterY, (frameCounter >> 2) & 1, 1);
		blasterX += 2;
		if (blasterX > 125) {								//Shot at edge of screen? Despawn it
			blasterX = spriteOff;
		}
	}
	
	if (jump) {
		bugY += jumpVelocity[(jump - 1) >> 1];
		
		tone(jump * 500, 20, 8, 0);
		
		if (++jump == 33) {
			jump = 0;
			if (jumpScore) {
				score += jumpScore;
				jumpScore = 0;
				tone(5000, 200, 8, -5);
			}
			else {
				tone(10000, 200, 8, 0);
			}
			
		}
		
	}
	
	uint8_t shootWeapons = 0;

	if (playerState & playerAlive) {				//Controls don't work if you're DEAD!

		if (sideOfMoon) {							//Flip left/right if screen rotated 180
			if (gamePad & dRight) {
				if (bugX > 0) {
					bugX--;
				}
			}
			if (gamePad & dLeft) {
				if (bugX < 103) {
					bugX++;
				}
			}
		}
		else {
			if (gamePad & dLeft) {
				if (bugX > 0) {
					bugX--;
				}
			}
			if (gamePad & dRight) {
				if (bugX < 103) {
					bugX++;
				}
			}
		}
		
		if (buttonsPressed & dA) {
			if (jump == 0) {			//Jump!
				jump = 1;
			}
		}
		if (buttonsPressed & dB) {
			shootWeapons = 1;
		}

		if (shootWeapons) {
			if (heroShots[shotPointer].y == spriteOff) {		//This sprite is eligible to be spawn
				heroShots[shotPointer].x = bugX + 2;			//X position
				heroShots[shotPointer].y = bugY - 8;
				
				tone(800, 100, 10, 10);
				
				if (++shotPointer > 1) {			//Max 2 vert shots at any time
					shotPointer = 0;
				}
			}
			
			if (blasterX == spriteOff) {			//Can fire hori shot?
				blasterX = bugX + 16;				//Blaster in front of buggie
				tone(800, 100, 10, 10);
				if (jump) {
					blasterY = bugY + 2;
				}
				else {
					blasterY = bugY - 3;
				}
				
			}
		}
		
	}
	
	if (stageWin) {
		
		if (frameCounter & 0x04) {				//Flash EXTRA LIFE
			setRowScroll(128, 6);
		}
		else {
			setRowScroll(0, 6);	
		}
		
		if (--playerTimer == 0) {
			stage++;							//New state
			lives++;							//New life!
			gameState = stateGetReady;
			frameCounter = 0;
			bossBattle = 0;
			checkPoint = 0;			//Which goal you've reached (0=A, 5=F)
			startNewLife();
		}
	}

}

void updateHighScores() {

	scoreRanking--;												//Set to zero index	

	uint32_t tempScore = 0;
	int8_t tempRow;

	if (scoreRanking < 5) {										//Player not in 6th place? Shift all places below them
		
		for (int row = 4 ; row >= scoreRanking ; row--) {		//6th places (5 index) is gone no matter what so start with 4, shifting it down to 5
			tempRow = row + 1;
			nameShift[0] = eepromReadByte((row * 32) + 0);		//Get name and put into temp array
			nameShift[1] = eepromReadByte((row * 32) + 1);
			nameShift[2] = eepromReadByte((row * 32) + 2);	
			tempScore = eepromReadLong((row * 32) + 4);
		
			eepromWrite(tempRow * 32, nameShift, tempScore);	//Shift into lower place
		}
		
	}
	
	eepromWrite(scoreRanking * 32, name, score);		//Write new high score in slot

	gameState = stateHighScores;
	isDrawn = 0;
		
}

void sendTiles() {

	screenLoad();							//Dump the frame buffer to the OLED
	drawTiles(tileData, tileMap);			//Draw background tiles	
	
}

void gameStatus(uint8_t type) {				//Draw status screen. Type 0 = READY, 1 = PAUSE screen

	csLow();

	if ((++frameCounter & 0x20) && type == 0) {
		drawText("PAUSED", tileMap + 26);	
	}

	nextRow();
	drawText("STAGE:", tileMap + 16);
	drawDecimal(stage, tileMap + 22);	
	nextRow();									//Blank row
	drawText("SCORE:", tileMap + 16);
	drawDecimal(score, tileMap + 22);
	nextRow();
	drawText("LIVES:", tileMap + 16);
	if (lives < 255) {
		drawDecimal(lives, tileMap + 22);
	}
	nextRow();
	drawText("A  B  C  D  E  F", tileMap + 16);		//Map
	nextRow();
	drawText("m~~n~~n~~n~~n~~o", tileMap + 16);
	int8_t buggyPosX = 16 + (checkPoint * 3);

	if (messageFlash == 0xFFFF && checkPoint < 5 && type != 1) {	//Fake shit, where we make it look like progress is mapped. IT ISN'T
		buggyPosX++;
	}
	
	uint8_t buggyGFX = 37;
	
	if (type != 2) {
		buggyGFX = 125;
	}

	if (frameCounter & 0x08) {						//Animated buggie on map (cuuuuuuuuuuuuuuuute!)
		tileMap[buggyPosX] = buggyGFX;
	}	
	else {
		tileMap[buggyPosX] = buggyGFX + 2;
	}
	nextRow();
		
	if (type) {								//Pause screen?
		nextRow();
		if (type == 2) {
			drawText("GAME OVER", tileMap + 20);	
		}
		else {
			if (frameCounter & 0x08) {						//Animated buggie on map (cuuuuuuuuuuuuuuuute!)
				drawText("GET  READY", tileMap + 19);
			}			
		}		
	}
	else {
		drawText("B+A = SLEEP", tileMap + 21);
		nextRow();
		drawText("MENU = WAKE", tileMap + 21);				
	}

	nextRow();					
	csHigh();
	
	if (gamePad & dB && gamePad & dA && type == 0) {
		buttonRepeatFlag = dMenu;						//Clear this bit so pressing Menu doesn't exit PAUSE when we awaken from our dark, deep slumber
		gotoSleep();
	}
	
}

void nextRow() {

	rowLoad(tileData, tileMap + 16, draw);
	memset(&tileMap[16], ' ', 16);
	
}

void startNewGame() {

	stage = 1;

	score = 0;
	lives = 2;
	
	startNewStage();

	
}

void startNewStage() {
	
	checkPoint = 0;							//Which goal you've reached (0=A, 5=F)
	attackSpeed = 140 - (stage * 20);
	startNewLife();
	
}

void phaseStart() {				//After CHECKPOINT message scrolls off, decide what to do in this phase (could also be a boss if I have time)

	skyEnemies = 0;
	byDistance = 0;				//Goal based (not scroll based) advancement unless otherwise noted
	distance = 0;
	distanceGoal = 10;			

	obstacleTimer = 1;
	clearEnemies();											//Wipe enemy memory
	numberSpawned = 0;
	numberOfWaves = 3 + (stage >> 1);	
	
	uint8_t offset = (stage * 2);
	
	speed = 1;
				
	switch(checkPoint) {
					
		case 0:
			stagePhase = phaseRocks | phasePits;
			setGoal(4);
			if (stage > 1) {
				speed = 2;
				obstacleTimer = 90;						//Give player more time to react to first one
				setGoal(4 + (stage * 3));
			}
		break;
				
		case 1:	
			if (stage == 1) {							//Training phase. But it's too boring for anything past stage 1
				stagePhase = phaseUFOs;
				eventGoal = 80;				
				spawnPerWave = 2;
				numberOfWaves = 3;			
			}
			else {
				stagePhase = phaseUFOs;
				eventGoal = 40 - offset;
				spawnPerWave = 4;				
			}	

		break;
		
		case 2:
			stagePhase = phaseUFOs | phaseRocks | phasePits;
			eventGoal = 70 - offset;
			spawnPerWave = 3;	
		break;	
		
		case 3:
			stagePhase = phaseBalls;
			eventGoal = 50 - offset;
			spawnPerWave = 3;	
		break;	
		
		case 4:
			stagePhase = phaseBalls | phaseUFOs | phaseRocks;
			eventGoal = 60 - offset;
			spawnPerWave = 4;	
		break;		
						
		case 5:														//BOSS UFO!
			bossBattle = 1;											//Boss battle is GO!
			bossHealth = bossStartingHealth;						//Refill health
			stagePhase = 0;
			spawnBoss();
		break;		
	}

	eventTimer = 1;				//Make something happen RIGHT AWAY (if that type of phase)
				
}

void setGoal(uint8_t theGoal) {

	distance = 0;
	distanceGoal = theGoal;
	byDistance = 1;
	
}

void spawnObstacle(int16_t xPos, uint8_t whichOne) {

	uint8_t yPos = 48;
	
	findNextSlot();
		
	switch(whichOne) {
		case enemySmallRock:
			enemy[enemyPointer].gfx = smallRock;		//This sprite graphic
		break;
		
		case enemyBigRock:
			enemy[enemyPointer].gfx = largeRock;		//This sprite graphic
		break;
		
		case enemySmallPit:
			enemy[enemyPointer].gfx = smallPit;			//This sprite graphic
			yPos = 56;
		break;
		
		case enemyBigPit:
			enemy[enemyPointer].gfx = largePit;			//This sprite graphic
			yPos = 56;
		break;
	}
	
	enemy[enemyPointer].x = xPos;	
	enemy[enemyPointer].y = yPos;
	enemy[enemyPointer].type = whichOne;
	enemy[enemyPointer].grounded = 1;				//Stuck to ground

	if (++enemyPointer == maxEnemies) {				//Cycle baddies
		enemyPointer = 0;
	}

	lastObstacleSpawned = whichOne;

}

void spawnShot(int8_t x, int8_t y, uint8_t whichType) {

	findNextSlot();
	
	switch(whichType) {
		case enemyWaveShot:
			enemy[enemyPointer].gfx = evilWave;		//This sprite graphic
		break;
		
		case enemyBombShot:
			enemy[enemyPointer].gfx = evilBomb;		//This sprite graphic
		break;

	}
	
	enemy[enemyPointer].x = x;
	enemy[enemyPointer].y = y;
	enemy[enemyPointer].type = whichType;
	enemy[enemyPointer].grounded = 0;				//Not stuck to ground
	
	if (++enemyPointer == maxEnemies) {				//Cycle baddies
		enemyPointer = 0;
	}	
	
}

void spawnBoss() {
	
	enemy[enemyPointer].gfx = superUFO;				//This sprite graphic

	enemy[enemyPointer].timer = attackSpeed;
	
	enemy[enemyPointer].x = 95;					//UFO reveal from the top right
	enemy[enemyPointer].y = -8;
	enemy[enemyPointer].state = 1;					//Initial descent

	enemy[enemyPointer].dir = 0;
	enemy[enemyPointer].frame = 0;
	
	enemy[enemyPointer].type = enemyBossUFO;			    //Boss
	enemy[enemyPointer].grounded = 0;				//Not stuck to ground
	
	if (++enemyPointer == maxEnemies) {				//Cycle baddies
		enemyPointer = 0;
	}
		
}

void spawnEnemy(int16_t x, int8_t y, int8_t dir, uint8_t whichType) {

	findNextSlot();
	
	switch(whichType) {
		case enemyUFO:
			enemy[enemyPointer].gfx = ufo;					//This sprite graphic
		break;
		case enemyBalls:
			enemy[enemyPointer].gfx = balls;					//This sprite graphic
		break;
		case enemyBullet:
			enemy[enemyPointer].gfx = bulletMan;					//This sprite graphic
			enemy[enemyPointer].state = getRandom(0xFF);			//Randomize sine wave starting point
		break;		
		
	}

	enemy[enemyPointer].timer = 20; //attackSpeed;	//Enemy will attack player after 20 frames. Then, the spacing between attacks will be shorter per level
	
	enemy[enemyPointer].x = x;
	enemy[enemyPointer].y = y;
	enemy[enemyPointer].dir = dir;

	enemy[enemyPointer].type = whichType;
	enemy[enemyPointer].grounded = 0;				//Not stuck to ground
	
	if (++enemyPointer == maxEnemies) {				//Cycle baddies
		enemyPointer = 0;
	}
		
}

void spawnExplosion(int16_t x, int8_t y, uint8_t speed) {

	findNextSlot();
	
	enemy[enemyPointer].gfx = mediumExplode;					//This sprite graphic

	enemy[enemyPointer].timer = speed;					//Numbers of game frames for each frame of explosion
	
	enemy[enemyPointer].x = x;
	enemy[enemyPointer].y = y;
	enemy[enemyPointer].frame = 0;					//Starting frame
	enemy[enemyPointer].dir = 1;
	
	enemy[enemyPointer].state = speed;
		
	enemy[enemyPointer].type = enemyExplosion;
	enemy[enemyPointer].grounded = 1;				//Explosions "moves" with the ground, should look cool I think?
	
	if (++enemyPointer == maxEnemies) {				//Cycle baddies
		enemyPointer = 0;
	}
	
}

uint8_t findNextSlot() {

	uint8_t iterations = 0;
	
	while(enemy[enemyPointer].y != spriteOff) {
		if (++enemyPointer == maxEnemies) {				//Cycle baddies
			enemyPointer = 0;
		}
		if (++iterations == maxEnemies) {				//No slots left? Return a goose egg
			return 0;
		}
	}	
	
	//tileMap[33] = 32;
	//drawDecimal(enemyPointer, tileMap + 32);
	enemy[enemyPointer].frame = 0;	
	return 1;
		
}

void shotLogic() {
	
	for (int g = 0 ; g < 2 ; g++) {					//Scan through the 3 possible hero and baddie shots (vertical, not the buggie hori blaster)		
		if (heroShots[g].y != spriteOff) {
			drawSprite(dualShot, heroShots[g].x, heroShots[g].y, 0, 1);
			if (--heroShots[g].y == -8) {
				heroShots[g].y = spriteOff;
			}
		}			
	}
	
}

void gotoSleep() {

	sleepState = 1;
	displayOnOff(0);
	ledState(0);
	//Roll around to the main loop which will put us to sleep

}

void enemies() {

	for (int g = 0 ; g < maxEnemies ; g++) {

		if (enemy[g].y != spriteOff) {

			drawSprite(enemy[g].gfx, enemy[g].x, enemy[g].y, enemy[g].frame, enemy[g].mirror);

			int8_t xSize = pgm_read_byte(enemy[g].gfx);
			int8_t ySize = pgm_read_byte(enemy[g].gfx + 1);
			
			xCollide = 0;													//X and Y collision flags for enemy-player
			yCollide = 0;
			uint8_t blasted = 0;													//Collision flag for enemy-blaster (horizontal shot)
			
			if (enemy[g].x < (bugX + 20) && bugX < (enemy[g].x + (xSize - 4))) {			//Check the X and Y collision flags for enemies (including shots) and Player Buggie
				xCollide = 1;
			}			

			if (enemy[g].y < (bugY + 8) && bugY < (enemy[g].y + ySize)) {
				yCollide = 1;
			}	

			for (int h = 0 ; h < 2 ; h++) {											//Scan through the 2 possible hero vertical shots and check for collision against enemies
				if (heroShots[h].y != spriteOff) {
					if (enemy[g].x < (heroShots[h].x + 8) && heroShots[h].x < (enemy[g].x + xSize)) {
						if (enemy[g].y < (heroShots[h].y + 8) && heroShots[h].y < (enemy[g].y + ySize) && enemy[g].type != enemyExplosion) {
							
							if (enemy[g].type == enemyBossUFO) {					//Damage boss or
								damageBoss(g);
								spawnExplosion(enemy[g].x + 8, enemy[g].y + 8, 5);
							}
							else {		
								if (enemy[g].type & skyMask) {							//Was this a sky enemy? (not a projectile)
									skyEnemies--;										//Decrement their counter
									score += 1000;
									spawnExplosion(enemy[g].x, enemy[g].y, 6);				//An explosion where they were
								}
								else {
									score += 10;									//10 pts for projectiles
								}												
								enemy[g].y = spriteOff;								//Kill enemy	
								tone(700, 300, 10, -1);	
							}
	
							heroShots[h].y = spriteOff;								//Despawn hero shot						
						}
					}
				}
			}
			
			if (blasterX != spriteOff) {
				if (enemy[g].x < (blasterX + 16) && blasterX < (enemy[g].x + xSize)) {			//Check the X and Y collision flags for enemies (including shots) and Horizontal Blaster Shot
					if (enemy[g].y < (blasterY + 8) && blasterY < (enemy[g].y + ySize)) {
						blasted = 1;															//Set "Blasted" flag (since effect varies by enemy)
					}
				}				
			}

			if (enemy[g].grounded) {									//Enemy stuck to ground? Obstacle? Scroll with the BG
				enemy[g].x -= speed;

				if (enemy[g].x < (0 - xSize)) {							//Scrolled off-screen? despawn.
					enemy[g].y = spriteOff;
				}	
						
			}
			
			if (enemy[g].y == spriteOff) {								//Did a check above "kill" this enemy? Skip logic checks, goto next enemy in array
				continue;
			}	

			switch(enemy[g].type) {

				case enemyExplosion:
					if (--enemy[g].timer == 0) {
						if (++enemy[g].frame == 3) {
							enemy[g].y = spriteOff;	
						}
						else {
							enemy[g].timer = 6;
						}
					}
				break;
			
				case enemySmallRock:			
				
					if (blasted) {	
						enemy[g].y = spriteOff;
						blasterX = spriteOff;
						tone(1500, 200, 10, 10);
						score += 100;
					}
					
					checkObstacle(g, 44, 500);			//Check for collision and set jump score value	
											
				break;
				
				case enemyBigRock:

					if (blasted) {
						enemy[g].gfx = smallRock;							//Change the sprite graphics
						enemy[g].type = enemySmallRock;							//It becomes SMALL ROCK
						enemy[g].x += 8;										//Move 8 pix right to make gap
						blasterX = spriteOff;
						tone(400, 100, 10, 3);
						score += 50;
					}	
					checkObstacle(g, 44, 1000);			//Check for collision and set jump score value									
									
				break;
				
				case enemySmallPit:
					checkObstacle(g, 50, 300);			//Check for collision and set jump score value						
				break;	
				
				case enemyBigPit:
					checkObstacle(g, 50, 600);			//Check for collision and set jump score value		
				break;
				
				case enemyWaveShot:
					checkHitPlayer(g);					
					if (++enemy[g].y > 52) {										//Despawn wave shot when it hits ground
						enemy[g].y = spriteOff;
					}
					if (blasted) {
						enemy[g].y = spriteOff;										//Hori blaster hit it?
						blasterX = spriteOff;
						tone(700, 300, 9, -1);
					}					
				break;
				
				case enemyBombShot:
					checkHitPlayer(g);
					
					if (++enemy[g].y > 52) {										//Bomb shot makes hole in the ground
						tone(2000, 300, 8, 1);
						enemy[g].y = spriteOff;
						spawnObstacle(enemy[g].x - 8, enemySmallPit);
					}
					if (blasted) {
						enemy[g].y = spriteOff;										//Hori blaster hit it?
						blasterX = spriteOff;
						tone(700, 300, 9, -1);
					}					
				break;	
								
				case enemyUFO:
				
					if (frameCounter & 0x08) {
						enemy[g].mirror = 1; 
					}
					else {
						enemy[g].mirror = -1;
					}
				
					enemy[g].x += enemy[g].dir;
					
					if (enemy[g].x < 1) {
						enemy[g].dir = 1;	
					}
					if (enemy[g].x > 111) {
						enemy[g].dir = -1;
					}
					
					if (enemy[g].x > bugX + 4 && enemy[g].x < bugX + 8 && !enemy[g].timer) {
						enemy[g].timer = attackSpeed;
						spawnShot(enemy[g].x + 4, enemy[g].y + 8, enemyWaveShot);
						tone(400, 100, 10, -1);					
					}
					if (enemy[g].timer) {
						enemy[g].timer--;
					}				
				
				break;	
				
				case enemyBalls:
					if (frameCounter & 0x08) {
						enemy[g].mirror = 1;
					}
					else {
						enemy[g].mirror = -1;
					}
					
					enemy[g].x += enemy[g].dir;
					
					if (enemy[g].x < 1) {
						enemy[g].dir = 1;
					}
					if (enemy[g].x > 111) {
						enemy[g].dir = -1;
					}
					
					if (!enemy[g].timer) {			//Eligible to take a shot?
						if (bugX > 86) {				//Player near right side of screen, and we can't drop a bomb in front of him? OR HER! #PC
							if (enemy[g].x == bugX + 4) {			//Drop bomb right onto player instead. TIME 2 DIE!
								enemy[g].timer = attackSpeed;
								spawnShot(enemy[g].x + 4, enemy[g].y + 8, enemyBombShot);
								tone(600, 100, 10, -1);
							}							
						}
						else {
							if (enemy[g].x == bugX + 40) {
								enemy[g].timer = attackSpeed;
								spawnShot(enemy[g].x + 4, enemy[g].y + 8, enemyBombShot);
								tone(600, 100, 10, -1);
							}							
						}						
					}
									
					if (enemy[g].x == bugX + 40 && !enemy[g].timer) {
						enemy[g].timer = attackSpeed;
						spawnShot(enemy[g].x + 4, enemy[g].y + 8, enemyBombShot);
						tone(600, 100, 10, -1);
					}
					if (enemy[g].timer) {
						enemy[g].timer--;
					}				
				break;		
	
				case enemyBossUFO:;							//Boss patterns here

					static uint8_t spinSpeed;
									
					if (frameCounter & spinSpeed) {
						enemy[g].mirror = 1;
					}
					else {
						enemy[g].mirror = -1;
					}	
								
					if (blasted) {
						blasterX = spriteOff;				
						damageBoss(g);
						spawnExplosion(enemy[g].x, enemy[g].y, 5);
					}
					
					if (enemy[g].y == spriteOff) {								//Did a check above "kill" this enemy? Skip logic checks, goto next enemy in array
						continue;
					}	
									
					if (xCollide && yCollide && (playerState & playerAlive)) {
						killPlayer();
					}	
													
					switch(enemy[g].state) {
					
						case 1:								//Initial descent
							spinSpeed = 0x08;
							if (frameCounter & 1) {
								if (++enemy[g].y == 36) {
									enemy[g].state = 2;			//Next state
									enemy[g].timer = 30 + (getRandom(0x07) * 7);		//2 seconds of next stage
									wavePointer = 95;
								}
							}
						break;
						case 2:								//Pause
							if (++wavePointer == 128) {
								wavePointer = 0;
							}
							enemy[g].y = 36 - (sineWave[wavePointer] >> 2);	
												
							if (!--enemy[g].timer) {
								enemy[g].state = 3;
								enemy[g].timer = 60;		//1 seconds between bombs
								enemy[g].count = 5 + stage + getRandom(0x03);		//Drop this many bombs	
							}
						break;					
						case 3:								//Bombing
							if (++wavePointer == 128) {
								wavePointer = 0;
							}
							enemy[g].y = 36 - (sineWave[wavePointer] >> 2);
							
							if (!--enemy[g].timer) {
								if (--enemy[g].count) {	//Bombs left to drop?
									spawnShot(enemy[g].x + 12, enemy[g].y + 8, enemyBombShot);
									enemy[g].timer = 90 - (stage * 3);					//+ (getRandom(0x03) * 3) 				
								}
								else {
									enemy[g].state = 4;		//Wind up..
									spinSpeed = 0x02;
									enemy[g].timer = 70;	//Timer
									enemy[g].dir = 0;
								}
							}
						break;					
						case 4:								//Winding up						
							if (enemy[g].y > 8) {
								if (++enemy[g].dir == 4) {
									enemy[g].y--;
									enemy[g].dir = 0;
								}
							}
							if (!--enemy[g].timer) {
								//branchState here
								enemy[g].state = 5;
							}
						break;	
						case 5:								//Slam to ground
							enemy[g].y += 2;
							if (++enemy[g].y > 54) {
								enemy[g].y = 55;
								enemy[g].state = 6;
								enemy[g].timer = 40;		//Wind up for lateral move
							}
						break;
						case 6:								//Wind up on ground
							if (frameCounter & 1) {
								enemy[g].x++;
							}
							if (!--enemy[g].timer) {
								enemy[g].state = 7;
							}
						break;						
						case 7:								//On ground
							enemy[g].x -= 2;				//Fast move
							if (enemy[g].x < -32) {			//Scrolled past?
								enemy[g].state = 8;
								enemy[g].x = -128 + (getRandom(0x07) * 6);		//Scroll it further left (to cause a delay for return)
								enemy[g].y = (getRandom(0x03) * 3) + 8;
								enemy[g].grounded = 0;				//Not stuck to ground
								spinSpeed = 0x04;
								enemy[g].timer = 30;
							}
						break;
						case 8:								//Scrolling back to the right
							if (enemy[g].x > 0) {
								if (--enemy[g].timer == 0) {
									spawnShot(enemy[g].x + 12, enemy[g].y + 8, enemyWaveShot);
									enemy[g].timer = 40 - (stage * 4);	
								}
							}
							if (++enemy[g].x > 94) {		//Back to right side? Repeat
								enemy[g].state = 1;
								bossPhase++;
							}
						break;																														
					}				

				break;
						
			}		
		}	
	}

}

void checkHitPlayer(uint8_t which) {

	if (xCollide && yCollide && (playerState & playerAlive)) {
		killPlayer();
		enemy[which].y = spriteOff;
	}
	
}

void checkObstacle(uint8_t which, uint8_t yHit, uint16_t value) {			//Check for obstable collision and jump score status

	if (xCollide && (playerState & playerAlive)) {							//Ground obstacle and player X collision?
	
		if (bugY > yHit) {				//Buggie low enough to hit it?			
			killPlayer();
		}
		else {
			jumpScore = value;			//Set jump score value. If player lands without dying, they make these points
		}
		
	}

}

void damageBoss(uint8_t g) {

	tone(400, 100, 10, 5);
	score += 250;
	
	if (--bossHealth == 0) {
		spawnExplosion(enemy[g].x, enemy[g].y, 20);			//Twin explosions!
		spawnExplosion(enemy[g].x + 16, enemy[g].y, 24);
		enemy[g].y = spriteOff;							//Despawn UFO
		stageWin = 1;									//Flag for stage win
		playerTimer = 180;								//Timer for transition
		drawText(" EXTRA LIFE!  ", tileMap + 192 + 18);
	}	
	
}

uint8_t getGround() {

	uint8_t randomSlope = getRandom(0x01);			//Ground either slopes up or down
	
	if (randomSlope) {							//Positive? Slope up
		if (groundSlope > 0) {
			groundSlope--;
		}	
	}
	else {
		if (groundSlope < 2) {
			groundSlope++;
		}		
	}
	
	return groundSlope;
	
}

void drawHighScores() {
	
	cls();
	setWindow(0, 0);
	tileMap[0] = '<';
	tileMap[15] = '>';
	
	drawText("TOP SCORES", tileMap + 3);

	uint8_t eepromPointer = 0;
	uint8_t mapPointer = 64;

	for (int place = 0 ; place < 6 ; place++) {

		tileMap[mapPointer] = place + 49;			//Draw place #

		for (int x = 0 ; x < 3 ; x++) {
			tileMap[mapPointer + x + 2] = eepromReadByte(eepromPointer + x);
		}
		
		drawDecimal(eepromReadLong(eepromPointer + 4), tileMap + mapPointer + 6);
		
		mapPointer += 32;
		eepromPointer += 32;
		
	}
	
	isDrawn = 1;

}

void drawOptions() {
	
	cls();
	setWindow(0, 0);
	tileMap[0] = '<';
	tileMap[15] = '>';
	
	drawText("OPTIONS", tileMap + 5);

	drawText("B+A=SLEEP", tileMap + (2 * 32));
	drawText("(MENU TO WAKE)", tileMap + (3 * 32));	
	
	drawText("B+DOWN=CLEAR", tileMap + (5 * 32));
	drawText("HIGH SCORES", tileMap + (6 * 32) + 5);
	
	if (gamePad & dDown && gamePad & dB) {
		writeDefaultHighScores();
	}
			
	if (gamePad & dB && gamePad & dA) {
		gotoSleep();
	}
		
}

void drawTime() {

	cls();
	setWindow(0, 0);	
	uint8_t temp = hours;
	
	if (hours > 9) {
		drawTimeDigit(0, 3, 1);	
	}	
	
	temp %= 10;
	drawTimeDigit(3, 3, temp);	
	
	if (seconds & 1) {
		drawTimeDigit(7, 3, 0x0A);
	}	
	
	temp = minutes;

	if (temp > 9) {
		drawTimeDigit(9, 3, temp / 10);
	}
	else {
		drawTimeDigit(9, 3, 0);	
	}

	temp %= 10;	
	drawTimeDigit(13, 3, temp);

	if (playerTimer) {
		playerTimer--;
		
	
		tileMap[0] = '<';
		tileMap[15] = '>';		
		
		switch (changeTime) {
			case 0:
			drawText("A = ROTATE", tileMap + 34);
			drawText("B = SET TIME", tileMap + 2);
			break;
			case 1:
			drawText("]^ HOURS", tileMap + 67);
			break;
			case 2:
			drawText("MINUTES ]^", tileMap + 68);
			break;
		}	
		
		if (changeTime) {
			drawText("B = NEXT", tileMap + 2);
			if (buttonsPressed & dUp) {
				playerTimer = 300;
				if (changeTime == 1) {
					if (++hours == 13) {
						hours = 1;
					}
				}
				else {
					if (++minutes == 60) {
						minutes = 0;
					}
				}
			}
			if (buttonsPressed & dDown) {
				playerTimer = 300;
				if (changeTime == 1) {
					if (--hours == 0) {
						hours = 12;
					}
				}
				else {
					if (--minutes == 255) {
						minutes = 59;
					}
				}
			}
		}		
		
				
	}




	
}

void drawTitle() {
	
	cls();

	drawMountains2(5);
	setWindow(0, 0);

	tileMap[0] = '<';
	tileMap[15] = '>'; 
				
	drawText("MOON FORCE", tileMap + 3);	
	drawText("A = START GAME", tileMap + (3 * 32) + 1);
	drawText("MGC 2021", tileMap + (7 * 32) + 5);
	drawText("BY BEN HECK", tileMap + (7 * 32) + 21);
		
	isDrawn = 1;
		
}

void drawNameEntryScreen() {
	
	cls();

	setWindow(0, 0);
	
	xWindow = 0;

	memset(&name, 124, 3);						//Change name array to underline

	findScorePosition();						//Check if we got a high score (top 6)
	
	if (scoreRanking == 0xFF) {					//No? Back to title screen
		gameState = stateTitle;
		isDrawn = 0;;
		return;
	}						

	drawText("HIGH SCORE #", tileMap + 1);
	tileMap[14] = scoreRanking + 48;	
	
	drawDecimal(score, tileMap + (32 * 3) + 4);
	
	tileMap[(32*6) + 7] = ']';
	
	bugX = 65;
	bugY = 0;			//CHAR position in name build array

	drawText("B=RUB    A=ENTER", tileMap + (32 * 7));

	playerTimer = 0;

	isDrawn = 1;	

}

void drawCharacterRow() {

	uint8_t temp = bugX;

	for (int x = 0 ; x < 9 ; x++) {
		tileMap[167 + x] = temp;
		if (++temp > 90) {
			temp = 65;
		}
	}

	temp = bugX;

	for (int x = 0 ; x < 7 ; x++) {
		if (--temp < 65) {
			temp = 90;
		}
		tileMap[166 - x] = temp;
	}

}

void findScorePosition() {

	scoreRanking = 0xFF;							//You're a LOSER flag

	for (int x = 5 ; x > -1 ; x--) {

		if (score >= eepromReadLong((x * 32) + 4)) {		//See what position this player is against EEPROM scores
			scoreRanking = x + 1;						//Player is at LEAST this good (new equal scores overwrite old ones)
		}
		else {
			break;									//Not higher than what's in EEPROM? Break		
		}		
	}

}

void startNewLife() {

	distance = 0;			//How many screens to the next goal
	frameCounter = 0;
	gameState = stateGetReady;
	playerState = playerDriving;	

	stageWin = 0;
	frameCounter = 0;

	spawnPointer = 224 + 20;

	bugX = 16;
	bugY = 51;
	jump = 0;
	blasterX = spriteOff;						//Disabled state
	
	clearShots();
	clearEnemies();
	bossBattle = 0;
	
	jumpScore = 0;
	speed = 1;
		
}

void killPlayer() {

	playerFrame = 0;
	playerState = playerExploding;
	playerTimer = 5;
	
}

void clearShots() {
	
	for (int g = 0 ; g < 2 ; g++) {
		heroShots[g].y = spriteOff;
	}

}

void clearEnemies() {

	for (int g = 0 ; g < maxEnemies ; g++) {
		enemy[g].y = spriteOff;
		enemy[g].mirror = 1;
		enemy[g].grounded = 0;			
	}
	
	enemyPointer = 0;
	lastObstacleSpawned = 0;
			
}

void drawMoonscape() {

	cls();
	
	tileMap[0] = 11;					//Draw stars
	tileMap[5] = 137;
	tileMap[7] = 134;	
	tileMap[12 + 32] = 134;
		
	tileMap[34] = 11;
	tileMap[38] = 137;
	tileMap[41] = 134;

	tileMap[15] = 11;					//Draw stars
	tileMap[30] = 137;
	
	tileMap[11] = 'l';					//Draw the SUN. My favorite planet			
			
	fillTiles((14 - stage), 26, 3);						//Draw Earthrise
	fillTiles((14 - stage) + 32, 29, 3);
	
	drawMountains2(2);

	//Draw near mountains or moon city

	if (stage & 1) {								//1 (odd) = Mountains

		fillTiles(5 * 32, 3, 13);					//Foreground mountains
		fillTiles(4 * 32, 17, 2);
		fillTiles((4 * 32) + 7, 19, 2);
		fillTiles((4 * 32) + 11, 21, 5);
			
	}
	else {											//0 (even) = Moon City		
		for (int x = 160 ; x < 192 ; x++) {
			tileMap[x] = 124;
		}
		fillTiles(4 * 32, 96, 3);
		fillTiles((5 * 32), 96 + 16, 3);
		
		fillTiles((4 * 32) + 4, 99, 2);
		fillTiles((5 * 32) + 4, 99 + 16, 2);
		
		fillTiles((4 * 32) + 7, 101, 2);
		fillTiles((5 * 32) + 7, 101 + 16, 2);

		fillTiles((4 * 32) + 10, 103, 3);
		fillTiles((5 * 32) + 10, 103 + 16, 3);
		
		fillTiles((4 * 32) + 13, 106, 2);
		fillTiles((5 * 32) + 13, 106 + 16, 2);	
	}

	for (int x = 128 ; x < 144 ; x++) {				//Dupe background to other screen
		tileMap[x + 16] = tileMap[x];
		tileMap[x + 48] = tileMap[x + 32];
	}

	for (int x = 0 ; x < 32 ; x++) {
		tileMap[(7 * 32) + x] = getGround();
	}
	
	xWindow = 0;
	
	clearShots();
	clearEnemies();

	drawText("CHECKPOINT (0)", tileMap + 192 + 18);
	tileMap[192 + 30] = checkPoint + 65;
	messageFlash = 0;
	
}

void drawMountains2(uint8_t vertRow) {

	vertRow *= 32;

	fillTiles(vertRow, 8 * 16, 16);				//Draw background mountains
	fillTiles(vertRow + 16, 8 * 16, 16);
	
	tileMap[vertRow + 6] = 32;					//Erase random stars from BG mountains memory
	tileMap[vertRow + 9] = 32;
	tileMap[vertRow + 10] = 32;
	
	tileMap[vertRow + 6 + 16] = 32;				//Erase random stars from BG mountains memory
	tileMap[vertRow + 9 + 16] = 32;
	tileMap[vertRow + 10 + 16] = 32;

	vertRow += 32;
	
	fillTiles(vertRow, 9 * 16, 16);				//Draw lower portion of rear mountains
	fillTiles(vertRow + 16, 9 * 16, 16);
	
}

void fillTiles(uint8_t location, uint8_t startingTile, uint8_t howMany) {
	
	for (int x = location ; x < location + howMany ; x++) {
		tileMap[x] = startingTile++;
	}
	
}

void drawDecimal(int32_t theValue, uint8_t *rowRAM) {			//Send up to a 9 digit decimal value to memory

	int zPad = 0;							//Flag for zero padding
	uint32_t divider = 100000000;			//Divider starts at 900 million = max decimal printable is 999,999,999

	for (int xx = 0 ; xx < 9 ; xx++) {		//9 digit number
		if (theValue >= divider) {
			*rowRAM++ = '0' + (theValue / divider);
			theValue %= divider;
			zPad = 1;
		}
		else if (zPad || divider == 1) {			
			*rowRAM++ = '0';
		}
		divider /= 10;
	}
}

void drawText(const char *text, uint8_t *rowRAM) {

	while(*text) {
		*rowRAM++ = *text++;		
	}
	
}

void drawTimeDigit(int xPos, int yPos, int theDigit) {
	
	uint8_t pointer = (theDigit * 3);
	uint8_t tilePointer;

	for (int y = 0 ; y < 3 ; y++) {

		tilePointer = (yPos * 32) + xPos + y;			//Get yPointer
		
		for (int x = 0 ; x < 5 ; x++) {
			if (makeDigits[pointer] & (1 << x)) {
				tileMap[tilePointer + (x * 32)] = 0;
			}
			else {
				tileMap[tilePointer + (x * 32)] = 32;
			}
		}			

		pointer++;	
		
	}

}

void cls() {

	memset(tileMap, 32, 256);					//Erase tiles to the blank space character	

}

uint8_t eepromReadByte(uint8_t addr) {										//can read without anything special, just need to adjust address
	return *(volatile uint8_t*)(EEPROM_START + addr);	
}

uint32_t eepromReadLong(uint8_t addr) {										//can read without anything special, just need to adjust address
	
	return *(volatile uint32_t*)(EEPROM_START + addr);
	
}

void eepromWrite(uint8_t addr, char *initials, uint32_t theScore) {

	while( NVMCTRL.STATUS & NVMCTRL_EEBUSY_bm );					//wait while ee busy

	*(volatile uint8_t*)(EEPROM_START + addr++) = *initials++;					//write word to page buffer
	*(volatile uint8_t*)(EEPROM_START + addr++) = *initials++;					//write word to page buffer	
	*(volatile uint8_t*)(EEPROM_START + addr++) = *initials;					//write word to page buffer

	*(volatile uint16_t*)(EEPROM_START+addr + 1) = theScore;			//write word to page buffer
	*(volatile uint16_t*)(EEPROM_START+addr + 3) = theScore >> 16;		//write word to page buffer

	CCP = CCP_SPM_gc; //unlock spm
	NVMCTRL.CTRLA = NVMCTRL_CMD_PAGEERASEWRITE_gc;					//ERWP command, erase and write 32 byte page
	
}

void writeDefaultHighScores() {

	uint32_t fillerScore = 90000;

	name[0] = 'A';
	name[1] = 'B';
	name[2] = 'C';		

	for (int place = 0 ; place < 7 ; place++) {
		eepromWrite(place * 32, name, fillerScore);			//Write little endian name and score
		fillerScore -= 10000;	
	}
		
}

ISR(TCB0_INT_vect) {				//Timer trips? (This timer is disabled in Power-Down sleep mode)

	frameFlag++;					//Once 16 ms has passed, run frame (60 FPS... ish)
	toneLogic();					//Sound checks
	TCB0_INTFLAGS = 1;				//Clear flag
	
}

ISR(PORTA_PORT_vect) {				//PORTA ISR tied to Menu button for sleep wake-up

	if (sleepState) {				//Button change state with sleep mode enabled? WAKE UP! 
		sleepState = 0;
		displayOnOff(1);			//Turn display back on
		if (gameState == statePaused) {		//Did we sleep while paused? Super hacky-hack where we resume the game, so the MENU flag will take us right back into the pause screen. HACKY AS FUCK!
			gameState = stateRunning;
		}
	}
	
	PORTA.INTFLAGS = 0x20;			//Clear interrupt flag

}

ISR(RTC_PIT_vect) {						//ISR called every second

	if (++seconds == 60) {				//time lord stuff
		seconds = 0;
		if (++minutes == 60) {
			minutes = 0;
			++hours;
			//if (hours == 12) {
				//amPM++;					//amPM & 0x01 -> 0 = AM 1 = PM
			//}
			if (hours == 13) {			//Time is so stupid. It's a bunch of rocks spinning around a star.
				hours = 1;
			}
		}
	}

	if (!sleepState) {					//If awake, change flag to indicate the seconds changed (for the display)
		secFlag = 1;
	}

	RTC.PITINTFLAGS = RTC_PI_bm;

}