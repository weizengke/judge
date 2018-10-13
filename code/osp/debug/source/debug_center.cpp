
#include <iostream>
#include <sstream>  
#include <algorithm> 
#include <queue>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef _WIN32_
#include <windows.h>
#include <conio.h>
#include <io.h>
#include <winsock2.h>
#endif

#include "kernel.h"

#include "product/main/root.h"

#include "osp/common/include/osp_common_def.h"
#include "product/include/pdt_common_inc.h"
#include "osp/event/include/event_pub.h"
#include "osp/command/include/icli.h"
#include "osp/debug/include/debug_center_inc.h"

#if (OS_YES == OSP_MODULE_DEBUG)

using namespace std;

char logPath[MAX_PATH]="logfile";

enum DEBUG_CMO_TBLID_EM {
	DEBUG_TBL_SHOW = 0,
	DEBUG_TBL_CFG,	
};

enum DEBUG_CMO_SHOW_ID_EM {
	DEBUG_CMO_SHOW_DISPLAY = CMD_ELEMID_DEF(MID_DEBUG, DEBUG_TBL_SHOW, 0),
	DEBUG_CMO_SHOW_DEBUG,	
};

enum DEBUG_CMO_CFG_ID_EM {
	DEBUG_CMO_CFG_UNDO = CMD_ELEMID_DEF(MID_DEBUG, DEBUG_TBL_CFG, 0),
	DEBUG_CMO_CFG_ENABLE,	
	DEBUG_CMO_CFG_ENABLE_ALL,
	DEBUG_CMO_CFG_TERMINAL,
	DEBUG_CMO_CFG_COMMON,
	DEBUG_CMO_CFG_JUDGE,
	DEBUG_CMO_CFG_MYSQL,
	DEBUG_CMO_CFG_DEBUG_CENTER,
	DEBUG_CMO_CFG_CLI,
	DEBUG_CMO_CFG_EVT,
	DEBUG_CMO_CFG_NDP,
	DEBUG_CMO_CFG_AAA,
	DEBUG_CMO_CFG_TELNET,
	DEBUG_CMO_CFG_FTP,
	
};

char *szModuleName[32] = {
	"none",
	"common",
	"judge",
	"sysmng",
	"mysql",
	"debug-center",
	"command",
	"event",
	"ndp",
	"aaa",
	"telnet",
	"ftp",
	"end",
};

char *szDebugName[DEBUG_TYPE_MAX] = {
	"none",
	"error",
	"function",
	"info",
	"message",
};


static const char* LEVEL_NAME[] = {"INFO", "WARNING", "ERROR", "FATAL", "SYSTEM_ERROR"};

unsigned long g_aulDebugMask[MID_ID_END][DEBUG_TYPE_MAX/32 + 1] = {0};
int g_debug_switch = DEBUG_DISABLE;
queue <MSGQUEUE_S> g_stMsgQueue;

#define DEBUG_Debug(x, args...) debugcenter_print(MID_DEBUG, x, args)

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
    stMsgQ.thread_id = thread_get_self();
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
    stMsgQ.thread_id = thread_get_self();
	stMsgQ.stTime = *p;

	g_stMsgQueue.push(stMsgQ);

}

void RunDelay(int t)
{
    //std::this_thread::sleep_for(std::chrono::milliseconds(t));
    Sleep(t);
}

int g_dotprint = 0;

int msg_dot_thread(void *pEntry)
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
	(void)thread_create(msg_dot_thread, NULL);
}

void MSG_StopDot()
{
	g_dotprint = 0;
}


