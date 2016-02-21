#ifndef _COMMON_H_
#define _COMMON_H_

#include "types.h"

//TCP Server
#define ERR_SKTCRT -1
#define ERR_IOCTRL -2
#define ERR_NONETCARD -3

#define DEFAULT_TS_PORT 9998

//support max num of detected nic
#define SUPPORT_NIC_NUM  8
// so the size of bus is X * 32
#define IFCONF_SIZE    (8 * 32)

void CrtTCPSvr(void);
void CrtTCPClt(void);



#endif
