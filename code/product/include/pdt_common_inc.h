#ifndef _PDT_COMMON_INC_
#define _PDT_COMMON_INC_


#include <thread>


#define UCHAR unsigned char
#define ULONG unsigned long
#define CHAR char

#define OS_OK   0
#define OS_ERR  1

#define OS_TRUE 0
#define OS_FALSE 1

#define OS_YES 1
#define OS_NO  0


extern void RunDelay(int t);
extern void MSGQueueMain();
extern void pdt_debug_print(const char *format, ...);
extern void pdt_debug_print_ex(int level, const char *format, ...);

extern void MSG_StartDot();
extern void MSG_StopDot();

#endif _PDT_COMMON_INC_

