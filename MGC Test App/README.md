This is the test application for the "gameBadge" mini console.

Graphics were converted using an online Arduboy tool: http://www.crait.net/tochars/

File structure:

main.c - Loads drivers and initiates game
game.c/h - Your custom game code goes here
drivers/gameBadge.c/h - Drivers for the audio/video system.

This app demonstrates the following:

1) Multi-scrolling background
2) Simple tone generation
3) Sprite generation
4) Sleep mode
5) Real-time counter (RTC) module used for time-keeping

THEORY OF OPERATION

The TCB0 timer is setup to trigger an interrupt every 1 millisecond. This interrupt increments a variable called frameFlag.
