#include <iostream>
#include <windows.h>
#include <stdlib.h>
//#include <thread>
#include <string>
//#include <mutex>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <queue>

#include "vrp\debug\include\debug_center_inc.h"

using namespace std;

#define MAX_MSGBUF_SIZE 255

typedef struct tag_MSGQueue_S {
    char szMsgBuf[MAX_MSGBUF_SIZE];
	unsigned long thread_id;
	struct tm stTime;

}MSGQUEUE_S;

queue <MSGQUEUE_S> g_stMsgQueue;

void pdt_debug_print(const char *format, ...)
{
	time_t  timep = time(NULL);
	struct tm *p;
    MSGQUEUE_S stMsgQ;
    char buf[MAX_MSGBUF_SIZE] = {0};
    char buf_t[MAX_MSGBUF_SIZE] = {0};

    p = localtime(&timep);
    p->tm_year = p->tm_year + 1900;
    p->tm_mon = p->tm_mon + 1;

	va_list args;
	va_start(args, format);
	vsprintf(buf_t,format, args);
	sprintf(buf,"%s%s", buf,buf_t);
	va_end(args);
	sprintf(buf,"%s", buf);

    strcpy(stMsgQ.szMsgBuf, buf);

    stMsgQ.thread_id = GetCurrentThreadId();
	stMsgQ.stTime = *p;

	g_stMsgQueue.push(stMsgQ);

}


void pdt_debug_print_ex(int level, const char *format, ...)
{
	time_t  timep = time(NULL);
	struct tm *p;
    MSGQUEUE_S stMsgQ;
    char buf[MAX_MSGBUF_SIZE] = {0};
    char buf_t[MAX_MSGBUF_SIZE] = {0};

    p = localtime(&timep);
    p->tm_year = p->tm_year + 1900;
    p->tm_mon = p->tm_mon + 1;


	/*
	sprintf(buf, "<%04d-%02d-%02d %02d:%02d:%02d>",p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
	*/

	va_list args;
	va_start(args, format);
	//vprintf(format, args);
	vsprintf(buf_t,format, args);
	sprintf(buf,"%s%s", buf,buf_t);
	va_end(args);
	sprintf(buf,"%s", buf);

    strcpy(stMsgQ.szMsgBuf, buf);

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

void msg_dot_thread(void *pEntry)
{
	int i;

	g_dotprint = 1;

	for (int i=0; g_dotprint!=0; i++)
    {
        printf(".");
        RunDelay(500);
    }

}
void MSG_StartDot()
{
	_beginthread(msg_dot_thread,0,NULL);
}

void MSG_StopDot()
{
	g_dotprint = 0;
}

void MSGQueueMain(void *pEntry)
{
    MSGQUEUE_S stMsgQ = {0};
    for (;;)
    {
        while (!g_stMsgQueue.empty())
        {
            stMsgQ = g_stMsgQueue.front();
			printf("\r\n%04d-%02d-%02d %02d:%02d:%02d",stMsgQ.stTime.tm_year, stMsgQ.stTime.tm_mon,stMsgQ.stTime.tm_mday,
													stMsgQ.stTime.tm_hour,stMsgQ.stTime.tm_min,stMsgQ.stTime.tm_sec);
            std::cout<<"/TASKID/" << stMsgQ.thread_id << "/DEBUG: " << stMsgQ.szMsgBuf;


			/* BEGIN: Added by weizengke, 2013/11/17 */
			extern void cmd_outcurrent();
			cmd_outcurrent();
			/* END:   Added by weizengke, 2013/11/17 */


            g_stMsgQueue.pop();

            RunDelay(1);
        }
        RunDelay(1);
    }
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

