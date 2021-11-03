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
void spawnExplosion(int16_t x, int8_t y, uint8_t speed);
void spawnBoss();
uint8_t findNextSlot();
void shotLogic();
void gotoSleep();
void enemies();
void checkBlasted(uint8_t which);
void checkHitPlayer(uint8_t which);
void checkObstacle(uint8_t which, uint8_t yHit, uint16_t value);
void damageBoss(uint8_t g);
uint8_t getGround();
void drawHighScores();
void drawOptions();
void drawTime();
void drawMountains2(uint8_t vertRow);
void drawTitle();
void drawNameEntryScreen();
void findScorePosition();
void startNewGame();
void startNewStage();
void startNewLife();
void killPlayer();
void clearShots();
void clearEnemies();
void drawMoonscape();
void gameFrame();
void drawCharacterRow();
void updateHighScores();
void sendTiles();
void gameStatus(uint8_t type);
void gameAction();
void drawDecimal(int32_t theValue, uint8_t *rowRAM);
void drawText(const char *text, uint8_t *rowRAM);
void drawTimeDigit(int xPos, int yPos, int theDigit);
void nextRow();

void cls();

uint8_t eepromReadByte(uint8_t addr);
uint32_t eepromReadLong(uint8_t addr);
void eepromWrite(uint8_t addr, char *initials, uint32_t theScore);
//void eepromWrite(uint8_t addr, uint32_t name, uint32_t theScore);
void writeDefaultHighScores();

void fillTiles(uint8_t location, uint8_t startingTile, uint8_t howMany);


#define stateTitle			0x08			//MSN of low nibble = can move screens left and right
#define stateShowTime		0x09
#define stateHighScores		0x0A
#define stateOptions		0x0B

#define stateEntry			0x05			//Enter your name!
#define stateGetReady		0x07			//Between stages/lives
#define stateGameOver		0x06			//You have died
#define stateRunning		0x80			//MSB means in these states MENU toggles paused and running
#define statePaused			0x90

#define canPauseMask		0x80			//Can pause?
#define canSwitchScreens	0x08			//Can switch screens? (title/time/high scores)

#define playerShot			1
#define waveShot			10
#define bombShot			20

#define playerDriving		0x01
#define playerJumping		0x02
#define playerWinner		0x03				//State when a stage has been beaten (timer leads to score countdown/next stage)
#define playerAlive			0x07				//Mask for driving/jumping/winning/tiger blood
#define playerExploding		0x10				//Self-explanatory
#define playerDead			0x20				//Delay before going back to the splash screen

#define enemySmallRock		0x01				//Static obstacles
#define enemyBigRock		0x02
#define enemySmallPit		0x04
#define enemyBigPit			0x08

#define enemyUFO			0x10				//A small moving UFO
#define enemyBalls			0x20				//A 16x16 ball monster that drops pit-making bombs
#define enemyExplosion		0x30				//An explosion, which may be able to kill the player

#define enemyBossUFO		0xA0				//32x16 boss UFO

#define skyMask				0x70				//Are enemies in the sky?

#define enemyWaveShot		0x80				//Sky bullets? (top MSB set)
#define enemyBombShot		0x90

#define phaseUFOs			0x01
#define phaseBalls			0x02
#define phaseUFOBalls		0x03
#define phaseSkyMask		0x0F

#define phaseRocks			0x10
#define phasePits			0x20
#define phaseGroundMask		0xF0

#define phaseNone			0xFF				//Nothing going on

#define spriteOff			-128

#endif /* GAME_H_ */