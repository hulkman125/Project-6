#pragma once
#include "./types.h"

// Define our function prototypes
void outb(uint16 port, uint8 value);
void outw(uint16 port, uint16 value);
uint8  inb(uint16 port);
uint16 inw(uint16 port);

int printint(uint32 n);
void printhex(uint32 num);
char putchar(char character);
int print(char string[]);
char getchar();
void scan(char address[]);
void clearscreen();