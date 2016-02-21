#include "include/common.h"
#include "tthread/tthread.h"
#include "rxtx/rxtx.h"

#include <netdb.h>
#include <net/if.h>

#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include <curses.h>

typedef struct _tut_tcp_server_info
{
    //server bind at which interface
    char ethname[IFNAMSIZ];

    //net byte seq
    uint16_t port;

    SOCKET listen_socket;

    //net byte seq
    char numdot_IP[32];
    struct in_addr sin_addr;

    WINDOW *RxWindow;

    WINDOW *TxWindow;

    int reserved;

    fd_set server_set;

    //TODO:
    //After the single client test is ok, 
    //I should implement a linked list of client here.
    SOCKET client_acksocket;
    fd_set client_set;
    struct sockaddr* client_sa;

    void *listen_thread;
    void *recv_thread;
    void *send_thread;

    int flag1;
    int flag2;

}tut_ts_info_t;

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

static int init(void *ptr)
{
    tut_ts_info_t *info;
    info = (tut_ts_info_t *)ptr;

    //it seems that nothing to do now
    //sinfo->RxWindow = (WINDOW *)malloc(sizeof(WINDOW));

    info->client_sa = NULL;
    FD_ZERO(&info->client_set);

    return 0;
}
static int detect_nics(void *ptr)
{
    tut_ts_info_t *info;
    info = (tut_ts_info_t *)ptr;

    //int ret;

    SOCKET s;

    struct ifconf conf;
    struct ifreq *ifr;

    char buff[IFCONF_SIZE];

    //total num of nic, including loopback and inactive
    uint32_t total;
    //active non-loop nic count
    uint32_t count = 0;
    //small table of total<--->count
    int dif[SUPPORT_NIC_NUM + 1][2] = {{0, 0}};
    //the dif table fisrt param id
    uint32_t table_rid = 0;

    int i;

    conf.ifc_len = IFCONF_SIZE;
    conf.ifc_buf = buff;

    s = socket(PF_INET, SOCK_STREAM, 0);
    if(s < 0)
        return ERR_SKTCRT;

    if(ioctl(s, SIOCGIFCONF, &conf) < 0)
        return ERR_IOCTRL;

    total = conf.ifc_len / sizeof(struct ifreq);
    ifr   = conf.ifc_req;

    if(1 >= total)
    {
        printf("The tcpudptool doesn't detect any active network interface!!!\r\n");
        printf("Please check your Net Card!!!\r\n");
        refresh();
        sleep(1);
        return ERR_NONETCARD;
    }

    if(2 == total)
    {
        
        for(i = 0; i < total; i++)
        {
            struct sockaddr_in *sin = (struct sockaddr_in *)(&ifr->ifr_addr);

            ioctl(s, SIOCGIFFLAGS, ifr);
            
            if(((ifr->ifr_flags & IFF_LOOPBACK) == 0) && (ifr->ifr_flags & IFF_UP))
            {
                printf("Detect %s : %s \r\n", ifr->ifr_name, inet_ntoa(sin->sin_addr));
                refresh();
                info->listen_socket = s;
                strncpy(info->ethname, ifr->ifr_name, IFNAMSIZ);
                info->ethname[IFNAMSIZ - 1] = '\0';
                strncpy(info->numdot_IP, inet_ntoa(sin->sin_addr), 30);
                info->numdot_IP[31] = '\0';

                memcpy(&info->sin_addr, &sin->sin_addr, sizeof(struct in_addr));
            }
            ifr++;
        }
    }
    else// more than one nic, we should let the user choose one.
    {
        //the pos in the array of ifr
        uint32_t select_num = 1;

        printf("The tcpudptool detected %d net interface.\r\n", total - 1);
        for(i = 0; i < total; i++)
        {
            struct sockaddr_in *sin = (struct sockaddr_in *)(&ifr->ifr_addr);

            ioctl(s, SIOCGIFFLAGS, ifr);
            if(((ifr->ifr_flags & IFF_LOOPBACK) == 0) && (ifr->ifr_flags & IFF_UP))
            {
                count++;
                printf("%x. %s \t: %s\r\n", count, ifr->ifr_name, inet_ntoa(sin->sin_addr));
                
                dif[table_rid][0] = i;
                dif[table_rid][1] = count;
                table_rid++;
            }
            ifr++;
        }



        while(1)
        {
            printf("\rWhich one will the tcp server work on ?\r\n");
            printf("Enter the number of the nic inner 5 sec.[Default is the first one] : ");
            refresh();

            //user entered num
            int32_t input_num;

            struct timeval tv;

            tv.tv_sec  = 5;
            tv.tv_usec = 0;

            fd_set set;
            FD_ZERO(&set);

            FD_SET(STDIN_FILENO, &set);

            if(select(65535, &set, NULL, NULL, &tv) > 0)
            {
                input_num = getch();
                //printf("The input num is %d\n", input_num);
                

                input_num -= '0';
                //printf("after minus The input num is %d\n", input_num);
                
                if(input_num <= count && input_num > 0)
                {
                    select_num = input_num;
                    break;//use the select_num
                }
                
                if(input_num == '\n' - '0' || 
                    input_num == '\r' - '0' ||
                     input_num == '\0' - '0')
                {
                    select_num = 1;
                    break;
                }
                else
                {
                    printf("\n\r!!!Invaild Number!!!\n");
                    refresh();
                    continue;
                }
            }

            //using the first dispalyed net interface
            select_num = 1;
            break;
        }

        //find the corresponding ifr id of the input_num;
        for(i = 0;i < total;i++)
        {
            if(dif[i][1] == select_num)
            {
                select_num = dif[i][0];
                //printf("the select num is %d\n", select_num);
                break;
            }
        }

        ifr = conf.ifc_req;
        
        while(select_num > 0)
        {
            ifr++;
            select_num--;
        }

        struct sockaddr_in *sinx = (struct sockaddr_in *)(&ifr->ifr_addr);

        ioctl(s, SIOCGIFFLAGS, ifr);

        info->listen_socket = s;
        
        strncpy(info->ethname, ifr->ifr_name, IFNAMSIZ);
        //printf("\r\nThe ethname is %s\r\n", info->ethname);
        strncpy(info->numdot_IP, inet_ntoa(sinx->sin_addr), 30);
        info->numdot_IP[31] = '\0';
        //printf("The numdot_IP is %s\r\n", info->numdot_IP);
        memcpy(&info->sin_addr, &sinx->sin_addr, sizeof(struct in_addr));
        printf("\r\n");

    }

    return 0;
}

