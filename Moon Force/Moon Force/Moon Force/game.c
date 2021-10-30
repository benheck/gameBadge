#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#define F_CPU 10000000UL
#include <util/delay.h>
#include "drivers/gameBadge.h"
#include "game.h"
#include <string.h>

uint8_t tileMap[256];							//Tilemap ram (2 screens of 16x8 tiles)
static volatile uint8_t sleepState = 0;			//0 = normal 1 = asleep
static volatile uint8_t frameFlag;				//ms Frame counter to decide when a frame should be drawn

static volatile uint8_t secFlag = 0;			//This flag is set when a seconds transition occurs
static volatile uint8_t day = 0;				//0 = Sunday... 7 - Saturday
static volatile uint8_t amPM = 0;				//am = 0 pm = 1
static volatile uint8_t hours = 11;				//The hours
static volatile uint8_t minutes = 5;			//The minutes
static volatile uint8_t seconds = 0;			//The seconds

//Your flash game data here--------------------------------------------------------------------------------

#define maxEnemies			16

const char PROGMEM tileData[] = {
0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0x13, 0x0b, 0x05, 0x02, 0x00, 0x00, 0x01, 0x03, 0x01, 0x00, 0x04, 0x00, 0x01, 0x01, 0x01, 0x12, 0x02, 0x02, 0x04, 0x04, 0x04, 0x48, 0x08, 0x08, 0x88, 0x90, 0x90, 0xb0, 0xb0, 0x98, 0x98, 0x98, 0x58, 0x4c, 0x2c, 0x26, 0x16, 0x13, 0x03, 0x07, 0x07, 0x03, 0x06, 0x0a, 0x02, 0x02, 0x14, 0x04, 0x44, 0x4c, 0x4c, 0x2c, 0x26, 0x16, 0x13, 0x0b, 0x09, 0x05, 0x04, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x09, 0x01, 0x02, 0x06, 0x16, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x13, 0x13, 0x0b, 0x09, 0x09, 0x09, 0x09, 0x05, 0x04, 0x04, 0x04, 0x04, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xe0, 0xc0, 0x80, 0x80, 0xc0, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xc0, 0xc0, 0x60, 0x30, 0x70, 0x78, 0x30, 0x30, 0xa0, 0x20, 0x40, 0x40, 0x80, 0x80, 0x80, 0x80, 0x80, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x60, 0x60, 0x60, 0x60, 0x30, 0x30, 0x30, 0xb0, 0x98, 0x58, 0x4c, 0x4c, 0x0c, 0x0e, 0x0e, 0x0c, 0x08, 0x28, 0x08, 0x08, 0x08, 0x50, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x40, 0x40, 0x80, 0x80, 0x00, 0x00, 0x40, 0x50, 0xb8, 0x2c, 0x7c, 0xfe, 0xfe, 0xf7, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe, 0xfc, 0xfc, 0xf8, 0xf0, 0xc0, 0x00, 0x00, 0x01, 0x00, 0x15, 0x02, 0x49, 0x12, 0x0a, 0x0a, 0x24, 0x15, 0x15, 0x09, 0x0a, 0x23, 0x0b, 0x8b, 0x0b, 0x23, 0x0a, 0x23, 0x0b, 0x23, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xde, 0xde, 0xde, 0x00, 0x00, 0x00, 0x1e, 0x1e, 0x00, 0x00, 0x1e, 0x1e, 0x00, 0x00, 0x6c, 0xfe, 0xfe, 0x6c, 0xfe, 0xfe, 0x6c, 0x00, 0x48, 0x5c, 0x54, 0xfe, 0x54, 0x74, 0x24, 0x00, 0xc6, 0xe6, 0x70, 0x38, 0x1c, 0xce, 0xc6, 0x00, 0x60, 0xf4, 0x9e, 0xba, 0x6e, 0xe4, 0xa0, 0x00, 0x20, 0x18, 0x18, 0x0c, 0x18, 0x18, 0x20, 0x00, 0x00, 0x00, 0x7c, 0xfe, 0xc6, 0x82, 0x00, 0x00, 0x00, 0x82, 0xc6, 0xfe, 0x7c, 0x00, 0x00, 0x10, 0x54, 0x7c, 0x38, 0x38, 0x7c, 0x54, 0x10, 0x00, 0x18, 0x18, 0x7e, 0x7e, 0x18, 0x18, 0x00, 0x00, 0x00, 0x80, 0xe0, 0xe0, 0x60, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x04, 0x00, 0x00, 0x7c, 0xfe, 0xfe, 0xfe, 0x82, 0xfe, 0x7c, 0x00, 0x00, 0x0c, 0xfc, 0xfe, 0xfe, 0xfe, 0x00, 0x00, 0xc4, 0xe6, 0xf2, 0xde, 0xde, 0xde, 0xcc, 0x00, 0x44, 0xd6, 0x92, 0xfe, 0xfe, 0xfe, 0x7c, 0x00, 0x3e, 0x3e, 0x30, 0x3e, 0xfe, 0xfe, 0x30, 0x00, 0x5e, 0xde, 0xda, 0xda, 0xda, 0xfa, 0x72, 0x00, 0x7c, 0xfe, 0x8a, 0xfa, 0xfa, 0xfa, 0x72, 0x00, 0x02, 0x02, 0xf2, 0xfa, 0xfe, 0x0e, 0x06, 0x00, 0x6c, 0xfe, 0x92, 0xfe, 0xfe, 0xfe, 0x6c, 0x00, 0x1c, 0xbe, 0xa2, 0xfe, 0xfe, 0xfe, 0x7c, 0x00, 0x00, 0x00, 0x6c, 0x6c, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x80, 0xec, 0xec, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x10, 0x38, 0x7c, 0xfe, 0x00, 0x00, 0x00, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x00, 0x00, 0x00, 0xfe, 0x7c, 0x38, 0x10, 0x00, 0x00, 0x00, 0x0c, 0x06, 0xd2, 0xda, 0xde, 0x0e, 0x04, 0x00, 0x7c, 0xfe, 0xc6, 0xba, 0xaa, 0xb6, 0xbc, 0x00, 0xfc, 0xfe, 0x12, 0xfe, 0xfe, 0xfe, 0xfc, 0x00, 0xfe, 0xfe, 0xfe, 0xfe, 0x92, 0xfe, 0x6c, 0x00, 0x7c, 0xfe, 0xfe, 0xfe, 0x82, 0xc6, 0x44, 0x00, 0xfe, 0xfe, 0xfe, 0xfe, 0x82, 0xfe, 0x7c, 0x00, 0xfe, 0xfe, 0xfe, 0xfe, 0x92, 0x92, 0x82, 0x00, 0xfe, 0xfe, 0xfe, 0xfe, 0x12, 0x12, 0x02, 0x00, 0x7c, 0xfe, 0xfe, 0xfe, 0x82, 0xa6, 0xe4, 0x00, 0xfe, 0xfe, 0x10, 0xfe, 0xfe, 0xfe, 0xfe, 0x00, 0x82, 0xfe, 0xfe, 0xfe, 0xfe, 0x82, 0x00, 0x00, 0x40, 0xc2, 0x82, 0xfe, 0xfe, 0x7e, 0x02, 0x00, 0xfe, 0xfe, 0xfe, 0xfe, 0x38, 0xee, 0xc6, 0x00, 0xfe, 0xfe, 0xfe, 0xfe, 0x80, 0x80, 0x80, 0x00, 0xfe, 0xfe, 0x1e, 0xfc, 0x1e, 0xfe, 0xfe, 0x00, 0xfe, 0xfe, 0xfe, 0x1c, 0x38, 0xfe, 0xfe, 0x00, 0x7c, 0xfe, 0x82, 0xfe, 0xfe, 0xfe, 0x7c, 0x00, 0xfe, 0xfe, 0xfe, 0xfe, 0x22, 0x3e, 0x1c, 0x00, 0x7c, 0xfe, 0xa2, 0xfe, 0xfe, 0xfe, 0x7c, 0x00, 0xfe, 0xfe, 0xfe, 0xfe, 0x22, 0xfe, 0xdc, 0x00, 0x4c, 0xde, 0x9e, 0xfe, 0xf2, 0xf6, 0x64, 0x00, 0x02, 0x02, 0xfe, 0xfe, 0xfe, 0x02, 0x02, 0x00, 0x7e, 0xfe, 0x80, 0xfe, 0xfe, 0xfe, 0x7e, 0x00, 0x3e, 0x7e, 0xe0, 0xfe, 0xfe, 0x7e, 0x3e, 0x00, 0x7e, 0xfe, 0xe0, 0x7c, 0xe0, 0xfe, 0x7e, 0x00, 0xc6, 0xee, 0x7c, 0x38, 0x7c, 0xee, 0xc6, 0x00, 0x0e, 0x1e, 0xfe, 0xf8, 0xfe, 0x1e, 0x0e, 0x00, 0xe2, 0xf2, 0xfa, 0xfe, 0xbe, 0x9e, 0x8e, 0x00, 0x10, 0x38, 0x7c, 0x10, 0x10, 0x10, 0x10, 0x00, 0x60, 0x92, 0x64, 0x02, 0xf1, 0x61, 0x91, 0x00, 0x20, 0x30, 0x38, 0x3c, 0x38, 0x30, 0x20, 0x00, 0x10, 0x30, 0x70, 0xf0, 0x70, 0x30, 0x10, 0x00, 0x30, 0x78, 0xfc, 0xfc, 0x78, 0x30, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0xc0, 0xe0, 0xa0, 0xfc, 0xb8, 0xb8, 0xff, 0xb0, 0xa0, 0xf4, 0xba, 0xa1, 0xa1, 0xa1, 0xa5, 0xae, 0xb4, 0xe0, 0xc0, 0x18, 0x14, 0x34, 0xff, 0x1e, 0xd6, 0x16, 0x1e, 0x1f, 0x17, 0x17, 0x1e, 0x1e, 0xfe, 0xff, 0x3c, 0x00, 0x80, 0xe0, 0xd0, 0xd8, 0xfc, 0xde, 0xd0, 0xf0, 0xe0, 0x80, 0xe0, 0xc0, 0xff, 0xc0, 0x80, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x38, 0x74, 0x56, 0xde, 0x76, 0xd6, 0x5c, 0x7c, 0x38, 0x00, 0x00, 0x90, 0xf0, 0x90, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0xfe, 0x58, 0x58, 0x5f, 0x7a, 0x5e, 0x5a, 0x5e, 0x7e, 0x7f, 0x7f, 0xe0, 0xc0, 0x00, 0x00, 0x06, 0x0f, 0x0f, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x7f, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7f, 0x7f, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7f, 0x7f, 0x00, 0x00, 0x00, 0x80, 0xc0, 0x20, 0x9f, 0x50, 0x51, 0x52, 0x52, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x1c, 0x14, 0x16, 0x23, 0xc1, 0x80, 0x80, 0xfe, 0x01, 0xfc, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xfc, 0xc1, 0x22, 0x17, 0x8a, 0x43, 0x22, 0x13, 0x02, 0x83, 0x42, 0x23, 0x33, 0x3a, 0x3f, 0x77, 0xe3, 0xc0, 0x20, 0x1f, 0x90, 0x48, 0x28, 0x28, 0x08, 0x0c, 0x07, 0x04, 0x07, 0x0c, 0x08, 0x08, 0x08, 0x18, 0x10, 0x3f, 0xe0, 0xc0, 0x80, 0x80, 0x80, 0xff, 0xf0, 0x10, 0x09, 0xc5, 0x23, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x1e, 0xf3, 0xe1, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x18, 0x7e, 0x18, 0x1e, 0xda, 0x1a, 0x7c, 0x18, 0x40, 0x40, 0x40, 0x60, 0x60, 0x40, 0x40, 0x40, 0x0c, 0xcf, 0x0c, 0x0f, 0x3d, 0x0d, 0xce, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xe0, 0x50, 0xa0, 0xc0, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x38, 0x16, 0x68, 0x28, 0x50, 0x60, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x70, 0x28, 0xd0, 0xe0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0xa0, 0x30, 0x38, 0x16, 0x41, 0x22, 0x04, 0x08, 0x70, 0x80, 0x00, 0x80, 0x80, 0x40, 0xa0, 0x40, 0x80, 0x40, 0xb0, 0x08, 0x28, 0x17, 0x4a, 0x3c, 0x9a, 0x34, 0x38, 0x60, 0xc0, 0x00, 0x80, 0x40, 0x01, 0x05, 0x0a, 0x1c, 0x9a, 0x22, 0x11, 0x00, 0x00, 0x05, 0x0a, 0x0f, 0x14, 0x08, 0x10, 0x50, 0x28, 0x94, 0x0a, 0x25, 0x01, 0x0d, 0x02, 0x08, 0x05, 0x01, 0x03, 0x27, 0x06, 0x0d, 0x0e, 0x08, 0x1e, 0x45, 0x1a, 0x0d, 0x10, 0x0a, 0x00, 0x00, 0x00, 0x01, 0x12, 0x07, 0x85, 0x0a, 0x04, 0x08, 0x10, 0x50, 0x28, 0x94, 0x0a, 0x25, 0x11, 0x1e, 0x28, 0x70, 0x50, 0xd0, 0x2c, 0x82, 0x43, 0x01, 0x04, 0x02, 0x40, 0x01, 0x0b, 0x16, 0x3c, 0x70, 0x50, 0xd0, 0x28, 0x94, 0x44, 0x22, 0x01, 0x02, 0x0a, 0x14, 0x38, 0x70, 0x50, 0x88, 0x04, 0x08, 0x28, 0x14, 0x44, 0x34, 0x0a, 0x25, 0x02, 0x09, 0x84, 0x00, 0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x01, 0x84, 0x03, 0x23, 0x03, 0x04, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x82, 0x07, 0x06, 0x09, 
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
0x0f, 0x3f, 0x78, 0xe6, 0xe6, 0x78, 0x3f, 0x0f, 0x07, 0x08, 0x70, 0x84, 0x84, 0x70, 0x08, 0x07, 
};

