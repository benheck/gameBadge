#ifndef GAME_H_
#define GAME_H_

void gameSetup();
void systemLoop();
void gameFrame();
void playerLogic();
void phaseStart();
void setGoal(uint8_t theGoal);
void spawnObstacle(int16_t xPos, uint8_t whichOne);
void spawnEnemy(int16_t x, int8_t y, int8_t dir, uint8_t whichType);
void shotLogic();
void gotoSleep();
void enemies();
uint8_t getGround();
void drawMountains2(uint8_t vertRow);
void drawTitle();
void startNewGame();
void startNewLife();
void killPlayer();
void clearObjects();
void drawMoonscape();
void gameFrame();
void gameStatus(uint8_t type);
void gameAction();
void drawDecimal(int32_t theValue, uint8_t *rowRAM);
void drawText(const char *text, uint8_t *rowRAM);
void nextRow();


void cls();

//YOUR OTHER FUNCTION PROTOTYPES HERE

void fillTiles(uint8_t location, uint8_t startingTile, uint8_t howMany);


#define stateTitle			0x00
#define stateHighScores		0x01
#define stateGetReady		0x09
#define stateGameOver		0x08
#define stateRunning		0x80			//MSB means in these states MENU toggles paused and running
#define statePaused			0x8F
#define canPauseMask		0x80

#define playerShot			1
#define waveShot			10
#define bombShot			20

#define playerDriving		0x01
#define playerJumping		0x02
#define playerAlive			0x03				//Mask for driving/jumping
#define playerExploding		0x10				//Self-explanatory
#define playerDead			0x20				//Delay before going back to the splash screen

#define enemySmallRock		0x01				//Static obstacles
#define enemyBigRock		0x02
#define enemySmallPit		0x04
#define enemyBigPit			0x08

#define spawnRocks			0x03
#define spawnPits			0x0C

#define enemyUFO			0x10				//A small moving UFO
#define enemyBalls			0x20				//A 16x16 ball monster that drops pit-making bombs

#define skyMask				0x70				//Are enemies in the sky?

#define enemyWaveShot		0x80				//Sky bullets? (top MSB set)
#define enemyBombShot		0x90

#define phaseObstacles		0x10
#define phaseUFOs			0x01
#define phaseBalls			0x02
#define phaseUFOBalls		0x03
#define phaseSkyMask		0x0F
#define phaseGroundMask		0xF0

#define phaseNone			0xFF				//Nothing going on

#define spriteOff			-128

#endif /* GAME_H_ */