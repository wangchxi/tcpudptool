#include <curses.h>

#include "TCPSvr.h"
#include "TCPClt.h"

int main(int argc, char* argv[])
{
    initscr();
    refresh();
    cbreak();
    echo();

//    CrtTCPSvr();
    CrtTCPClt();

    endwin();

    return 0;
}