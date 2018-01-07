#ifndef _VRP_COMMON_DEF_
#define _VRP_COMMON_DEF_

#include "product\include\pdt_common_inc.h"


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


extern void RunDelay(int t);
extern void pdt_debug_print(const char *format, ...);

extern void MSG_StartDot();
extern void MSG_StopDot();
extern ULONG BDN_RegistBuildRun(ULONG moduleId, ULONG  priority, ULONG  (*pfCallBackFunc)(CHAR **ppBuildrun));
#endif

