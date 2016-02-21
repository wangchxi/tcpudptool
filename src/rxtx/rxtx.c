#include "rxtx.h"

void DumpData(void* pwin, const void* Buffer, int Length, int StartLabel)
{   
    WINDOW *win = pwin;
    unsigned char *CurrentPointer = (unsigned char *)Buffer, LocalBuffer[17];
    int CurrentIndex;
    TODO:
    //I think I should add a timer here~
    wprintw(win ,"Receive %d byte Data, Dump it in HEX and ASCII", Length);
    for (CurrentIndex = (StartLabel & ~0xF) - StartLabel;
        StartLabel + CurrentIndex < ((StartLabel + Length - 1) & ~0xF) + 16;
        CurrentIndex++)
    {
        int Offset = (StartLabel + CurrentIndex) & 0xF;
        if (Offset == 0)
            wprintw(win, "\n0x%04X: ", StartLabel + CurrentIndex);
        if (CurrentIndex < Length && CurrentIndex >= 0)
        {
            LocalBuffer[Offset] = *CurrentPointer++;
            wprintw(win, "%02X ", LocalBuffer[Offset]);
            if (LocalBuffer[Offset] < 0x20 || LocalBuffer[Offset] >= 0x7f)
                LocalBuffer[Offset] = '.';
        }
        else
        {
            wprintw(win, "   ");
            LocalBuffer[Offset] = ' ';
        }
        if (Offset == 0xF)
        {
            LocalBuffer[16] = 0;
            wprintw(win, "  | %s", LocalBuffer);
        }
    }
    wprintw(win, "\n");
    wrefresh(win);
}

//not be responsiable to check the input data

int HextoASCII(const void * Buffer,int Length, void* const retBuffer,int *retLen)
{
    char* pbuff = (void *)Buffer;

    char  newbuffer[Length];
    char  *pnewbuff = newbuffer;

    int originlen = Length;

    char *prBuff = (void *)retBuffer;

    originlen += 1;
    
    while(originlen > 0)
    {
        while(*pbuff && isspace(*pbuff))pbuff++;
        *pnewbuff++ = *pbuff++;
        originlen--;
    }

    pnewbuff = newbuffer;
    int newlen = strlen(newbuffer);

    //if the Buffer is just "0x" and some blanks ,just send "0x"
    if(2 == newlen)
    {
        *prBuff = '0';
        *(prBuff + 1) = 'x';
        *retLen = newlen;
        return -1;
    }
    
    if((newlen & 1) != 0)
    {
        newlen += 1;
    }
    
    int i;
    char temp[3];
    int  rlen = newlen/2 - 1;

    for(i = 0; i < rlen; i++)
    {
        strncpy(temp, pnewbuff + 2 + i*2, 2);
        temp[2] = 0;
        //printf("temp is %s \n", temp);
        *(prBuff + i) = strtol(temp, NULL, 16);
        //printf("res is %c\n", res);
    }
    *retLen = rlen;

    return 0;
}