static int get_listenport(void* ptr)
{
    tut_ts_info_t *info;
    info = (tut_ts_info_t *)ptr;

    uint16_t port = DEFAULT_TS_PORT;

    //printf("NOTE : for a better displaying, please MAXMIZE the term window\r\n");
    //printf("You have chosen to create a TCP Server!\r\n");
    printf("Which Port the Server listening at? [scale : 1025 --- 65535]\r\n");
    refresh();

    while(1)
    {
        printf("Enter a valid number inner 5 sec.[Default is 9998] : ");
        //fflush(stdout);
        refresh();

        struct timeval tv;

        tv.tv_sec  = 5;
        tv.tv_usec = 0;

        fd_set set;
        FD_ZERO(&set);

        FD_SET(STDIN_FILENO, &set);

        if(select(65535, &set, NULL, NULL, &tv) > 0)
        {
            char buff[32];
            int  num;

            //fgets(buff, 8, stdin);
            //fflush(stdout);
            //echo();
            getstr(buff);
            if(buff[31] != '\0')
                buff[31] = '\0';

            if(buff[0] == '\n' || buff[0] == '\r' || buff[0] == '\0')
            {
                break;
            }

            sscanf(buff, "%d", &num);

            if(num >= 1025 && num < 65535)
            {
                port = (uint16_t)num;
                break;
            }
            else
            {
                printf("\r!!!Invaild Number!!! \r\n");
                refresh();
                continue;
            }
        }

        break;//using the default port
    }

    info->port = port;

    return 0;
}

static int setup(void* ptr)
{
    tut_ts_info_t *info;
    info = (tut_ts_info_t *)ptr;

    int reuseflag = 1;

    struct sockaddr_in tcpsvr_sa;

    tcpsvr_sa.sin_family = PF_INET;
    tcpsvr_sa.sin_addr.s_addr = info->sin_addr.s_addr;
    tcpsvr_sa.sin_port = htons(info->port);

    if(setsockopt(info->listen_socket, SOL_SOCKET, SO_REUSEADDR,
                         (const char*)&reuseflag, sizeof(reuseflag)) < 0)
        return -1;

    //Todo : if the socket is binded by another app, it should alarm the user

    if(bind(info->listen_socket, (struct sockaddr*)&tcpsvr_sa, 
                         sizeof(struct sockaddr)) < 0)
        return -2;

    //At current, the tut TCP Server can and only can handle one client
    if(listen(info->listen_socket, 1) < 0)
        return -3;

    printf("The TCP Server is set up!\r\n");
    refresh();
    return 0;
}

