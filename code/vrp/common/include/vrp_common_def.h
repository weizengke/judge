#ifndef _VRP_COMMON_DEF_
#define _VRP_COMMON_DEF_

#include "..\..\..\product\include\pdt_common_inc.h"



extern void RunDelay(int t);
extern void pdt_debug_print(const char *format, ...);
extern void pdt_debug_print_ex(int level, const char *format, ...);

extern void MSG_StartDot();
extern void MSG_StopDot();

#endif

