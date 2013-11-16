#ifndef _PDT_COMMON_INC_
#define _PDT_COMMON_INC_


#include <thread>

extern void RunDelay(int t);
extern void MSGQueueMain();
extern void pdt_debug_print(const char *format, ...);
extern void pdt_debug_print_ex(int level, const char *format, ...);

extern void MSG_StartDot();
extern void MSG_StopDot();

#endif _PDT_COMMON_INC_