const char PROGMEM evilBomb[] = {
8,8,
0x30, 0x78, 0xfe, 0xff, 0xff, 0xff, 0x7b, 0x31, 0x00, 0x30, 0x68, 0xf6, 0xfd, 0x79, 0x31, 0x00, 
};

const char PROGMEM vertExplode[] = {
16, 24,

0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xf8, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0c, 0xfc, 0xf0, 0xff, 0xff, 0xff, 0xe0, 0xe0, 0xe8, 0x18, 0x00, 0x00, 0x80, 0x70, 0x70, 0xf1, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0x7f, 0xfe, 0xef, 0xe3, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x04, 0x30, 0xe0, 0x80, 0xff, 0xe0, 0x80, 0xc0, 0x60, 0x08, 0x00, 0x00, 0x80, 0x20, 0x20, 0xa0, 0x52, 0x74, 0x7d, 0xff, 0x37, 0x3f, 0x7d, 0xbf, 0x3e, 0x6c, 0x42, 0x40, 0x00, 0x00, 0x40, 0xc0, 0x84, 0x7c, 0xf8, 0xc0, 0xfe, 0xfe, 0xf0, 0x7e, 0x8e, 0xf8, 0xfe, 0x06, 0x00, 0x00, 0x10, 0x71, 0xff, 0x9e, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xfc, 0xcf, 0xf7, 0x3c, 0x0e, 0x02, 0x00, 0x0c, 0x38, 0x71, 0x7f, 0x7f, 0xff, 0x03, 0x07, 0x03, 0xff, 0xff, 0x13, 0x08, 0x04, 0x04, 0x00, 0x40, 0x80, 0x04, 0x78, 0xc0, 0x00, 0x7e, 0x80, 0x00, 0x70, 0x0e, 0x80, 0xf8, 0x06, 0x00, 0x00, 0x10, 0x61, 0x9e, 0x00, 0x93, 0xe0, 0x7c, 0xef, 0xfc, 0xc0, 0x08, 0xc7, 0x30, 0x0c, 0x02, 0x00, 0x08, 0x30, 0x01, 0x06, 0x77, 0x00, 0x00, 0x03, 0x00, 0x01, 0xfe, 0x03, 0x08, 0x00, 0x04, 0x06, 0x0f, 0x23, 0x33, 0xff, 0xee, 0x06, 0x67, 0x83, 0xfb, 0xe2, 0x6e, 0xe7, 0xc3, 0x1f, 0x3e, 0x00, 0x02, 0x33, 0x1b, 0x73, 0xc6, 0x1c, 0x7c, 0x00, 0xff, 0xff, 0x00, 0x78, 0x0c, 0x86, 0x80, 0x00, 0x04, 0x0c, 0x00, 0x00, 0x01, 0x06, 0x00, 0x00, 0x18, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x06, 0x01, 0x21, 0x13, 0x36, 0xe0, 0x02, 0x27, 0x03, 0x7b, 0x22, 0x20, 0x26, 0xc3, 0x07, 0x1e, 0x00, 0x02, 0x11, 0x09, 0x32, 0xc4, 0x18, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x38, 0x04, 0x02, 0x80, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x7f, 0x67, 0xe3, 0xc0, 0x00, 0x04, 0x08, 0x20, 0x10, 0x41, 0xe1, 0xe7, 0xff, 0xfe, 0x7c, 0x00, 0x07, 0x0f, 0x0c, 0x08, 0x40, 0x82, 0x04, 0x00, 0x00, 0x40, 0x20, 0x09, 0x0f, 0x0e, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x06, 0x06, 0x3c, 0x66, 0x43, 0xc1, 0x80, 0x00, 0x04, 0x00, 0x00, 0x10, 0x00, 0x41, 0xc1, 0xe6, 0x7c, 0x18, 0x00, 0x07, 0x0c, 0x08, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x08, 0x06, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x04, 	

};

