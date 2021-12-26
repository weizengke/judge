
#include <iostream>
#include <sstream>  
#include <algorithm> 
#include <queue>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include<stdlib.h>
#include <stdarg.h>

#ifdef WIN32
#include <windows.h>
#include <io.h>
#endif

#include "kernel.h"
#include "root.h"
#include "util/util.h"
#include "securec.h"

#include "osp_common_def.h"
#include "pdt_common_inc.h"
#include "event/include/event_pub.h"
#include "../include/icli.h"
#include "ic/include/debug_center_inc.h"

#if (OS_YES == OSP_MODULE_DEBUG)

using namespace std;

char logPath[MAX_PATH]="logfile";
mutex_t hSemaphoreDebug;

enum DEBUG_CMO_TBLID_EM {
	DEBUG_TBL_SHOW = 0,
	DEBUG_TBL_CFG,	
	DEBUG_TBL_USERDEF, /* �û��Զ���ģ��TBLID */
	
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
};

enum DEBUG_CMO_UDF_ID_EM {
	DEBUG_CMO_UDF_UNDO = CMD_ELEMID_DEF(MID_DEBUG, DEBUG_TBL_USERDEF, 0),
	DEBUG_CMO_UDF_DBG_ALL,
	DEBUG_CMO_UDF_DBG_ERROR,
	DEBUG_CMO_UDF_DBG_FUNC,
	DEBUG_CMO_UDF_DBG_INFO,
	DEBUG_CMO_UDF_DBG_MESSAGE,
	/* ����Ϊ�û��Զ���ģ��id */
};

char *g_szModuleName[32] = {
	"none",
	"common",
	"sysmng",
	"judge",	
	"mysql",
	"debug-center",
	"cli",
	"event",
	"ndp",
	"aaa",
	"telnet",
	"ftp",
	"end",
};

char *g_szDebugName[DEBUG_TYPE_MAX] = {
	"none",
	"error",
	"function",
	"info",
	"message",
};


static const char* LEVEL_NAME[] = {"INFO", "WARNING", "ERROR", "FATAL", "SYSTEM_ERROR"};

unsigned long g_aulDebugMask[MID_ID_END][DEBUG_TYPE_MAX/32 + 1] = {0};
int g_debug_switch = DEBUG_DISABLE;
queue <char*> g_stMsgQueue;

typedef struct tagDEBUG_MODULE_INFO_S {
	ULONG ulCMOId;
	ULONG ulMid;
	ULONG ulDebugMask;
	CHAR szModuleName[CMD_MAX_CMD_ELEM_SIZE];
	CHAR szModuleHelp[CMD_MAX_CMD_ELEM_HELP_SIZE];
} DEBUG_MODULE_INFO_S;

DEBUG_MODULE_INFO_S g_stDebugModuleInfo[128] = {0};
ULONG g_ulDebugModuleNum = 0;
ULONG g_DebugModuleElemid = DEBUG_CMO_UDF_DBG_MESSAGE  + 1;

#define DEBUG_Debug(x, format, ...) debugcenter_print(MID_DEBUG, x, format, ##__VA_ARGS__)

VOID DEBUG_CreateSem()
{
	hSemaphoreDebug = mutex_create("DEBUG_SEM");
}
VOID DEBUG_SemP()
{
	(void)mutex_lock(hSemaphoreDebug);
}

VOID DEBUG_SemV()
{
	(void)mutex_unlock(hSemaphoreDebug);
}

ULONG DEBUG_GetModuleIndex(ULONG mid)
{
	ULONG ulLoop = 0;
	for (ulLoop= 0; ulLoop < g_ulDebugModuleNum; ulLoop++)
	{
		if (g_stDebugModuleInfo[ulLoop].ulMid == mid)
		{
			return ulLoop;
		}
	}

	return 0xFFFFFFFF;
}

