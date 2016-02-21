#ifndef _RXTX_H_
#define _RXTX_H_

#include <curses.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "../include/types.h"

void DumpData(void* win, const void* Buffer, int Length, int StartLabel);

int HextoASCII(const void *Buffer, int Length, void* const retBuffer, int *retLen);
#endif