const int jumpVelocity[] = {-3, -2, -1, -1, 0, -1, 0, 0, 0, 0, 1, 0, 1, 1, 2, 3};

//Your game variables (RAM) here-------------------------------------------------------------------------------
uint8_t gamePad;
uint16_t padSeed;

uint8_t attackSpeed = 150;					//Frames between each enemy attacking player. Lower = harder

uint8_t xWindow = 0;
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
int playerTimer = 0;
int playerFrame = 0;

uint8_t spawnFineCounter = 0;
uint8_t spawnPointer = 224 + 20;

uint8_t groundSlope = 1;					//The next block of terrain to be drawn 0 = full, 1 = one pixel lower, 2 = 2 pixels lower

uint8_t speed = 1;

int16_t collide = 0;

uint8_t stage;
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
uint32_t score;
uint8_t lives;
uint8_t stagePhase = phaseNone;					//Stage phase (none)

uint16_t messageFlash = 0xFFFF;

struct shotData {					//Vertical player and enemy shots
	int x;
	int y;
	int type;	
};

struct shotData heroShots[3];
uint8_t shotPointer = 0;

struct enemyData {
	const char *gfx;
	int16_t x;
	int8_t y;
	uint8_t type;
	int8_t mirror;
	int8_t dir;
	int8_t state;
	uint8_t frame;
	uint8_t grounded;
	uint16_t timer;	
};