static int addto_client_list(void *ptr, SOCKET skt, struct sockaddr *clt_ska)
{
    tut_ts_info_t *info;
    info = (tut_ts_info_t*)ptr;

    SOCKET ack_socket = skt;

    struct sockaddr *client;

    client = (struct sockaddr *)malloc(sizeof(struct sockaddr));

    //I only malloc a tiny winy mem in heap, however, 
    //the school teacher always give some questions about "malloc" in exam,
    //so, I'd better check it;
    if(NULL == client)
        return -1;

    memcpy(client, clt_ska, sizeof(struct sockaddr));

    TODO:
    //at current , I only implement one client,
    //it should implement a linked-list here
    info->client_acksocket = ack_socket;
    //FD_SET(ack_socket, &info->client_set);
    FD_SET(info->client_acksocket, &info->client_set);
    info->client_sa = client;

    wprintw(info->RxWindow, "Shake hands with a client from %s : %d\n", 
                    inet_ntoa(((struct sockaddr_in*)info->client_sa)->sin_addr),
                    ntohs(((struct sockaddr_in*)client)->sin_port));
    
    //printf("\n");
    wrefresh(info->RxWindow);

    touchwin(info->TxWindow);
    
    return 0;
}

static void del_client_list(void* ptr, SOCKET skt)
{
    tut_ts_info_t *info;
    info = (tut_ts_info_t*)ptr;

    if(FD_ISSET(skt, &info->client_set))
    {
        FD_ZERO(&info->client_set);
    }
    
    free(info->client_sa);
    
    info->client_acksocket = 0;
    info->client_sa = NULL;
}

void listen_thread(void *ptr)
{
    tut_ts_info_t *info;
    info = (tut_ts_info_t *)ptr;

    struct timeval tv;
    fd_set set;

    tv.tv_sec  = 1;
    tv.tv_usec = 0;

    FD_ZERO(&set);
    FD_SET(info->listen_socket, &set);

    while(1)
    {
        uint32_t len = sizeof(struct sockaddr_in);

        fd_set setx = set;

        if(select(65535, &setx, NULL, NULL, &tv) > 0)
        {
            struct sockaddr client_sa;

            SOCKET socket_ack;

            socket_ack = accept(info->listen_socket, &client_sa, &len);

            if(-1 == socket_ack)
            {
                usleep(10 * 1000);
                continue;
            }

            //wprintw(info->RxWindow, "cao!!!\r\n");
            //wrefresh(info->RxWindow);

            if(addto_client_list((void*)info, socket_ack, &client_sa) < 0)
                break;
        }
        else
            sleep(2);
    }

    //I should add some flag here
    //TODO:
}

