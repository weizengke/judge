#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>

#ifdef _LINUX_
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <assert.h>
#endif

#ifdef _WIN32_
#include <conio.h>
#include <io.h>
#include <winsock2.h>
#endif

#include "kernel.h"

#include "osp/common/include/osp_common_def.h"
#include "product/include/pdt_common_inc.h"
#include "osp/event/include/event_pub.h"
#include "osp/aaa/aaa.h"
#include "osp/command/include/icli.h"
#include "product/main/root.h"

#if (OS_YES == OSP_MODULE_AAA)

using namespace std;

AAA_USER_S g_stAAAUser[AAA_USER_MAX_NUM] = {0};
EVENT_NTF_NODE *pstAAAEvtTbl = NULL;

#define AAA_Debug(x, format, ...) debugcenter_print(MID_AAA, x, format, ##__VA_ARGS__)

ULONG AAA_EvtInit()
{
	ULONG ulRet = OS_OK;

	ulRet = EVENT_InitTbl(&pstAAAEvtTbl, AAA_EVT_MAX);

	return ulRet;
}

ULONG AAA_EvtRegistFunc(CHAR *pModuleName, ULONG eventId, ULONG priority, EVENT_CB_FUNC pfCallBackFunc)
{
	ULONG ulRet = OS_OK;

	if (NULL == pstAAAEvtTbl)
	{
		ulRet = AAA_EvtInit();
		if (OS_OK != ulRet)
		{
			return ulRet;
		}
	}
	
	ulRet = EVENT_RegistFunc(pstAAAEvtTbl, pModuleName, eventId, priority, pfCallBackFunc);
	
	return ulRet;
}

ULONG AAA_EvtNotify(ULONG evtId, ULONG keyId, ULONG cmdId, VOID *pData, VOID **ppInfo)
{
	ULONG ulRet = OS_OK;

	if (NULL == pstAAAEvtTbl)
	{
		ulRet = AAA_EvtInit();
		if (OS_OK != ulRet)
		{
			return ulRet;
		}
	}
	
	ulRet = EVENT_Notify(pstAAAEvtTbl, evtId, keyId, cmdId, pData, ppInfo);
	
	return ulRet;
}


AAA_USER_S * AAA_LookupUser(char *uname)
{
	for (int i = 0; i < AAA_USER_MAX_NUM; i++)
	{
		if (1 == g_stAAAUser[i].used
			&& 0 == strcmp(uname, g_stAAAUser[i].user_name))
		{
			return &g_stAAAUser[i];
		}
	}

	return NULL;
}

AAA_USER_S * AAA_GetIdleUser(char *uname, char *psw)
{
	for (int i = 0; i < AAA_USER_MAX_NUM; i++)
	{
		if (0 == g_stAAAUser[i].used)
		{
			memset(&g_stAAAUser[i], 0, sizeof(AAA_USER_S));
			return &g_stAAAUser[i];
		}
	}

	return NULL;
}


ULONG AAA_AddUser(ULONG vtyId, char *uname, char *psw, ULONG ulServiceType)
{
	AAA_USER_S * user = NULL;

	user = AAA_LookupUser(uname);
	if (NULL == user)
	{	
		if (0 == strlen(psw))
		{
			vty_printf(vtyId, "Error: User %s is not created.\r\n", uname);
			return OS_ERR;
		}
		
		user = AAA_GetIdleUser(uname, psw);
		if (NULL == user)
		{
			vty_printf(vtyId, "Error: Create user failed, because exceed the max number(%d).\r\n", AAA_USER_MAX_NUM);
			return OS_ERR;
		}

		user->used = OS_YES;
		strcpy(user->user_name, uname);		
	}

	/* 更新密码 */
	if (0 != strlen(psw)
		&& strcmp(user->user_psw, psw))
	{
		strcpy(user->user_psw, psw);
		
		(VOID)AAA_EvtNotify(AAA_EVT_USER_PSW_CHANGE, 0, 0, (VOID*)uname, 0);
	}

	/* 更新服务类型 */
	user->service_type |= ulServiceType;
		
	return OS_OK;
}

