#include "project1.h"
#include "project2.h"

// Define our constants that will be widely used
#define TEXT_COLOR 0x07
#define VIDEO_MEM 0xB8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

// Track the current cursor's row and column
static int cursorCol = 0;
static int cursorRow = 0;

#if PROJECT == 1
// This function runs when compiled with the #define PROJECT 1 flag
int kernel()
{
	print("Hello World!\n");
	return 0;
}
#endif

// Setting the cursor does not display anything visually
// Setting the cursor is simply used by putchar() to find where to print next
// This can also be set independently of putchar() to print at any x, y coordinate on the screen
/*int setcursor(int x, int y)
{
	(void)x; // Remove this when implementing your function
	(void)y; // Remove this when implementing your function
	(void)cursorCol; // Remove this when implementing your function
	(void)cursorRow; // Remove this when implementing your function
	return 0;
}*/

// Using a pointer to video memory we can put characters to the display
// Every two addresses contain a character and a color
char putchar(char character)
{
	char* videoMemoryPointer = (char*)VIDEO_MEM; // step 1:creating a pointer to video memory

	if(character == '\n') { // step 2: check whether the character is '\n'
		setcursor(0, cursorRow + 1);
	}
	else {
		//Step 3: if not a newline character, we will print the character to the screen
	int position = (cursorRow * SCREEN_WIDTH + cursorCol) * 2; // step 3a: calculate the position in video memory
	videoMemoryPointer[position] = character; // step 3b: write the character to video memory
	videoMemoryPointer[position + 1] = TEXT_COLOR; // step 3c: write

	//Step 4: Move the cursor to the next position
	setcursor(cursorCol + 1, cursorRow); 
	}

	return character;
}

// Print the character array (string) using putchar()
// Print until we find a NULL terminator (0)
int print(char string[]) 
{
	// Use character count to track how many characters we print
	int characterCount = 0;
	while (*string != 0) {
		putchar(*string);
		string++;
		characterCount++;
	}
	
	return characterCount;
}

// Clear the screen by placing a ' ' character in every character location
void clearscreen()
{
	char* videoMemory =(char*)VIDEO_MEM;
	for(int row = 0; row < SCREEN_HEIGHT; row++) {
		for(int col = 0; col < SCREEN_WIDTH; col++) {
			int position = (row * SCREEN_WIDTH + col) * 2;
			videoMemory[position] = ' ';
			videoMemory[position + 1] = TEXT_COLOR;
		}
	}
	setcursor(0, 0);
}

void setcursor(int x, int y)
{
	
	cursorCol = x;
	cursorRow = y;

	if(cursorCol < SCREEN_WIDTH){
		cursorCol = x % SCREEN_WIDTH;
	}
	
	if(cursorRow > SCREEN_HEIGHT){
		scroll(cursorRow);
		cursorRow = SCREEN_HEIGHT - 1;
	}	
	return;
}