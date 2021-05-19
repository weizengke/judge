#ifndef _VRP_COMMON_DEF_
#define _VRP_COMMON_DEF_

#include "pdt_common_inc.h"

#ifndef UCHAR
#define UCHAR unsigned char
#endif

#ifndef ULONG
#define ULONG unsigned long
#endif

#ifndef LONG
#define LONG long
#endif

#ifndef USHORT
#define USHORT unsigned short
#endif

#ifndef CHAR
#define CHAR char
#endif

#ifndef VOID
#define VOID void
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

typedef ULONG DWORD;

#ifndef INVALID_SOCKET
#define INVALID_SOCKET	(ULONG)(~0)
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR	(-1)
#endif

#define M_DES(x,y) y

#define BUFSIZE 65535

#define BDN_PRIORITY_NORMAL 128
#define BDN_PRIORITY_HIGH   256
#define BDN_PRIORITY_LOW    64

#define BDN_MAX_BUILDRUN_SIZE 65535

#define BDN_BUILDRUN	"\r\n"
#define BDN_BUILDRUN_INDENT_1	"\r\n "
#define BDN_BUILDRUN_INDENT_2	"\r\n  "
#define BDN_BUILDRUN_INDENT_3	"\r\n   "

#if _MSC_VER
#define snprintf _snprintf
#endif

/* assert(0) */
#define OS_DBGASSERT(x,args,...) \
if (0 == x) {\
	printf("\r\nAssert at %s:%d. ", __FILE__, __LINE__);\
	printf(args);\
}\

extern void RunDelay(int t);

extern void MSG_StartDot();
extern void MSG_StopDot();
extern void write_log(int level, const char *fmt, ...);
extern ULONG cli_bdn_regist(ULONG moduleId, ULONG view_id, ULONG  priority, ULONG  (*pfCallBackFunc)(CHAR **ppBuildrun, ULONG ulIncludeDefault));
#endif

