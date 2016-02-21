#include "include/common.h"
#include "tthread/tthread.h"
#include "rxtx/rxtx.h"

#include <netdb.h>
#include <net/if.h>

#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>

#include <curses.h>

#include <errno.h>

typedef  struct  _tut_tcp_client_info
{
    //dest TCP Server numdot IP, is a string
    char  dest_numdot_IP[32];  

    //in_addr.s_aadr type is a uint32
    struct in_addr  dest_sin_addr;
    
    uint16_t  dest_port;

    //the client 's socket
    SOCKET  con_socket;

    WINDOW *RxWindow;
    WINDOW *TxWindow;

    void* recv_thread;
    void* send_thread;

    int recvthread_quit;

    int flag1;
    int flag2;
}tut_tc_info_t;


static int socket_rx(SOCKET socket, void *pbuffer, int len)
{
    int actual = 0;
    
    fd_set set;

    struct timeval tv;        
    tv.tv_sec = 0;
    tv.tv_usec = 10 * 1000;
    
    while (actual < len)
    {
        int ret;
        FD_ZERO(&set);
        FD_SET((SOCKET)socket, &set);

        if (select(65535, &set, NULL, NULL, &tv)>0)
        {
            ret = recv(socket, (char*)pbuffer+actual, len-actual, 0);
            if (ret > 0)
            {
                actual += ret;
            }
            else if(0 == ret)
            {
                return 0;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            return actual;
        }
    }
    
    return 0;
}

static int socket_tx(SOCKET socket, void *pbuffer, int len)
{
    int actual = 0;

    if(0 == len)
        return -2;
        
    while (actual < len)
    {
        int ret = send(socket, (char *)pbuffer+actual, len-actual, 0);
        if (ret > 0)
        {
            actual += ret;
        }
        else
        {
            return -1;
        }
    }
    
    return 0;
}

static void set_fl(int fd, int flags)
{
    int val;

    val = fcntl(fd, F_GETFL, 0);

    val |= flags;

    fcntl(fd, F_SETFL, val);
}

static void clr_fl(int fd, int flags)
{
    int val;

    val = fcntl(fd, F_GETFL, 0);

    val &= ~flags;

    fcntl(fd, F_SETFL, val);
}

static int connect_nonblk(SOCKET skt, struct sockaddr* sa, socklen_t salen,
                                unsigned int sec)
{
    int error;

    fd_set rset, wset;
    FD_ZERO(&rset);
    FD_SET(skt, &rset);
    wset = rset;

    struct timeval tv;
    tv.tv_sec  = sec;
    tv.tv_usec = 0;

    errno = 0;

    if(connect(skt, sa, salen) < 0)
    {
        if(EINPROGRESS != errno)
            return -1;
    }
    else
        return 0;//connect successfully

    if(0 == select(65536, &rset, &wset, NULL, &tv))
    {
        close(skt);
        return -1;        
    }
    
    if(FD_ISSET(skt, &rset) || FD_ISSET(skt, &wset))
    {
        int len = sizeof error;
        if(getsockopt(skt, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len) < 0)
            return -2;

        // definetly can not connect TCP Server
        if(error)
        {
            close(skt);
            errno = error;
            return -3; 
        }
    }

    return 0;    
}


void  get_DestIP(void *ptr)
{
    tut_tc_info_t *info;
    info = (tut_tc_info_t*)ptr;

    struct in_addr in;

    char     buff[256];
    uint32_t len;

    while(1)
    {
        printw("Please Input the IP of Target TCP Server\n");
        refresh();
        
        

        getstr(buff);

        len = strlen(buff);

        if(len < 7 || len > 15)
        {
            printw("!!!Invalid address!!!\n");
            usleep(100 * 1000);
            continue;
        }
   
        break;
    }

    inet_aton(buff, &in);
    
    memcpy(info->dest_numdot_IP, buff, len);
    info->dest_numdot_IP[len] = 0;
    info->dest_sin_addr.s_addr = in.s_addr; 
}

void get_DestPort(void *ptr)
{
    tut_tc_info_t *info;
    info = (tut_tc_info_t*)ptr;

    char buff[256];
    uint32_t num;
    
    while(1)
    {
        printw("Please Input the Port.\n");
        refresh();
        
        uint32_t len;

        getstr(buff);

        len = strlen(buff);

        if(len < 1 || len > 5)
        {
            printw("!!!Invalid num!!!\n");
            usleep(100 * 1000);
            continue;
        }

        sscanf(buff, "%d", &num);

        if(num > 65535)
        {
            printw("!!!Invalid num!!!\n");
            usleep(100 * 1000);
            continue;
        }
        break;
    }
    
    info->dest_port = htons((uint16_t)num);
}

int connect_DestTS(void *ptr)
{
    tut_tc_info_t *info;
    info = (tut_tc_info_t*)ptr;

    struct sockaddr_in dest_sa;
    dest_sa.sin_family = PF_INET;
    dest_sa.sin_addr.s_addr = info->dest_sin_addr.s_addr;
    dest_sa.sin_port   = info->dest_port;

    int salen = sizeof(struct sockaddr);

    BOOL on = TRUE;

    SOCKET skt;

    uint32_t time  = 2;
    while(1)
    {
        if((skt = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        {
            printw("Create socket error!!!\n");
            refresh();
            return -1;
        }

        setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof on);


        set_fl(skt, O_NONBLOCK);
        if(0 == connect_nonblk(skt, (struct sockaddr*)&dest_sa, 
                                                (socklen_t)salen, 2))
        {
            printw("Successfully connect TCP Server\n");
            refresh();
            sleep(1);
            clr_fl(skt, O_NONBLOCK);
            break;
        }
        else
        {
            sleep(time);
            printw("Fail to connect TCP Server,Try again in %d seconds\n", time);
            refresh();
            sleep(time);
            time  *= 2;
            printw("Reconnectng to the TCP Server!\n");
            refresh();
        }
        
        
        if(time > 8)
        {
            printw("Can not connect TCP Server!!! Please check your network\n", time);
            refresh();
            sleep(2);
            return -2;
        }
    }   

    info->con_socket = skt;
    return 0;
}

static void recv_thread(void *ptr)
{
    tut_tc_info_t *info;
    info = (tut_tc_info_t *)ptr;

    struct timeval tv;
    tv.tv_sec  = 2;
    tv.tv_usec = 0;

    fd_set rx_set;

    while(1)
    {          
        int ret = 0;
        FD_ZERO(&rx_set);
        FD_SET(info->con_socket, &rx_set);
        
        if((ret = select(65536, &rx_set, NULL, NULL, &tv)) > 0)
        {
            unsigned char buff[1024];
            int  ret;
            if((ret = socket_rx(info->con_socket, (void*)buff, 1024)) < 0)
            {
                wprintw(info->RxWindow, "Recceive data error!!\n");
                wrefresh(info->RxWindow);                
            }
            if(0 == ret)
            {
                wprintw(info->RxWindow, "Server cut off connection!!\n");
                wrefresh(info->RxWindow);
                sleep(2);
                break;
            }
            if(ret > 0)
            {   
                DumpData((void*)info->RxWindow, (const void *)buff, ret, 0);
            }
        }
        else
        {
            usleep(100 * 1000);
        }
    }
    info->recvthread_quit = 1;
    
    return;
}

static void send_thread(void *ptr)
{
    tut_tc_info_t *info;
    info = (tut_tc_info_t*)ptr;

    int hex_malloc_flag = 0;

    while(1)
    {
        if(1 == info->recvthread_quit)
            break;
            
        char  *InputBuff = (char *)malloc(1024);
        char  *TxBuff;

        wgetstr(info->TxWindow, InputBuff);

        //char  TxBuff[1024];
        
        int   Inputlen;
        int   Sendlen;

        Inputlen = strlen(InputBuff);
        if(0 == Inputlen)
            continue;

        //if the first two char is "0x"
        if(InputBuff[0] == '0' && InputBuff[1] == 'x' && InputBuff[2] != 0)
        {
            hex_malloc_flag = 1;
        
            char *pSendBuff =(char*)malloc(Inputlen);
            int  ModLen;

            HextoASCII((const void*)InputBuff, Inputlen, 
                                (void* const)pSendBuff, &ModLen);     
            TxBuff  = pSendBuff;
            Sendlen = ModLen;
        
        }
        else//send the ASCII bitstream
        {
            TxBuff  = InputBuff;
            Sendlen = Inputlen;
        }

        if(socket_tx(info->con_socket,(void *)TxBuff, Sendlen) < 0)
        {
            wprintw(info->TxWindow, "Transimit data error!!\n");
            wrefresh(info->TxWindow);

            if(hex_malloc_flag)
                free(TxBuff);
        }
        else
        {
            wprintw(info->TxWindow, "Transimit data success!!\n");
            wrefresh(info->TxWindow);

            if(hex_malloc_flag)
                free(TxBuff);            
        }

        free(InputBuff);
        
        hex_malloc_flag = 0;
    }
}


void CrtTCPClt(void)
{
    tut_tc_info_t tc_info;
    
    printf("\r\nYou have chosen [2] Create a TCP Client.\r\n");

    get_DestIP((void *)&tc_info);

    get_DestPort((void *)&tc_info);

    printw("Try to connect the TCP Server %s : %d\n", tc_info.dest_numdot_IP, 
                                        ntohs(tc_info.dest_port));
    refresh();

    //while(1);
    if(connect_DestTS((void *)&tc_info) < 0)
    {
        return;
    }

    //while(1);
    //sleep(2);
    clear();
    refresh();
    
    WINDOW  *SplitWindow;
#ifdef ARCH_IS_ARM
    if((tc_info.RxWindow = newwin(11, 0, 0, 0)) == NULL)
#else
    if((tc_info.RxWindow = newwin(21, 0, 0, 0)) == NULL)
#endif
    {
        printf("Create RxWindows error\r\n");
        refresh();
        sleep(3);
        return;
    }

#ifdef ARCH_IS_ARM
    if((tc_info.TxWindow = newwin(10, 0, 14, 0)) == NULL)
#else
    if((tc_info.TxWindow = newwin(15, 0, 24, 0)) == NULL)
#endif
    {
        delwin(tc_info.RxWindow);
        printf("Create TxWindows error\r\n");
        refresh();
        sleep(3);
        return;
    }

#ifdef ARCH_IS_ARM
    if((SplitWindow = newwin(2, 0, 11, 0)) == NULL)
#else
    if((SplitWindow = newwin(2, 0, 21, 0)) == NULL)
#endif
    {
        delwin(tc_info.RxWindow);
        delwin(tc_info.TxWindow);
        printf("Create SplitWindow error\r\n");
        refresh();
        sleep(3);
        return;
    }

    WINDOW  *RxWindow = tc_info.RxWindow;
    WINDOW  *TxWindow = tc_info.TxWindow;

    box(SplitWindow, ACS_PLUS, ACS_PLUS);
    wrefresh(SplitWindow);

    scrollok(RxWindow, TRUE);
    scrollok(TxWindow, TRUE);

    wprintw(RxWindow, "This the Rx Window displaying ingress data :\n");
    wprintw(TxWindow, "This the Tx Window. Please Enter egress data :\n");
    wrefresh(RxWindow);
    wrefresh(TxWindow);

    touchwin(TxWindow);

    tc_info.recv_thread = tut_create_thread((void*)recv_thread,(void*)&tc_info);

    if(NULL == tc_info.recv_thread)
    {
        wprintw(RxWindow, "Create recv_thread fail %d\n", __LINE__);
        wrefresh(RxWindow);
        goto err_TCPCltThread;
    }

    tc_info.send_thread = tut_create_thread((void*)send_thread, (void*)&tc_info);
    if(NULL == tc_info.send_thread)
    {
        wprintw(RxWindow, "Create send_thread fail %d\n", __LINE__);
        wrefresh(RxWindow);
        goto err_TCPCltThread;
    }

    while(1)
    {
        if(1 == tc_info.recvthread_quit)
        {
            clear();
            wprintw(tc_info.RxWindow, "Exit proggram!!!\n");
            refresh();
            goto err_TCPCltThread;
        }
        sleep(2);
    }

    
err_TCPCltThread:
    delwin(RxWindow);
    delwin(TxWindow);
    
    if(tc_info.recv_thread)
        tut_release_thread(tc_info.recv_thread);
    if(tc_info.send_thread)
        tut_release_thread(tc_info.send_thread);
    
    return;
}


