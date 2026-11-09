#pragma once
#include "types.h"
#include "io.h"

#if PROJECT == 1
int kernel();
#endif

//int setcursor(int x, int y);
void setcursor(int x, int y);
char putchar(char character);