void write_log(int level, const char *fmt, ...) {
	va_list ap;
	char buffer[BUFSIZE];
	time_t  timep = time(NULL);
	int l;
	struct tm *p;

	if( (file_access(logPath, 0 )) == -1 )
	{
		create_directory(logPath);
	}

    p = localtime(&timep);
    p->tm_year = p->tm_year + 1900;
    p->tm_mon = p->tm_mon + 1;
	sprintf(buffer,"%s/%04d-%02d-%02d.log", logPath, p->tm_year, p->tm_mon, p->tm_mday);

	FILE *fp = fopen(buffer, "a+");
	if (fp == NULL) {
		fprintf(stderr, "open logfile error!\n");
		return;
	}

	fprintf(fp, "%s:%04d-%02d-%02d %02d:%02d:%02d ",LEVEL_NAME[level],p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);

	va_start(ap, fmt);
	l = vsnprintf(buffer, BUFSIZE, fmt, ap);
	fprintf(fp, "%s\r\n", buffer);

	/* BEGIN: Added by weizengke, 2013/11/15 for vrp */
	//pdt_debug_print("%s", buffer);
	DEBUG_Debug(DEBUG_TYPE_INFO, "%s", buffer);
	/* END:   Added by weizengke, 2013/11/15 */

	va_end(ap);
	fclose(fp);
}

VOID DEBUG_Show(ULONG vtyId)
{
	ULONG m = 0;
	ULONG i = 0;
	
	if (g_debug_switch == DEBUG_ENABLE)
	{
		vty_printf(vtyId, "Global debugging is enable.\r\n");
	}
	else
	{
		vty_printf(vtyId, "Global debugging is disable.\r\n");
	}

#if 0
	vty_printf(vtyId, " DebugMask(0x%x", g_aulDebugMask[0]);
	for (i = 1; i < DEBUG_TYPE_MAX/DEBUG_MASKLENTG + 1 ; i++)
	{
		vty_printf(vtyId, "	,0x%x", g_aulDebugMask[i]);
	}
	
	vty_printf(vtyId, ").\r\n");
#endif

	for (m = MID_NULL+1; m < MID_ID_END; m++)
	{
		for (i = DEBUG_TYPE_NONE + 1; i < DEBUG_TYPE_MAX; i++ )
		{
			if (DEBUG_MASK_GET(m, i))
			{
				vty_printf(vtyId, " Debugging %s %s switch is on.\r\n", szModuleName[m], szDebugName[i]);
			}
		}
	}

	return ;

}

ULONG DEBUG_Enable(ULONG ulUndo)
{
	if (OS_YES == ulUndo)
	{
		g_debug_switch = DEBUG_DISABLE;
	}
	else
	{
		g_debug_switch = DEBUG_ENABLE;
	}

	return 0;
}

ULONG DEBUG_EnableAll(ULONG ulUndo)
{
	ULONG i;
	ULONG m= 0;
	for (m = MID_NULL+1; m < MID_ID_END; m++)
	{
		for (i = DEBUG_TYPE_NONE + 1; i < DEBUG_TYPE_MAX; i++ )
		{
			if (OS_YES == ulUndo)
			{
				DEBUG_MASK_CLEAR(m, i);
			}
			else
			{
				DEBUG_MASK_SET(m, i);
			}			
		}
	}

	return 0;
}

ULONG DEBUG_CFG_Show(VOID *pRcvMsg)
{
	ULONG iLoop = 0;
	ULONG iElemNum = 0;
	ULONG iElemID = 0;
	VOID *pElem = NULL;
	ULONG vtyId = 0;

	vtyId = cmd_get_vty_id(pRcvMsg);
	iElemNum = cmd_get_elem_num(pRcvMsg);
	OS_DBGASSERT(iElemNum, "DEBUG_CFG_Show, element num is 0");

	for (iLoop = 0; iLoop < iElemNum; iLoop++)
	{	
		pElem = cmd_get_elem_by_index(pRcvMsg, iLoop);
		iElemID = cmd_get_elemid(pElem);

		switch(iElemID)
		{
			case DEBUG_CMO_SHOW_DEBUG:	
				DEBUG_Show(vtyId);
				break;
				
			default:
				break;
		}
	}

	return 0;
}