void debugcenter_print(MID_ID_EM mid, DEBUG_TYPE_EM type, const char *format, ...)
{
	ULONG ulIndex = DEBUG_GetModuleIndex(mid);
	if (0xFFFFFFFF == ulIndex)
	{
		return;
	}

	if (!DEBUG_TYPE_ISVALID(type))
	{
		return;
	}

	if (!DEBUG_MASK_GET(ulIndex, type))
	{
		return;
	}

	time_t  timep = time(NULL);
	struct tm *p;
	char buf_t[DEBUG_BUFF_MAX_LEN] = {0};
    char *buff = (char*)malloc(DEBUG_BUFF_MAX_LEN);
	if (buff == NULL) {
		return;
	}
    p = localtime(&timep);
    p->tm_year = p->tm_year + 1900;
    p->tm_mon = p->tm_mon + 1;

	va_list args;
	va_start(args, format);
	vsnprintf(buf_t, DEBUG_BUFF_MAX_LEN, format, args);
	va_end(args);

	buf_t[DEBUG_BUFF_MAX_LEN - 1] = '\0';

	char module[32] = {0};
	char typeName[32] = {0};
	strcpy_s(module, sizeof(module), g_szModuleName[mid]);
	strcpy_s(typeName, sizeof(typeName), g_szDebugName[type]);
	std::transform(module, module + 31,module,::toupper);  
	std::transform(typeName, typeName + 31,typeName,::toupper);  
	
	snprintf(buff, DEBUG_BUFF_MAX_LEN, "%04d-%02d-%02d %02d:%02d:%02d/%u/DEBUG/%s/%s:%s\r\n",
				p->tm_year, p->tm_mon, p->tm_mday,
				p->tm_hour, p->tm_min, p->tm_sec, 
				thread_get_self(), module, typeName, buf_t);

	DEBUG_SemP();
	g_stMsgQueue.push(buff);
	DEBUG_SemV();
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
	thread_create(msg_dot_thread, NULL);
}

void MSG_StopDot()
{
	g_dotprint = 0;
}


void write_log(int level, const char *fmt, ...) {
	va_list ap;
	char buffer[DEBUG_BUFF_MAX_LEN] = {0};
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
	l = vsnprintf(buffer, DEBUG_BUFF_MAX_LEN, fmt, ap);
	fprintf(fp, "%s\n", buffer);

	/* BEGIN: Added by weizengke, 2013/11/15 for vrp */
	DEBUG_Debug(DEBUG_TYPE_INFO, "%s", buffer);
	/* END:   Added by weizengke, 2013/11/15 */

	va_end(ap);
	fclose(fp);
}

