#include "io.h"
#include "irq.h"
#include "isr.h"
#include "fat.h"
#include "string.h"
#include "project1.h"
#include "project2.h"
#include "project3.h"
#include "project4.h"
#include "project5.h"
#include "project6.h"

void prockernel();
void fileproc();

int main() 
{
	// Initialize our OS systems
	idt_install();
    isrs_install();
    irq_install();

	// clear the screen
	clearscreen();

#if PROJECT == 1
	print("Welcome to WSOS - Project 1\n");
#elif PROJECT == 2
	print("Welcome to WSOS - Project 2\n");
#elif PROJECT == 3
	print("Welcome to WSOS - Project 3\n");
#elif PROJECT == 4
	print("Welcome to WSOS - Project 4\n");
#elif PROJECT == 5
	print("Welcome to WSOS - Project 5\n");
#elif PROJECT == 6
	print("Welcome to WSOS - Project 6\n");
#else
	print("Error: OS was compiled without specifying the project!\n");
	while(1){};
#endif

	kernel();

	while(1){};

	return 0;
}