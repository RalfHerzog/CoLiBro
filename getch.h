#ifndef GETCH_INCLUDED
#define GETCH_INCLUDED

#if !(defined _WIN32 || defined _WIN64)

#include <stdio.h>
#include <termios.h>
#include <unistd.h>

char getch();

#else

#include <conio.h>

#endif

#endif // GETCH_INCLUDED
