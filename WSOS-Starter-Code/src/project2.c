#include "project2.h"

#define TEXT_COLOR 0x07
#define VIDEO_MEM 0xB8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

// Define a keymap to convert keyboard scancodes to ASCII
volatile static char keymap[128] = {

	
	[0x02] = '1',
	[0x03] = '2',
	[0x04] = '3',
	[0x05] = '4',
	[0x06] = '5',
	[0x07] = '6',
	[0x08] = '7',
	[0x09] = '8',
	[0x0A] = '9',
	[0x0B] = '0',
	

	[0x1E] = 'a',
    [0x30] = 'b',
    [0x2E] = 'c',
	[0x20] = 'd',
	[0x12] = 'e',
	[0x21] = 'f',
	[0x22] = 'g',
	[0x23] = 'h',
	[0x17] = 'i',
	[0x24] = 'j',
	[0x25] = 'k',
	[0x26] = 'l',
	[0x32] = 'm',
	[0x31] = 'n',
	[0x18] = 'o',
	[0x19] = 'p',
	[0x10] = 'q',
	[0x13] = 'r',
	[0x1F] = 's',
	[0x14] = 't',
	[0x16] = 'u',
	[0x2F] = 'v',
	[0x11] = 'w',
	[0x2D] = 'x',
	[0x15] = 'y',
	[0x2C] = 'z',
    // Add remaining ascii characters for each scan code

	[0x39] = ' ',
	[0x34] = '.',
	[0x35] = '/',
	[0x1C] = '\n',
};

#if PROJECT == 2
// This function runs when compiled with the #define PROJECT 2 flag for make
int kernel()
{
	char string[100]; // Create a character array to hold the input string
	while(1) {
		print("Please type something: "); // Prompt the user for input
		scan(string); // Read the input string from the keyboard
		print("\nYou typed: "); // Print a message indicating what the user typed
		print(string); // Print the input string back to the screen
		print("\n"); // Print a newline for formatting
	}
	
	return 0;
}
	
#endif

// Gets the character from the keyboard
// This is a blocking function that does not use interrupts
// Only I/O ports and polling are used
char getchar()
{
	unsigned char scancode = 0;
    
	while(1)
	{
		while(!(inb(0x64) & 0x01)) { // Wait for the keyboard to be ready
			//do absolutely nothing!!!!!!!
		}

		scancode = inb(0x60); // Read the scancode from the keyboard
		
		if(scancode & 0x80) { // Check if the key is released
			//return 0; // Return 0 for key release
			continue;
		}

		break;
	}
	return keymap[scancode]; // Return the corresponding ASCII character
}

// Read characters from the keyboard until the user hits the enter key
// Accepts a character array to fill with characters
// Terminates string with NULL terminator when done
void scan(char string[])
{
	int i = 0;
	char c;
	while (1) {
		c = getchar(); // Get a character from the keyboard
		if(c == 0) { // Check if the character is a key release
			continue; // Ignore key releases and continue to the next iteration
		}		
		if (c == '\n') {  // Check if the character is the enter key
			string[i] = '\0'; // Add NULL terminator to the string
			break; // Exit the loop
		}	
		string[i] = c; // Add the character to the string
		putchar(c); // Echo the character to the screen
		i++; // Move to the next position in the string
	}
}

void scroll(int rows)
{
	
	char *videoMemoryPointer = (char*)VIDEO_MEM; // Get a pointer to the video memory
	
	//move the remaining rows upward
	for(int row = 0; row < SCREEN_HEIGHT - rows; row++) {
		for(int col = 0; col < SCREEN_WIDTH; col++) {
			int destiation = (row * SCREEN_WIDTH + col) * 2;
			int source = ((row + rows) * SCREEN_WIDTH + col) * 2;
			videoMemoryPointer[destiation] = videoMemoryPointer[source];
			videoMemoryPointer[destiation + 1] = videoMemoryPointer[source + 1];
		}
	}

	//clear the last rows
	for(int row = SCREEN_HEIGHT - rows; row < SCREEN_HEIGHT; row++) {
		for(int col = 0; col < SCREEN_WIDTH; col++) {
			int position = (row * SCREEN_WIDTH + col) * 2;
			videoMemoryPointer[position] = ' ';
			videoMemoryPointer[position + 1] = TEXT_COLOR;
		}
	} 
}
