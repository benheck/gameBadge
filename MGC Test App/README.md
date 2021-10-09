This is the test application for the "gameBadge" mini console.

Graphics were converted using an online Arduboy tool: http://www.crait.net/tochars/

FILE STRUCTURE:

main.c - Loads drivers and initiates game
game.c/h - Your custom game code goes here
drivers/gameBadge.c/h - Drivers for the audio/video system.

This app demonstrates the following:

1) Multi-scrolling background
2) Simple tone generation
3) Sprite generation
4) Sleep mode
5) Real-time counter (RTC) module used for time-keeping

THEORY OF OPERATION:

1) main.c sets up system drivers. Default speed is 20Mhz, kind of pushing things at 3.3v but it seems to work :)
2) main.c calls function gameSetup() (in game.c) once. This contains the initial setup routines needed for your game. You own code can call this of course to "reset" a game later on.
3) main.c calls to systemLoop() (in game.c) where the code enters an infinite while() loop.
4) This while() loop checks the value of the variable frameFlag. frameFlag is automatically incremented once per millisecond by the TCB0 timer/interrupt. When frameFlag == 20 (20ms frame time = 50Hz, can be customized) gameFrame() is called.
5) gameFrame() is where your main game logic/state machines should reside (or branch from). In the example app the following happens:
	a) screenLoad() is called. This function also returns the state of the buttons (bitwise, pressed = 1, not pressed = 0)
	b) Background tiles are drawn with drawTiles(*,*) The 2 pointers passed are tileData (the PROGMEM that defines the glyph patterns) and tileMap (the 256 byte RAM array that specifies which gylph pattern appears on screen)
	c) Several sprites are drawn: drawSprite(* PROGMEM data pointer, x position, y position, frame #, mirroring (1 normal, -1 horizontal flip)
	d) x/y positions 0,0 start at the upper-left corner of the screen and go to the right and down. To scroll a sprite past the top or left of the display use a negative position value
	e) The gamePad variable returned from screenLoad() is AND compared to defines for each of the control buttons (dUp, dDown, dLeft, dRight, dMenu, dB and dA)
	f) The d-pad moves the ship up and right, and also scrolls the scene left and right. The layer can scroll at different speeds for each of the 8 rows. In the example this is acheived by taking a base movement and either multipling or dividing it with bitshift operations
	g) The tree in the background and debris in the foreground are both sprites, scrolled at different speeds to simulate parallax scrolling (a trick very common the PC Engine)
	h) clouds-- is an "autoscrolling" variable. This variable is also changed when the player moves, causing the rolling clouds to behave correctly when the screen scrolls
	i) setRowScroll(x, whichRow) sets the fine scroll position for each row. To scroll the entire map use setWindow(x, y)
	j) In this example button "B" puts the gameBadge to sleep. This is a low-power mode that still retains data (such as game variables and the RTC clock timer) To wake, press MENU
	