ULONG AAA_DelUser(ULONG vtyId, char *uname, ULONG ulServiceType)
{
	AAA_USER_S * user = NULL;

	user = AAA_LookupUser(uname);
	if (NULL == user)
	{	
		vty_printf(vtyId, "Error: The user %s is not exist.\r\n", uname);
		return OS_ERR;
	}

	if (0 == ulServiceType)
	{
		memset(user, 0, sizeof(AAA_USER_S));
		
		(VOID)AAA_EvtNotify(AAA_EVT_USER_DEL, 0, 0, (VOID*)uname, 0);
	}
	else
	{
		user->service_type &= ~ulServiceType;

		if (AAA_MASK_GET(ulServiceType, AAA_SERVICE_TYPE_TELNET))
		{
			(VOID)AAA_EvtNotify(AAA_EVT_USER_SERVICE_TELNET_DEL, 0, 0, (VOID*)uname, 0);
		}

		if (AAA_MASK_GET(ulServiceType, AAA_SERVICE_TYPE_FTP))
		{
			(VOID)AAA_EvtNotify(AAA_EVT_USER_SERVICE_FTP_DEL, 0, 0, (VOID*)uname, 0);
		}
	}

	return OS_OK;
}

ULONG AAA_UserAuth(char *uname, char *psw, ULONG type)
{
	AAA_USER_S * user = NULL;

	user = AAA_LookupUser(uname);
	if (NULL == user)
	{	
		return OS_ERR;
	}

	if (0 == strcmp(user->user_psw, psw)
		&& AAA_MASK_GET(user->service_type, type))
	{	
		return OS_OK;
	}

	return OS_ERR;
}

ULONG AAA_BuildRun(CHAR **ppBuildrun, ULONG ulIncludeDefault)
{
	CHAR *pBuildrun = NULL;

	*ppBuildrun = (CHAR*)malloc(BDN_MAX_BUILDRUN_SIZE);
	if (NULL == *ppBuildrun)
	{
		return OS_ERR;
	}
	memset(*ppBuildrun, 0, BDN_MAX_BUILDRUN_SIZE);
	
	pBuildrun = *ppBuildrun;

	pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"aaa");

	for (int i = 0; i < AAA_USER_MAX_NUM; i++)
	{
		if (g_stAAAUser[i].used)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"local-user %s password %s",
				g_stAAAUser[i].user_name, g_stAAAUser[i].user_psw);

			if (AAA_MASK_GET(g_stAAAUser[i].service_type, AAA_SERVICE_TYPE_TELNET)
				&& AAA_MASK_GET(g_stAAAUser[i].service_type, AAA_SERVICE_TYPE_FTP))
			{
				pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"local-user %s service-type telnet ftp",
					g_stAAAUser[i].user_name);
			}
			else if (AAA_MASK_GET(g_stAAAUser[i].service_type, AAA_SERVICE_TYPE_TELNET))
			{
					pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"local-user %s service-type telnet",
						g_stAAAUser[i].user_name);
			}
			else if (AAA_MASK_GET(g_stAAAUser[i].service_type, AAA_SERVICE_TYPE_FTP))
			{
					pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"local-user %s service-type ftp",
						g_stAAAUser[i].user_name);
			}
			else
			{

			}
		}
	}

	return OS_OK;
}

int AAA_Init()
{
	return OS_OK;
}

int AAA_TaskEntry(void *pEntry)
{
	ULONG ulRet = OS_OK;

	{
		extern ULONG AAA_RegCmd();
		(VOID)AAA_RegCmd();
	}

	(void)BDN_RegistBuildRun(MID_AAA, VIEW_AAA, BDN_PRIORITY_HIGH + 99, AAA_BuildRun);

	/* 循环读取消息队列 */
	for(;;)
	{
		/* 放权 */
		Sleep(1000);
	}
	
}

APP_INFO_S g_AAAAppInfo =
{
	MID_AAA,
	"AAA",
	AAA_Init,
	AAA_TaskEntry
};

void AAA_RegAppInfo()
{
	APP_RegistInfo(&g_AAAAppInfo);
}

#endif


