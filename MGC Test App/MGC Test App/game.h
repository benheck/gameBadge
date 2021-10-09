#ifndef GAME_H_
#define GAME_H_

void gameSetup();
void systemLoop();
void gameFrame();
void gotoSleep();

//YOUR OTHER FUNCTION PROTOTYPES HERE

void fillTiles(uint8_t location, uint8_t startingTile, uint8_t howMany);

#endif /* GAME_H_ */