ULONG DEBUG_CFG_Enalbe(VOID *pRcvMsg)
{
	ULONG iLoop = 0;
	ULONG iElemNum = 0;
	ULONG iElemID = 0;
	VOID *pElem = NULL;
	ULONG vtyId = 0;
	ULONG ulUndo = OS_NO;
	ULONG ulEnable = OS_NO;
	ULONG ulEnableAll = OS_NO;
	ULONG ulTerm = OS_NO;
	
	vtyId = cmd_get_vty_id(pRcvMsg);
	iElemNum = cmd_get_elem_num(pRcvMsg);

	OS_DBGASSERT(iElemNum, "DEBUG_CFG_Enalbe, cmd element num is 0");

	for (iLoop = 0; iLoop < iElemNum; iLoop++)
	{	
		pElem = cmd_get_elem_by_index(pRcvMsg, iLoop);
		iElemID = cmd_get_elemid(pElem);

		switch(iElemID)
		{
			case DEBUG_CMO_CFG_UNDO:
				ulUndo = OS_YES;
				break;
				
			case DEBUG_CMO_CFG_ENABLE:	
				ulEnable = OS_YES;
				break;
				
			case DEBUG_CMO_CFG_ENABLE_ALL:	
				ulEnableAll = OS_YES;
				break;
				
			case DEBUG_CMO_CFG_TERMINAL:
				ulTerm = OS_YES;
				break;
				
			default:
				break;
		}
	}

	if (OS_YES == ulTerm)
	{
		CMD_VTY_S * vty = NULL;
		
		extern CMD_VTY_S *cmd_vty_getById(ULONG vtyId);
		vty = cmd_vty_getById(vtyId);
		if (NULL != vty)
		{
			if (OS_YES == ulUndo)
			{
				vty->user.terminal_debugging = OS_NO;
			}
			else
			{
				vty->user.terminal_debugging = OS_YES;
			}
		}

		return 0;
	}
	
	if (OS_YES == ulEnableAll)
	{
		DEBUG_EnableAll(ulUndo);
		return 0;
	}
	else
	{
		DEBUG_Enable(ulUndo);
		return 0;
	}

	return 0;

}

ULONG DEBUG_CfgCallback(VOID *pRcvMsg)
{
	ULONG iRet = 0;
	ULONG iTBLID = 0;
	
	iTBLID = cmd_get_first_elem_tblid(pRcvMsg);

	switch(iTBLID)
	{
		case DEBUG_TBL_SHOW:
			iRet = DEBUG_CFG_Show(pRcvMsg);
			break;
			
		case DEBUG_TBL_CFG:
			iRet = DEBUG_CFG_Enalbe(pRcvMsg);
			break;

		default:
			break;
	}

	return iRet;
}

ULONG DEBUG_RegCmdShow()
{
	CMD_VECTOR_S * vec = NULL;
	
	/* 命令行注册四部曲1: 申请命令行向量 */
	CMD_VECTOR_NEW(vec);

	/* 命令行注册四部曲2: 定义命令字 */
	cmd_regelement_new(CMD_ELEMID_NULL, 		CMD_ELEM_TYPE_KEY,    "display",     "Display", vec);
	cmd_regelement_new(DEBUG_CMO_SHOW_DEBUG, 	CMD_ELEM_TYPE_KEY,    "debugging", 	 "Debugging", vec);

	/* 命令行注册四部曲3: 注册命令行 */
	cmd_install_command(MID_DEBUG, VIEW_GLOBAL, " 1 2 ", vec);

	/* 命令行注册四部曲4: 释放命令行向量 */
	CMD_VECTOR_FREE(vec);

	return OS_OK;
}

