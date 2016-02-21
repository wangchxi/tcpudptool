#include "include/types.h"
#include "include/common.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/select.h>

#include <fcntl.h>


#include <curses.h>


static  void  CrtUDPClt(void);
static  void  CrtUDPSvr(void);

#define DELAYSEC 5

//displaying caption and corresponding function
typedef struct _CmdTip 
{
    funhdl_t Function;
    char*    Caption;
}CMDTIP;

CMDTIP CmdTip[] =
{
    {CrtTCPSvr, "Create a TCP Server"},
    {CrtTCPClt, "Create a TCP Client"},
    {CrtUDPSvr, "Create a UDP Server"},
    {CrtUDPClt, "Create a UDP Client"},
    {NULL, NULL}
};



void CrtUDPClt(void)
{
    printf("\r\nThis is the %s\r\n", __func__);
    printf("Sorry the func has not been implemented yet");
    refresh();
    sleep(2);
}

void CrtUDPSvr(void)
{
    printf("\r\nThis is the %s\r\n", __func__);
    printf("Sorry the func has not been implemented yet");
    refresh();
    sleep(2);
}

int main(int argc, char* argv[])
{
    BOOL isAutoRun = TRUE;

    //defaulr function hanlde
    funhdl_t DefaultFunc;
    DefaultFunc = CrtTCPSvr;

    //API from libcurses
    initscr();
    refresh();
    //noecho();
    cbreak();

    uint32_t  index, selectId;
    int32_t   secLeft;

    printw("*******************************************\n");
    printw("*                                         *\n");
    printw("*        tcpudp_tool from SEU ASIC        *\n");
    printw("*                                         *\n");
    printw("*                                         *\n");
    printw("*                                         *\n");
    printw("*                                         *\n");
    printw("*   For more information, contact me by   *\n");
    printw("*         august.seu@gmail.com            *\n");
    printw("*                                         *\n");
    printw("*******************************************\n");
    refresh();

    
    printf("Pelease Enter the number of the following function\r\n");
    for(index = 0; NULL != CmdTip[index].Function; index++)
    printf(" %d. %s\r\n", index + 1, CmdTip[index].Caption);

    while(1)
    {
        if(isAutoRun)
        {
            printf("AutoRun will begin inner 0 sec : ");
            for(secLeft = DELAYSEC; secLeft >= 0; secLeft--)
            {

                printf("\rAutoRun will begin inner %d sec : ", secLeft);
                fflush(stdout);

                fd_set  set;
                FD_ZERO(&set);
                FD_SET(STDIN_FILENO, &set);

                struct timeval tv;

                tv.tv_sec  = 1;
                tv.tv_usec = 5000;

                while(1)
                {

                    if(select(65536, &set, NULL, NULL, &tv) > 0)
                    {
                        int nn = getch();
                        //printf("the n is %d\r\n", nn);
                        selectId = nn - '0';
                        //printf("the SelecId is %d\r\n", selectId);
                        isAutoRun = FALSE;
                        break;
                    }
                    else
                    {
                        fflush(stdout);
                        break;
                    }
                }

                if(!isAutoRun)
                    break;
            }//end for(Select...)
        }//end for if(isAutoRun)
        else
        {
        //TODO:
        }

        if(isAutoRun)
        {
            printf("\r\nTime Out! Not any Input!\r\nRun the default Function!\r\n");
            DefaultFunc();
        }
        else
        {
            if(selectId > 0 && selectId < 5)
            {
                selectId -= 1;
                
                CmdTip[selectId].Function();

                break;
                
            }
            else
            {
                printf("\r\n!!!Invalid input!!!\r\n");
                isAutoRun = TRUE;                
            }
        }

        
    }//end while(1)

    endwin();


end_process:

    return 0;
}//end main()

