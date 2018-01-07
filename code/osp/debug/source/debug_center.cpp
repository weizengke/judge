#include <iostream>
#include <windows.h>
#include <stdlib.h>
#include <process.h>
#include <string>  
#include <sstream>  
#include <algorithm> 

//#include <mutex>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <queue>


#include "osp\debug\include\debug_center_inc.h"

using namespace std;

char *szModuleName[32] = {
	"none",
	"common",
	"judge",
	"mysql",
	"debug-center",
	"command",
	"event",
	"ndp",
	"telnet",

	"end",
};

char *szDebugName[DEBUG_TYPE_MAX] = {
	"none",
	"error",
	"function",
	"info",
	"message",
};
unsigned long g_aulDebugMask[MID_ID_END][DEBUG_TYPE_MAX/32 + 1] = {0};
int g_debug_switch = DEBUG_DISABLE;
queue <MSGQUEUE_S> g_stMsgQueue;

void debugcenter_print(MID_ID_EM mid, DEBUG_TYPE_EM type, const char *format, ...)
{
#if 1
	if (g_debug_switch == DEBUG_DISABLE)
	{
		return;
	}

	if (!DEBUG_MID_ISVALID(mid))
	{
		return;
	}

	if (!DEBUG_TYPE_ISVALID(type))
	{
		return;
	}

	if (!DEBUG_MASK_GET(mid, type))
	{
		return;
	}
#endif

	time_t  timep = time(NULL);
	struct tm *p;
    MSGQUEUE_S stMsgQ;
    char buf[BUFSIZE + 1] = {0};
    char buf_t[BUFSIZE + 1] = {0};

    p = localtime(&timep);
    p->tm_year = p->tm_year + 1900;
    p->tm_mon = p->tm_mon + 1;

	va_list args;
	va_start(args, format);
	vsnprintf(buf_t, BUFSIZE, format, args);
	sprintf(buf,"%s%s", buf,buf_t);
	va_end(args);
	sprintf(buf,"%s", buf);

    strcpy(stMsgQ.szMsgBuf, buf);

	stMsgQ.mid = mid;
	stMsgQ.type = type;
    stMsgQ.thread_id = GetCurrentThreadId();
	stMsgQ.stTime = *p;

	g_stMsgQueue.push(stMsgQ);

}


void pdt_debug_print(const char *format, ...)
{
	time_t  timep = time(NULL);
	struct tm *p;
    MSGQUEUE_S stMsgQ;
    char buf[BUFSIZE + 1] = {0};
    char buf_t[BUFSIZE + 1] = {0};

    p = localtime(&timep);
    p->tm_year = p->tm_year + 1900;
    p->tm_mon = p->tm_mon + 1;

	va_list args;
	va_start(args, format);
	vsnprintf(buf_t, BUFSIZE, format, args);
	snprintf(buf, BUFSIZE, "%s%s", buf,buf_t);
	va_end(args);
	snprintf(buf, BUFSIZE, "%s", buf);

    strcpy(stMsgQ.szMsgBuf, buf);

	stMsgQ.mid = MID_NULL;
	stMsgQ.type = DEBUG_TYPE_NONE;
    stMsgQ.thread_id = GetCurrentThreadId();
	stMsgQ.stTime = *p;

	g_stMsgQueue.push(stMsgQ);

}

void RunDelay(int t)
{
    //std::this_thread::sleep_for(std::chrono::milliseconds(t));
    Sleep(t);
}


void MSG_OutStringWait()
{

}

int g_dotprint = 0;

unsigned _stdcall msg_dot_thread(void *pEntry)
{
	int i;

	g_dotprint = 1;

	for (int i=0; g_dotprint!=0; i++)
	{
		printf(".");
		RunDelay(500);
	}

	return 0;
}

void MSG_StartDot()
{
	_beginthreadex(NULL, 0, msg_dot_thread, NULL, NULL, NULL);
	//_beginthread(msg_dot_thread,0,NULL);
}

void MSG_StopDot()
{
	g_dotprint = 0;
}


unsigned _stdcall  MSGQueueMain(void *pEntry)
{
    MSGQUEUE_S stMsgQ;
	char module[32] = {0};
	char type[32] = {0};
	char buff[BUFSIZE] = {0};
    for (;;)
    {
        while (!g_stMsgQueue.empty())
        {
            stMsgQ = g_stMsgQueue.front();

			/* 向所有用户发送 */

			if (stMsgQ.mid >= MID_ID_END
				|| stMsgQ.type >= DEBUG_TYPE_MAX)
			{
				g_stMsgQueue.pop();
				RunDelay(1);
				continue;
			}

			strcpy(module, szModuleName[stMsgQ.mid]);
			strcpy(type, szDebugName[stMsgQ.type]);
			std::transform(module, module + 31,module,::toupper);  
			std::transform(type, type + 31,type,::toupper);  
			
			snprintf(buff, BUFSIZE, "%04d-%02d-%02d %02d:%02d:%02d/DEBUG/%s/%s:%s\r\n",
						stMsgQ.stTime.tm_year,stMsgQ.stTime.tm_mon,stMsgQ.stTime.tm_mday,
						stMsgQ.stTime.tm_hour,stMsgQ.stTime.tm_min,stMsgQ.stTime.tm_sec, 
						module, type,stMsgQ.szMsgBuf);

			extern void vty_print2all(const char *format, ...);
			vty_print2all(buff);
			
			/* BEGIN: Added by weizengke, 2013/11/17 */
			//extern void cmd_outcurrent();
			//cmd_outcurrent();
			/* END:   Added by weizengke, 2013/11/17 */

            g_stMsgQueue.pop();
			
            RunDelay(1);
        }

        RunDelay(1);

    }

	return 0;
}

APP_INFO_S g_DebugAppInfo =
{
	NULL,
	"Debug-Center",
	NULL,
	MSGQueueMain
};

void Debug_RegAppInfo()
{
	RegistAppInfo(&g_DebugAppInfo);
}