ULONG DEBUG_RegCmdConfig()
{
	CMD_VECTOR_S * vec = NULL;
		
	/* 命令行注册四部曲1: 申请命令行向量 */
	CMD_VECTOR_NEW(vec);

	/* 命令行注册四部曲2: 定义命令字 */
	cmd_regelement_new(DEBUG_CMO_CFG_UNDO,       CMD_ELEM_TYPE_KEY,   "undo", 	    "Undo operation", vec);	
	cmd_regelement_new(CMD_ELEMID_NULL,          CMD_ELEM_TYPE_KEY,   "debugging", 	"Debugging switch", vec);	
	cmd_regelement_new(DEBUG_CMO_CFG_ENABLE,     CMD_ELEM_TYPE_KEY,   "enable", 		"Enable the debugging", vec);
	cmd_regelement_new(DEBUG_CMO_CFG_ENABLE_ALL, CMD_ELEM_TYPE_KEY,   "all", 			"Enable all the debugging", vec);
	cmd_regelement_new(DEBUG_CMO_CFG_TERMINAL,   CMD_ELEM_TYPE_KEY,   "terminal", 	"Terminal", vec);
	cmd_regelement_new(DEBUG_CMO_CFG_COMMON,     CMD_ELEM_TYPE_KEY,   "common", 		"Common Module", vec);
	cmd_regelement_new(DEBUG_CMO_CFG_JUDGE,      CMD_ELEM_TYPE_KEY,   "judge", 		"Judge Module", vec);
	cmd_regelement_new(DEBUG_CMO_CFG_MYSQL,      CMD_ELEM_TYPE_KEY,   "mysql", 		"Mysql Module", vec);
	cmd_regelement_new(DEBUG_CMO_CFG_DEBUG_CENTER, CMD_ELEM_TYPE_KEY, "debug-center", "Debug Module", vec);
	cmd_regelement_new(DEBUG_CMO_CFG_CLI,        CMD_ELEM_TYPE_KEY,   "command", 		"Command Module", vec);
	cmd_regelement_new(DEBUG_CMO_CFG_EVT,        CMD_ELEM_TYPE_KEY,   "event", 		"Event Module", vec);
	cmd_regelement_new(DEBUG_CMO_CFG_NDP,        CMD_ELEM_TYPE_KEY,   "ndp", 			"NDP Module", vec);
	cmd_regelement_new(DEBUG_CMO_CFG_AAA,        CMD_ELEM_TYPE_KEY,   "aaa", 			"AAA Module", vec);
	cmd_regelement_new(DEBUG_CMO_CFG_TELNET,     CMD_ELEM_TYPE_KEY,   "telnet", 		"Telnet Module", vec);
	cmd_regelement_new(DEBUG_CMO_CFG_FTP,        CMD_ELEM_TYPE_KEY,   "ftp", 		    "FTP Module", vec);
	
	/* 命令行注册四部曲3: 注册命令行 */
	cmd_install_command(MID_DEBUG, VIEW_USER, " 1 2 3 ", vec); /* undo debugging enable */
	cmd_install_command(MID_DEBUG, VIEW_USER, " 2 3 ", vec);   /* debugging enable */
	cmd_install_command(MID_DEBUG, VIEW_USER, " 1 2 4 ", vec); /* undo debugging all */
	cmd_install_command(MID_DEBUG, VIEW_USER, " 2 4 ", vec); /* debugging all */
	cmd_install_command(MID_DEBUG, VIEW_USER, " 5 2 ", vec); /* terminal debugging */
	cmd_install_command(MID_DEBUG, VIEW_USER, " 1 5 2 ", vec); /* undo terminal debugging */
	
	/* 命令行注册四部曲4: 释放命令行向量 */
	CMD_VECTOR_FREE(vec);

	return OS_OK;
}

ULONG DEBUG_RegCmd()
{
	(VOID)DEBUG_RegCmdShow();

	(VOID)DEBUG_RegCmdConfig();
}

int DEBUG_MainEntry(void *pEntry)
{
    MSGQUEUE_S stMsgQ;
	char module[32] = {0};
	char type[32] = {0};
	char buff[BUFSIZE] = {0};

	(VOID)DEBUG_RegCmd();
	
	(VOID)cmd_regcallback(MID_DEBUG, DEBUG_CfgCallback);
	
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

			extern void vty_print2all(char *format, ...);
			vty_print2all(buff);

            g_stMsgQueue.pop();
			
            RunDelay(1);
        }

        RunDelay(1);

    }

	return 0;
}

APP_INFO_S g_DebugAppInfo =
{
	MID_DEBUG,
	"Debug",
	NULL,
	DEBUG_MainEntry
};

void Debug_RegAppInfo()
{
	APP_RegistInfo(&g_DebugAppInfo);
}

#endif