void recv_thread(void *ptr)
{
    tut_ts_info_t *info;
    info = (tut_ts_info_t *)ptr;


    //int flag = 0;

    struct timeval tv;
    tv.tv_sec  = 1;
    tv.tv_usec = 0;

    fd_set clt_set;

    while(1)
    {   

        if(info->client_sa == NULL)
        {
            //wprintw(info->RxWindow, "no client\n");
            //wrefresh(info->RxWindow);
            sleep(1);
            continue;
        }
        
        #if 0
        wprintw(info->RxWindow, "The client socket is %d\n", info->client_acksocket);
        wrefresh(info->RxWindow);
        if(flag == 0)
        {
            char test_buff[16] = "1234550000";
            DumpData((void *)info->RxWindow, (const void *)test_buff, 16, 0);
            flag++;
        }
        
        if(FD_ISSET(info->client_acksocket, &clt_set));
        {
            wprintw(info->RxWindow, "is in fd_set\n");
            wrefresh(info->RxWindow);            
        }
        #endif

        
        int ret = 0;
        clt_set = info->client_set;
        if((ret = select(65536, &clt_set, NULL, NULL, &tv)) > 0)
        {
            unsigned char buff[1024];
            int  ret = -1;
            if((ret = socket_rx(info->client_acksocket, (void*)buff, 1024)) < 0)
            {
                wprintw(info->RxWindow, "Recceive data error!!\n");
                wrefresh(info->RxWindow);                
            }
            if(0 == ret)
            {
                wprintw(info->RxWindow, "Client cut off the connnection\n");
                wrefresh(info->RxWindow);
                del_client_list((void *)info, info->client_acksocket);
                continue;
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
}

void send_thread(void *ptr)
{
    tut_ts_info_t *info;
    info = (tut_ts_info_t*)ptr;

    int hex_malloc_flag = 0;

    while(1)
    {
        if(NULL == info->client_sa)
        {
            sleep(1);
            continue;
        }
        
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

        if(socket_tx(info->client_acksocket,(void *)TxBuff, Sendlen) < 0)
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

void CrtTCPSvr(void)
{    
    tut_ts_info_t ts_info;

    int ret;

    printf("\r\nYou have chosen [1] Create a TCP Server.\r\n");

    //todo : initial ts_info
    init((void*)&ts_info);
    
    //it should examinr the ret of the following two func
    ret = detect_nics((void *)&ts_info);
    if(ret < 0)
    {
        printw("detect nic error\n");
        refresh();
        sleep(3);
        return;
    }
    
    ret = get_listenport((void *)&ts_info);
    if(ret < 0)
    {
        printw("get listenport error\n");
        refresh();
        sleep(3);
        return;
    }
    

    printf("\rThe tcpudp_tool is creating TCP server at \r\n");
    printf("[%s] %s : %hd\r\n", ts_info.ethname, ts_info.numdot_IP, 
                                            ts_info.port);
    refresh(); 

    ret = setup((void *)&ts_info);
    if(ret < 0)
    {
        printw("TCP Server Setup error");
        refresh();
        sleep(3);
        return;
    }
    
    //printw("The ret is %d\n", ret);
    //refresh();
    //while(1);
    sleep(2);
    clear();
    refresh();

    WINDOW  *SplitWindow;
#ifdef ARCH_IS_ARM
    if((ts_info.RxWindow = newwin(11, 0, 0, 0)) == NULL)
#else
    if((ts_info.RxWindow = newwin(21, 0, 0, 0)) == NULL)
#endif
    {
        printf("Create RxWindows is wrong\r\n");
        refresh();
        sleep(3);
        goto  err_TCPSvrWin;
    }
    
#ifdef ARCH_IS_ARM
    if((ts_info.TxWindow = newwin(10, 0, 14, 0)) == NULL)
#else
    if((ts_info.TxWindow = newwin(15, 0, 24, 0)) == NULL)
#endif
    {
        delwin(ts_info.RxWindow);
        printf("Create TxWindows is wrong\r\n");
        refresh();
        sleep(3);
        goto  err_TCPSvrWin;
    }
    
#ifdef ARCH_IS_ARM
    if((SplitWindow = newwin(2, 0, 11, 0)) == NULL)
#else
    if((SplitWindow = newwin(2, 0, 21, 0)) == NULL)
#endif
    {
        delwin(ts_info.RxWindow);
        delwin(ts_info.TxWindow);
        printf("Create SplitWindow is wrong\r\n");
        refresh();
        sleep(3);
        goto  err_TCPSvrWin;
    }

    WINDOW  *RxWindow = ts_info.RxWindow;
    WINDOW  *TxWindow = ts_info.TxWindow;
    
    //use diamond to split Rx and Tx windows
    TODO:
    //I should put some notes in the SplitWindow
    box(SplitWindow, ACS_DIAMOND, ACS_DIAMOND);
    wrefresh(SplitWindow);

    scrollok(RxWindow, TRUE);
    scrollok(TxWindow, TRUE);

    wprintw(RxWindow, "This the Rx Window displaying ingress data :\n");
    wprintw(TxWindow, "This the Tx Window. Please Enter egress data :\n");
    wrefresh(RxWindow);
    wrefresh(TxWindow);
        
    ts_info.listen_thread = tut_create_thread((void*)listen_thread,
                                                (void*)&ts_info);
    
    if(NULL == ts_info.listen_thread)
    {
        wprintw(RxWindow, "Create listen_thread fail %d\n", __LINE__);
        wrefresh(RxWindow);
        goto err_TCPSvrThread;
    }

    ts_info.recv_thread = tut_create_thread((void*)recv_thread,
                                                (void*)&ts_info);

    if(NULL == ts_info.recv_thread)
    {
        wprintw(RxWindow, "Create recv_thread fail %d\n", __LINE__);
        wrefresh(RxWindow);
        goto err_TCPSvrThread;
    }

    ts_info.send_thread = tut_create_thread((void*)send_thread,
                                                (void*)&ts_info);

    if(NULL == ts_info.send_thread)
    {
        wprintw(RxWindow, "Create send_thread fail %d\n", __LINE__);
        wrefresh(RxWindow);
        goto err_TCPSvrThread;
    }

    //I should add some thread flag here to get control of the threads
    //currently , a  endless-loop is now working fine, and Ctrl^C can end it
    
    while(1);

err_TCPSvrThread :

    //I should exam the thread status, if alloced
    if(ts_info.listen_thread)
        tut_release_thread(ts_info.listen_thread);
    if(ts_info.recv_thread)
        tut_release_thread(ts_info.recv_thread);
    if(ts_info.send_thread)
        tut_release_thread(ts_info.send_thread);
    
err_TCPSvrWin:
    

    return;
}