struct enemyData enemy[maxEnemies];
uint8_t enemyPointer = 0;
const int16_t enemySpawnSide[2] = {-16, 128};

int8_t blasterX = spriteOff;						//Disabled state
uint8_t blasterY = 0;

uint8_t buttonFire = 0;

uint8_t gameState; // = stateTitle; //stateRunning;

uint8_t isDrawn;

uint8_t obstacleTimer = 10;								//Spacing (in frames) between the spawning of obstacles. Lower= harder
uint8_t obstacleTimerBase = 80;							//Default timer base Lower= harder


void gameSetup() {							//This function is called once per boot/program

	setScrollDirection(horizontal);
	gameState = stateTitle;
		
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

	if ((gamePad & dMenu) && gameState & canPauseMask) {		//MENU to pause/unpause?
		if (buttonFire & dMenu) {			//Bit set saying we can press this button again?
			buttonFire &= !dMenu;			//Clear bit
			if (gameState == statePaused) {	//Change state
				gameState = stateRunning;
			}
			else {
				gameState = statePaused;
				frameCounter = 0x20;			//Clean transition to blinking PAUSED (pedantic, I know)
			}
		}
	}
	else {
		buttonFire |= dMenu;
	}

	switch(gameState) {

		case stateTitle:
			if (!isDrawn) {							//First time here? Draw the title screen
				drawTitle();
			}			
			screenLoad();							//Dump the frame buffer to the OLED
			drawTiles(tileData, tileMap);			//Draw background tiles
			
			if (++frameCounter & 1) {
				xWindow++;						
			}						
			setRowScroll(xWindow >> 1, 5);		//WE SCROLL THE MOON!
			setRowScroll(xWindow >> 1, 6);		//BECAUSE IT IS CLOSE TO US!
			setRowScroll(xWindow, 7);			//WE SCROLL THE MOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOON!
			
			if (gamePad & dA) {
				if (buttonFire & dA) {			//Start game?
					buttonFire &= !dA;			//Clear bit
					startNewGame();			
				}
			}
			else {
				buttonFire |= dA;
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
				cls();
				gameState = stateTitle;
				isDrawn = 0;
			}		
		break;
	
	}

}