VOID DEBUG_Show(ULONG vtyId)
{
	ULONG m = 0;
	ULONG i = 0;

#if 0	
	if (g_debug_switch == DEBUG_ENABLE)
	{
		vty_printf(vtyId, "Global debugging is enable.\r\n");
	}
	else
	{
		vty_printf(vtyId, "Global debugging is disable.\r\n");
	}

	vty_printf(vtyId, " DebugMask(0x%x", g_aulDebugMask[0]);
	for (i = 1; i < DEBUG_TYPE_MAX/DEBUG_MASKLENTG + 1 ; i++)
	{
		vty_printf(vtyId, "	,0x%x", g_aulDebugMask[i]);
	}
	
	vty_printf(vtyId, ").\r\n");
#endif

	for (m = MID_NULL+1; m < MID_ID_END; m++)
	{
		ULONG ulIndex = DEBUG_GetModuleIndex(m);
		if (0xFFFFFFFF == ulIndex)
		{
			continue;
		}
		
		for (i = DEBUG_TYPE_NONE + 1; i < DEBUG_TYPE_MAX; i++ )
		{
			if (DEBUG_MASK_GET(ulIndex, i))
			{
				vty_printf(vtyId, " Debugging %s %s switch is on.\r\n",
							g_stDebugModuleInfo[ulIndex].szModuleName,
							g_szDebugName[i]);
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
	for (m = 0; m < g_ulDebugModuleNum; m++)
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
	ULONG ulEnableAll = OS_NO;
	ULONG ulEnableByModule = OS_NO;
	ULONG ulTerm = OS_NO;
	CHAR szModule[128] = {0};
	
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

	return 0;

}

ULONG DEBUG_CFG_UserDebug(VOID *pRcvMsg)
{
	ULONG iLoop = 0;
	ULONG iElemNum = 0;
	ULONG iElemID = 0;
	VOID *pElem = NULL;
	ULONG vtyId = 0;
	ULONG ulUndo = OS_NO;
	ULONG ulEnableAll = OS_NO;
	ULONG ulDebugType = 0;
	ULONG ulMid = 0;
	ULONG iLoop1 = 0;
	vtyId = cmd_get_vty_id(pRcvMsg);
	iElemNum = cmd_get_elem_num(pRcvMsg);

	OS_DBGASSERT(iElemNum, "DEBUG_CFG_UserDebug, cmd element num is 0");

	for (iLoop = 0; iLoop < iElemNum; iLoop++)
	{	
		pElem = cmd_get_elem_by_index(pRcvMsg, iLoop);
		iElemID = cmd_get_elemid(pElem);

		switch(iElemID)
		{
			case DEBUG_CMO_UDF_UNDO:
				ulUndo = OS_YES;
				break;
				
			case DEBUG_CMO_UDF_DBG_ALL:	
				ulEnableAll = OS_YES;
				break;
				
			case DEBUG_CMO_UDF_DBG_ERROR: 
				ulDebugType = DEBUG_TYPE_ERROR;
				break;
			case DEBUG_CMO_UDF_DBG_INFO:	
				ulDebugType = DEBUG_TYPE_INFO;
				break;
			case DEBUG_CMO_UDF_DBG_FUNC:	
				ulDebugType = DEBUG_TYPE_FUNC;
				break;
			case DEBUG_CMO_UDF_DBG_MESSAGE:	
				ulDebugType = DEBUG_TYPE_MSG;
				break;
			
			default:

				for (iLoop1 = 0; iLoop1 < g_ulDebugModuleNum; iLoop1++)
				{
					if (iElemID == g_stDebugModuleInfo[iLoop1].ulCMOId)
					{
						ulMid = g_stDebugModuleInfo[iLoop1].ulMid;
						break;
					}
				}
				break;
		}
	}

	ULONG ulIndex = DEBUG_GetModuleIndex(ulMid);
	if (ulIndex == 0xFFFFFFFF)
	{
		return 0;
	}
	
	if (OS_YES == ulEnableAll)
	{
		for (iLoop1 = DEBUG_TYPE_NONE + 1; iLoop1 < DEBUG_TYPE_MAX; iLoop1++)
		{
			if (OS_YES == ulUndo)
			{
				DEBUG_MASK_CLEAR(ulIndex, iLoop1);
			}
			else
			{
				DEBUG_MASK_SET(ulIndex, iLoop1);
			}			
		}
		return 0;
	}
	else
	{
		DEBUG_MASK_SET(ulIndex, ulDebugType);
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

		case DEBUG_TBL_USERDEF:
			iRet = DEBUG_CFG_UserDebug(pRcvMsg);
			break;
		

		default:
			break;
	}

	return iRet;
}

ULONG DEBUG_RegCmdShow()
{
	CMD_VECTOR_S * vec = NULL;
	
	/* ������ע���Ĳ���1: �������������� */
	CMD_VECTOR_NEW(vec);

	/* ������ע���Ĳ���2: ���������� */
	cmd_regelement_new(CMD_ELEMID_NULL, 		CMD_ELEM_TYPE_KEY,    "display",     "Display", vec);
	cmd_regelement_new(DEBUG_CMO_SHOW_DEBUG, 	CMD_ELEM_TYPE_KEY,    "debugging", 	 "Debugging", vec);

	/* ������ע���Ĳ���3: ע�������� */
	cmd_install_command(MID_DEBUG, VIEW_GLOBAL, " 1 2 ", vec);

	/* ������ע���Ĳ���4: �ͷ����������� */
	CMD_VECTOR_FREE(vec);

	return OS_OK;
}

ULONG DEBUG_ModuleHelpCallback(VOID *pRcvMsg, CMD_ELMTHELP_S **ppstCmdElmtHelp, ULONG *pulNum)
{
	ULONG ulRet = OS_OK;
	CMD_ELMTHELP_S *pstCmdElmtHelp = NULL;
	
	pstCmdElmtHelp = (CMD_ELMTHELP_S*)malloc(2 * sizeof(CMD_ELMTHELP_S));
	if (NULL == pstCmdElmtHelp)
	{
		return OS_ERR;
	}
	memset(pstCmdElmtHelp, 0, sizeof(2 * sizeof(CMD_ELMTHELP_S)));

	*ppstCmdElmtHelp = pstCmdElmtHelp;
	
	strcpy(pstCmdElmtHelp[0].szElmtName, "telnet");
	strcpy(pstCmdElmtHelp[0].szElmtHelp, "Telnet proocol.");
	
	strcpy(pstCmdElmtHelp[1].szElmtName, "judge");
	strcpy(pstCmdElmtHelp[1].szElmtHelp, "Judger.");
	
	*pulNum = 2;
	
	return OS_OK;

}

ULONG DEBUG_ModuleCheckCallback(VOID *pRcvMsg)
{	
	CHAR *pszInput = (CHAR*)pRcvMsg;

	if (NULL == pszInput)
	{
		OS_ERR;
	}
	
	if (0 == util_strnicmp("telnet", pszInput, strlen(pszInput)))
	{
		return OS_OK;
	}
	
	if (0 == util_strnicmp("judge", pszInput, strlen(pszInput)))
	{
		return OS_OK;
	}

	return OS_ERR;
}

ULONG DEBUG_RegCmdConfig()
{
	CMD_VECTOR_S * vec = NULL;
		
	/* ������ע���Ĳ���1: �������������� */
	CMD_VECTOR_NEW(vec);

	/* ������ע���Ĳ���2: ���������� */
	cmd_regelement_new(DEBUG_CMO_CFG_UNDO,       CMD_ELEM_TYPE_KEY,   "undo", 	    	"Undo operation", vec);	
	cmd_regelement_new(CMD_ELEMID_NULL,          CMD_ELEM_TYPE_KEY,   "debugging", 		"Debugging switch", vec);	
	cmd_regelement_new(DEBUG_CMO_CFG_ENABLE,     CMD_ELEM_TYPE_KEY,   "enable", 		"Enable the debugging", vec);
	cmd_regelement_new(DEBUG_CMO_CFG_ENABLE_ALL, CMD_ELEM_TYPE_KEY,   "all", 			"Enable all the debugging", vec);
	cmd_regelement_new(DEBUG_CMO_CFG_TERMINAL,   CMD_ELEM_TYPE_KEY,   "terminal", 		"Terminal", vec);

	/* ������ע���Ĳ���3: ע�������� */
	//cmd_install_command(MID_DEBUG, VIEW_USER, " 1 2 3 ", vec); /* undo debugging enable */
	//cmd_install_command(MID_DEBUG, VIEW_USER, " 2 3 ", vec);   /* debugging enable */
	cmd_install_command(MID_DEBUG, VIEW_USER, " 1 2 4 ", vec); /* undo debugging all */
	cmd_install_command(MID_DEBUG, VIEW_USER, " 2 4 ", vec); /* debugging all */
	cmd_install_command(MID_DEBUG, VIEW_USER, " 5 2 ", vec); /* terminal debugging */
	cmd_install_command(MID_DEBUG, VIEW_USER, " 1 5 2 ", vec); /* undo terminal debugging */

	/* ������ע���Ĳ���4: �ͷ����������� */
	CMD_VECTOR_FREE(vec);

	return OS_OK;
}

CMD_VECTOR_S * g_DebugModulevVec = NULL;


ULONG DEBUG_PUB_RegModuleDebugs(ULONG ulMid, CHAR *szModuleName, CHAR *szModuleHelp)
{
	CHAR szCmdBuf[1024] = {0};
	ULONG ulLoop = 0;
	
	if (NULL == g_DebugModulevVec)
	{
		/* ������ע���Ĳ���1: �������������� */
		CMD_VECTOR_NEW(g_DebugModulevVec);
		
		/* ������ע���Ĳ���2: ���������� */
		cmd_regelement_new(DEBUG_CMO_UDF_UNDO,		 CMD_ELEM_TYPE_KEY,   "undo",			"Undo operation", g_DebugModulevVec); 
		cmd_regelement_new(CMD_ELEMID_NULL, 		 CMD_ELEM_TYPE_KEY,   "debugging",		"Debugging switch", g_DebugModulevVec);	
		cmd_regelement_new(DEBUG_CMO_UDF_DBG_ALL,	 CMD_ELEM_TYPE_KEY,   "all",			"Enable all the debugging", g_DebugModulevVec);
		cmd_regelement_new(DEBUG_CMO_UDF_DBG_ERROR,   CMD_ELEM_TYPE_KEY,  "error", 			"Debugging error level", g_DebugModulevVec);
		cmd_regelement_new(DEBUG_CMO_UDF_DBG_FUNC,	 CMD_ELEM_TYPE_KEY,   "function",		"Debugging function level", g_DebugModulevVec);
		cmd_regelement_new(DEBUG_CMO_UDF_DBG_INFO,	 CMD_ELEM_TYPE_KEY,   "info",			"Debugging info level", g_DebugModulevVec);
		cmd_regelement_new(DEBUG_CMO_UDF_DBG_MESSAGE,CMD_ELEM_TYPE_KEY,   "message",		"Debugging message level", g_DebugModulevVec);		
	}

	cmd_regelement_new(g_DebugModuleElemid,	 CMD_ELEM_TYPE_KEY,   szModuleName, 	szModuleHelp, g_DebugModulevVec);

	g_stDebugModuleInfo[g_ulDebugModuleNum].ulCMOId = g_DebugModuleElemid;
	g_stDebugModuleInfo[g_ulDebugModuleNum].ulMid = ulMid;
	g_stDebugModuleInfo[g_ulDebugModuleNum].ulDebugMask = 0;
	strcpy(g_stDebugModuleInfo[g_ulDebugModuleNum].szModuleName, szModuleName);
	strcpy(g_stDebugModuleInfo[g_ulDebugModuleNum].szModuleHelp, szModuleHelp);
	g_ulDebugModuleNum++;
	
	/* ������ע���Ĳ���3: ע�������� */
	for (ulLoop = DEBUG_CMO_UDF_DBG_ALL - DEBUG_CMO_UDF_UNDO + 2; ulLoop <= DEBUG_CMO_UDF_DBG_MESSAGE - DEBUG_CMO_UDF_UNDO + 2; ulLoop++)
	{
		memset(szCmdBuf, 0, sizeof(szCmdBuf));
		sprintf(szCmdBuf, " 1 2 %u %u ", g_DebugModuleElemid - DEBUG_CMO_UDF_UNDO + 2, ulLoop);  /* undo debugging xxx all */
		cmd_install_command(MID_DEBUG, VIEW_USER, szCmdBuf, g_DebugModulevVec);
		
		memset(szCmdBuf, 0, sizeof(szCmdBuf));
		sprintf(szCmdBuf, " 2 %u %u ", g_DebugModuleElemid - DEBUG_CMO_UDF_UNDO + 2, ulLoop);  /* debugging xxx all */
		cmd_install_command(MID_DEBUG, VIEW_USER, szCmdBuf, g_DebugModulevVec);
	}

	g_DebugModuleElemid++;
	
	//CMD_VECTOR_FREE(g_DebugModulevVec);

	return OS_OK;
}

ULONG DEBUG_RegCmd()
{
	(VOID)DEBUG_RegCmdShow();

	(VOID)DEBUG_RegCmdConfig();

	return 0;
}

int DEBUG_MainEntry(void *pEntry)
{
    char* msg;
	char buff[DEBUG_BUFF_MAX_LEN] = {0};
	
	DEBUG_CreateSem();

	(VOID)DEBUG_RegCmd();
	
	(VOID)cmd_regcallback(MID_DEBUG, DEBUG_CfgCallback);

	DEBUG_PUB_RegModuleDebugs(MID_DEBUG, "debug-center", "Debug Center");
	
    for (;;) {
		DEBUG_SemP();
        while (!g_stMsgQueue.empty()) {
            msg = g_stMsgQueue.front();
			g_stMsgQueue.pop();

			extern void vty_print2all(char *format, ...);
			vty_print2all(msg);
			free(msg);
			
            RunDelay(1);
        }

		DEBUG_SemV();
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
