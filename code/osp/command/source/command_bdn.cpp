
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>

#ifdef _LINUX_
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
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
#include "osp/command/include/icli.h"

#if (OS_YES == OSP_MODULE_CLI)

typedef struct BDN_EVENT_Ntf_Node
{
	ULONG  moduleId;
	ULONG  view_id;
	ULONG  priority;
	ULONG  (*pfCallBackFunc)(CHAR **ppBuildrun, ULONG ulIncludeDefault);  /* 回调函数内申请内存，由调用者释放 */

	struct BDN_EVENT_Ntf_Node *pNext;
};

struct BDN_EVENT_Ntf_Node *g_pstBDNEventNtfList;

extern VOID vty_printf(VOID *vty, CHAR *format, ...);

/*
moduleId: 模块id
view_id : 归属视图id
*/
ULONG BDN_RegistBuildRun(ULONG moduleId, ULONG view_id, ULONG  priority,
								ULONG  (*pfCallBackFunc)(CHAR **ppBuildrun, ULONG ulIncludeDefault))
{
	struct BDN_EVENT_Ntf_Node * pstNow = NULL;
	struct BDN_EVENT_Ntf_Node * pstPre = NULL;
	struct BDN_EVENT_Ntf_Node * pstEvtNtfNodeNew = NULL;

	pstEvtNtfNodeNew = (struct BDN_EVENT_Ntf_Node *)malloc(sizeof(struct BDN_EVENT_Ntf_Node));
	if (NULL == pstEvtNtfNodeNew)
	{
		return OS_ERR;
	}

	pstEvtNtfNodeNew->moduleId = moduleId;
	pstEvtNtfNodeNew->view_id = view_id;
	pstEvtNtfNodeNew->pfCallBackFunc = pfCallBackFunc;
	pstEvtNtfNodeNew->priority = priority;

	if (NULL == g_pstBDNEventNtfList)
	{
		g_pstBDNEventNtfList = pstEvtNtfNodeNew;
		pstEvtNtfNodeNew->pNext = NULL;
		return OS_OK;
	}

	pstNow = g_pstBDNEventNtfList;
	pstPre = pstNow;

	while (NULL != pstNow)
	{
		if (pfCallBackFunc == pstNow->pfCallBackFunc)
		{
			free(pstEvtNtfNodeNew);
			return OS_OK;
		}
		
		if (priority > pstNow->priority)
		{
			break;
		}

		pstPre = pstNow;
		pstNow = pstNow->pNext;
	}

	if (NULL == pstNow)
	{
		/* INSERT TAIL*/
		pstPre->pNext = pstEvtNtfNodeNew;
		pstEvtNtfNodeNew->pNext = NULL;
	}
	else
	{
		/* INSERT HEAD */
		pstEvtNtfNodeNew->pNext = pstNow;

		if (pstNow == pstPre)
		{
			g_pstBDNEventNtfList = pstEvtNtfNodeNew;
		}
		else
		{
			pstPre->pNext = pstEvtNtfNodeNew;
		}
	}

	return OS_OK;
}

ULONG BDN_SystemBuildRun(CHAR **ppBuildrun, ULONG ulIncludeDefault)
{
	int index  = 0;
	int ret = OS_OK;
	CHAR *pBuildrun = NULL;
	
	*ppBuildrun = (CHAR*)malloc(BDN_MAX_BUILDRUN_SIZE);
	if (NULL == *ppBuildrun)
	{
		return OS_ERR;
	}

	memset(*ppBuildrun,0,BDN_MAX_BUILDRUN_SIZE);
	
	struct BDN_EVENT_Ntf_Node * pstHead =  g_pstBDNEventNtfList;

	while (NULL != pstHead)
	{
		
		pBuildrun = NULL;
		ret = pstHead->pfCallBackFunc(&pBuildrun, ulIncludeDefault);
		if (OS_OK != ret)
		{
		}

		if (NULL != pBuildrun)
		{			
			if (0 != strlen(pBuildrun))
			{
				strcat(*ppBuildrun, pBuildrun);
				strcat(*ppBuildrun, "\r\n#");
			}
			
			free(pBuildrun);
			pBuildrun = NULL;
		}

		pstHead = pstHead->pNext;
	}

	strcat(*ppBuildrun, "\r\nreturn");
	strcat(*ppBuildrun, "\r\n#");
			
	return OS_OK;
}


VOID BDN_ShowBuildRun(ULONG vtyId, ULONG ulIncludeDefault)
{
	ULONG ulRet = OS_OK;
	CHAR *pBuildrun = NULL;

	ulRet = BDN_SystemBuildRun(&pBuildrun, ulIncludeDefault);
	if (OS_OK != ulRet)
	{
		return;
	}
	
	vty_printf(vtyId, "%s\r\n", pBuildrun);

	free(pBuildrun);
}

VOID BDN_ShowCurrentViewBuildRun(ULONG vtyId, ULONG ulIncludeDefault)
{
	int index  = 0;
	int ret = OS_OK;	
	CHAR *pBuildrun = NULL;
	CHAR *pBuildrunTmp = NULL;
	ULONG view_id = VIEW_NULL;
	
	view_id = vty_get_current_viewid(vtyId);
		
	pBuildrun = (CHAR*)malloc(BDN_MAX_BUILDRUN_SIZE);
	if (NULL == pBuildrun)
	{
		return;
	}

	memset(pBuildrun,0,BDN_MAX_BUILDRUN_SIZE);
	
	struct BDN_EVENT_Ntf_Node * pstHead =  g_pstBDNEventNtfList;

	strcat(pBuildrun, "#");
	
	while (NULL != pstHead)
	{
		if (pstHead->view_id != view_id)
		{
			pstHead = pstHead->pNext;
			continue;
		}
		
		pBuildrunTmp = NULL;
		ret = pstHead->pfCallBackFunc(&pBuildrunTmp, ulIncludeDefault);
		if (OS_OK != ret)
		{
			
		}

		if (NULL != pBuildrunTmp)
		{	
			if (0 != strlen(pBuildrunTmp))
			{
				strcat(pBuildrun, pBuildrunTmp);
				strcat(pBuildrun, "\r\n#");
			}
			
			free(pBuildrunTmp);
			pBuildrunTmp = NULL;
		}

		pstHead = pstHead->pNext;	
	}

	strcat(pBuildrun, "\r\nreturn");
	strcat(pBuildrun, "\r\n#");

	vty_printf(vtyId, "%s\r\n", pBuildrun);

	free(pBuildrun);
}

#endif

