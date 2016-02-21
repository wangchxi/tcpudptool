#ifndef _TYPES_H_
#define _TYPES_H_

#include <linux/types.h>
#include <sys/types.h>

//define my favor usage of  unint8...
#ifndef _TYPES_INTNUM_
#define _TYPES_INTNUM_
typedef signed char			INT8,	*PINT8;
typedef signed short		INT16,	*PINT16;
typedef signed int			INT32,	*PINT32;
typedef unsigned char		UINT8,	*PUINT8;
typedef unsigned short		UINT16,	*PUINT16;
typedef unsigned int		UINT32,	*PUINT32;
#endif

//for port on codeblocks
#define int8_t   INT8
#define int16_t  INT16
#define int32_t  INT32
#define uint8_t  UINT8
#define uint16_t UINT16
#define uint32_t UINT32


#ifndef _TYPES_BOOL_
#define _TYPES_BOOL_
typedef int32_t BOOL;
#endif

#ifndef TRUE
#define	TRUE				(1)
#endif

#ifndef FALSE
#define	FALSE				(1)
#endif

//function handle types
#ifndef _TYPES_FUNC_HANDLE_
#define _TYPES_FUNC_HANDLE_
typedef void (*PFUN)(void);
#define funhdl_t PFUN
#endif

#ifndef _TYPES_SOCKET_
#define _TYPES_SOCKET_
typedef int SOCKET;
#endif


#endif