void gameStatus(uint8_t type) {				//Draw status screen. Type 0 = READY, 1 = PAUSE screen

	csLow();

	frameCounter++;
	
	if ((frameCounter & 0x20) && type == 0) {
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
	uint8_t subDistance = 0;
	if (messageFlash == 0xFFFF) {
		subDistance = 1;
	}
	int8_t buggyPosX = 15 + (checkPoint * 3) + subDistance;
	
	if (frameCounter & 0x08) {						//Animated buggie on map (cuuuuuuuuuuuuuuuute!)
		tileMap[buggyPosX] = 125;
	}	
	else {
		tileMap[buggyPosX] = 127;
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
		buttonFire = dMenu;						//Clear this bit so pressing Menu doesn't exit PAUSE when we awaken from our dark, deep slumber
		gotoSleep();
	}
	
}

void nextRow() {

	rowLoad(tileData, tileMap + 16, draw);
	memset(&tileMap[16], ' ', 16);
	
}

void gameAction() {

	screenLoad();							//Dump the frame buffer to the OLED
	frameCounter++;							//Increment counter
	drawTiles(tileData, tileMap);			//Draw background tiles

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
			if (--obstacleTimer == 0) {											//Time to spawn?
				obstacleTimer = obstacleTimerBase + (getRandom(0x07) * 16);		//Reset spawn timer + random (work on scaling difficulty)
				spawnObstacle(127, 1 << getRandom(0x03));							//Spawn obstacle
			}
			drawDecimal(obstacleTimer, tileMap);
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
	
	if (xWindow == 255 && messageFlash == 0xFFFF) {
		
		distanceCheck = 1;
		
		//drawDecimal(distance, tileMap);
		//drawDecimal(distanceGoal, tileMap + 32);		
	}
	
	if (distanceCheck) {
		
		distance += byDistance;					//Does simply scrolling the screen advance you?
		
		if (distance == distanceGoal) {
			distance = 0;
			checkPoint++;
			drawText("CHECKPOINT", tileMap + 192 + 18);
			tileMap[192 + 30] = checkPoint + 65;
			messageFlash = 0;
		}	
			
	}

	bgScroll += (frameCounter & 1);

	setRowScroll(0, 0);
	setRowScroll(0, 1);
	setRowScroll(bgScroll >> 1, 2);
	setRowScroll(bgScroll >> 1, 3);
	setRowScroll(bgScroll, 4);
	setRowScroll(bgScroll, 5);
	//setRowScroll(xWindow, 6);
	setRowScroll(xWindow, 7);
	
	
}

void phaseStart() {				//After CHECKPOINT message scrolls off, decide what to do in this phase (could also be a boss if I have time)

	skyEnemies = 0;
	byDistance = 0;				//Goal based (not scroll based) advancement unless otherwise noted
	distance = 0;
	distanceGoal = 10;			
	
	
	//if (stage == 1 && checkPoint == 0) {			//Always start with rocks
		//stagePhase = phaseObstacles;
		//obstacleTimerBase = 60;
		//obstacleTimer = obstacleTimerBase;
		//setGoal(4);	
		//return;
	//}

	obstacleTimer = 1;

	switch(checkPoint) {
					
		case 0:
			stagePhase = phaseObstacles;
			eventGoal = 40;

			numberSpawned = 0;
			spawnPerWave = 2;
			numberOfWaves = 3;
			setGoal(4);	
		break;
				
		case 1:
			stagePhase = phaseUFOs;
			eventGoal = 80;

			numberSpawned = 0;
			spawnPerWave = 2;
			numberOfWaves = 3;
		break;
		
		case 2:
			stagePhase = phaseUFOs | phaseObstacles;
			eventGoal = 70;

			numberSpawned = 0;
			spawnPerWave = 2;
			numberOfWaves = 3;		
		break;	
		
		case 3:
			stagePhase = phaseBalls;
			eventGoal = 70;

			numberSpawned = 0;
			spawnPerWave = 3;
			numberOfWaves = 4;		
		break;	
		
		case 4:
			stagePhase = phaseBalls | phaseUFOs | phaseObstacles;
			eventGoal = 50;

			numberSpawned = 0;
			spawnPerWave = 4;
			numberOfWaves = 5;		
		break;						
		
	}

	eventTimer = 1;				//Make something happen RIGHT AWAY (if that type of phase)
				
}

void setGoal(uint8_t theGoal) {

	distance = 0;
	distanceGoal = theGoal;
	byDistance = 1;
	
}

void playerLogic() {
	
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
			bugX--;
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
				if (lives == 255) {				//Rolls under from to 255?
				//if (--lives == 255) {				//Rolls under from to 255?
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
			tone(10000, 200, 8, 0);
		}
			
	}	

	if (playerState & playerAlive) {				//Controls don't work if you're DEAD!
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
	
		if (gamePad & dDown) {
			if (buttonFire & dDown) {			//Bit set saying we can jump because we released the button?
				buttonFire &= !dDown;			//Clear bit
				if (getRandom(0x01)) {
					spawnEnemy(-16, getRandom(0x01) * 8, 1, enemyUFO);
				}
				else {
					spawnEnemy(128, getRandom(0x01) * 8, -1, enemyBalls);
				}
			
			}
		}
		else {
			buttonFire |= dDown;
		}

		if (gamePad & dA) {
			if (buttonFire & dA) {			//Bit set saying we can jump because we released the button?
				buttonFire &= !dA;			//Clear bit
				if (jump == 0) {			//Jump!
					jump = 1;
				}
			}
		}
		else {
			buttonFire |= dA;
		}



		if (gamePad & dB) {
			if (buttonFire & dB) {			//Bit set saying we can fire because we released the button?
				buttonFire &= !dB;			//Clear bit

				//spawnShot(64, 0, enemyWaveShot);
				//spawnShot(115, 0, enemyBombShot);
			
				if (heroShots[shotPointer].y == spriteOff) {		//This sprite is eligible to be spawn
					heroShots[shotPointer].x = bugX + 2;			//X position
					heroShots[shotPointer].y = bugY - 8;
				
					tone(800, 100, 10, 10);
				
					if (++shotPointer > 1) {
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
		else {
			buttonFire |= dB;
		}		
	}
	


	//addSeed(gamePad);			//Use controls to help random seed
	
}

void spawnObstacle(int16_t xPos, uint8_t whichOne) {

	uint8_t yPos = 48;

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

}

void spawnShot(int8_t x, int8_t y, uint8_t whichType) {
	
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

void spawnEnemy(int16_t x, int8_t y, int8_t dir, uint8_t whichType) {

	switch(whichType) {
		case enemyUFO:	
			enemy[enemyPointer].gfx = ufo;					//This sprite graphic
		break;
		case enemyBalls:
			enemy[enemyPointer].gfx = balls;					//This sprite graphic
		break;		
	}

	enemy[enemyPointer].timer = attackSpeed;
	
	enemy[enemyPointer].x = x;
	enemy[enemyPointer].y = y;
	enemy[enemyPointer].dir = dir;
	
	enemy[enemyPointer].type = whichType;
	enemy[enemyPointer].grounded = 0;				//Not stuck to ground
	
	if (++enemyPointer == maxEnemies) {				//Cycle baddies
		enemyPointer = 0;
	}
		
}

void shotLogic() {
	
	for (int g = 0 ; g < 3 ; g++) {					//Scan through the 3 possible hero and baddie shots (vertical, not the buggie hori blaster)		
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
			
			uint8_t xCollide = 0;													//X and Y collision flags for enemy-player
			uint8_t yCollide = 0;
			uint8_t blasted = 0;													//Collision flag for enemy-blaster (horizontal shot)
			
			if (enemy[g].x < (bugX + 24) && bugX < (enemy[g].x + xSize)) {			//Check the X and Y collision flags for enemies (including shots) and Player Buggie
				xCollide = 1;
			}			

			if (enemy[g].y < (bugY + 8) && bugY < (enemy[g].y + ySize)) {
				yCollide = 1;
			}	

			for (int h = 0 ; h < 3 ; h++) {											//Scan through the 3 possible hero vertical shots and check for collision against enemies
				if (heroShots[h].y != spriteOff) {
					if (enemy[g].x < (heroShots[h].x + 8) && heroShots[h].x < (enemy[g].x + xSize)) {
						if (enemy[g].y < (heroShots[h].y + 8) && heroShots[h].y < (enemy[g].y + ySize)) {
							enemy[g].y = spriteOff;									//Kill enemy
							if (enemy[g].type & skyMask) {							//Was this a sky enemy? (not a projectile)
								skyEnemies--;										//Decrement their counter
							}
							heroShots[h].y = spriteOff;								//Despawn hero shot
							tone(700, 300, 9, -1);	
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
			
				case enemySmallRock:			
					collide = enemy[g].x - blasterX;
					
					//if (collide > -1 && collide < 8 && blasterY > 44) {
					if (blasted) {	
						enemy[g].y = spriteOff;
						blasterX = spriteOff;
						tone(1500, 200, 10, 10);
						score += 100;
					}	

					if (xCollide && (playerState & playerAlive) && bugY > 44) {
						killPlayer();
					}	
											
				break;
				
				case enemyBigRock:

					collide = enemy[g].x - blasterX;
					
					//if (collide > -1 && collide < 16 && blasterY > 44) {		//Blaster hits big rock?
					if (blasted) {
						enemy[g].gfx = smallRock;							//Change the sprite graphics
						enemy[g].type = enemySmallRock;							//It becomes SMALL ROCK
						enemy[g].x += 8;										//Move 8 pix right to make gap
						blasterX = spriteOff;
						tone(400, 100, 10, 3);
						score += 50;
					}	

					if (xCollide && (playerState & playerAlive) && bugY > 44) {
						killPlayer();
					}									
									
				break;
				
				case enemySmallPit:
					if (xCollide && (playerState & playerAlive) && bugY == 51) {
						killPlayer();
					}							
				break;	
				
				case enemyBigPit:
					if (xCollide && (playerState & playerAlive) && bugY == 51) {
						killPlayer();	
					}		
				break;
				
				case enemyWaveShot:
					if (xCollide && yCollide && (playerState & playerAlive)) {
						killPlayer();
						enemy[g].y = spriteOff;
					}
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
					if (xCollide && yCollide && (playerState & playerAlive)) {
						killPlayer();
						enemy[g].y = spriteOff;
					}
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
						
			}		
		}	
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

void drawTitle() {
	
	cls();

	drawMountains2(5);
	setWindow(0, 0);	
	
	drawText("MOON FORCE", tileMap + 3);	
	
	drawText("B = HIGH SCORES", tileMap + (2 * 32));	
	drawText("A = START GAME", tileMap + (3 * 32));

	drawText("MGC 2021", tileMap + (7 * 32) + 5);
	drawText("BY BEN HECK", tileMap + (7 * 32) + 21);
		
	isDrawn = 1;
		
}

void startNewGame() {

	stage = 1;
	checkPoint = 4;			//Which goal you've reached (0=A, 5=F)
	score = 0;
	lives = 2;
	
	startNewLife();
	
}

void startNewLife() {

	distance = 0;			//How many screens to the next goal
	frameCounter = 0;
	gameState = stateGetReady;
	playerState = playerDriving;	

	frameCounter = 0;

	spawnPointer = 224 + 20;

	bugX = 16;
	bugY = 51;
	blasterX = spriteOff;						//Disabled state
	
	clearObjects();
	
}

void killPlayer() {

	playerFrame = 0;
	playerState = playerExploding;
	playerTimer = 5;
	
}

void clearObjects() {
	
	for (int g = 0 ; g < 3 ; g++) {
		heroShots[g].y = spriteOff;
	}	
	
	for (int g = 0 ; g < maxEnemies ; g++) {
		enemy[g].y = spriteOff;
		enemy[g].mirror = 1;
		enemy[g].grounded = 0;
	}
		
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
	
	tileMap[11] = 'l';					//Draw the SUN. My favorite planet			
			
	fillTiles(14 - stage, 26, 3);						//Draw Earthrise
	fillTiles(14 - stage + 32, 29, 3);

	
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
	
	clearObjects();

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
	uint32_t divider = 100000000;			//Divider starts at 900 million

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

void cls() {

	memset(tileMap, 32, 256);					//Erase tiles to the blank space character	
	
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
		gameState = stateRunning;
	}
	
	PORTA.INTFLAGS = 0x20;			//Clear interrupt flag

}

ISR(RTC_PIT_vect) {						//ISR called every second

	if (++seconds == 60) {				//time lord stuff
		seconds = 0;
		if (++minutes == 60) {
			minutes = 0;
			++hours;
			if (hours == 12) {
				amPM++;					//amPM & 0x01 -> 0 = AM 1 = PM
			}
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