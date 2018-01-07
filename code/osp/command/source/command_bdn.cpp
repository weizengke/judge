
#include <windows.h>
#include <process.h>
#include <iostream>
#include <conio.h>
#include <stdlib.h>
#include <io.h>
#include <time.h>
#include <queue>
#include <string>
#include <sstream>

#include "tlhelp32.h"

#include "osp\common\include\osp_common_def.h"
#include "osp\command\include\command_inc.h"

typedef struct BDN_EVENT_Ntf_Node
{
	ULONG  moduleId;
	ULONG  priority;
	ULONG  (*pfCallBackFunc)(CHAR **ppBuildrun);  /* 回调函数内申请内存，由调用者释放 */

	struct BDN_EVENT_Ntf_Node *pNext;
};

struct BDN_EVENT_Ntf_Node *g_pstBDNEventNtfList;


ULONG BDN_RegistBuildRun(ULONG moduleId, ULONG  priority,
								ULONG  (*pfCallBackFunc)(CHAR **ppBuildrun))
{
	struct BDN_EVENT_Ntf_Node * pstNow = NULL;
	struct BDN_EVENT_Ntf_Node * pstPre = NULL;
	struct BDN_EVENT_Ntf_Node * pstEvtNtfNodeNew = NULL;

	pstEvtNtfNodeNew = (struct BDN_EVENT_Ntf_Node *)malloc(sizeof(struct BDN_EVENT_Ntf_Node));
	if (NULL == pstEvtNtfNodeNew)
	{
		return OS_ERR;
	}

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

ULONG BDN_SystemBuildRun(CHAR **ppBuildrun)
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
		ret = pstHead->pfCallBackFunc(&pBuildrun);
		if (OS_OK != ret)
		{
		}

		if (NULL != pBuildrun)
		{			
			strcat(*ppBuildrun, pBuildrun);
			strcat(*ppBuildrun, "\r\n#");
			
			free(pBuildrun);
			pBuildrun = NULL;
		}

		pstHead = pstHead->pNext;
	}

	strcat(*ppBuildrun, "\r\nreturn");
	strcat(*ppBuildrun, "\r\n#");
			
	return OS_OK;
}


void BDN_ShowBuildRun(CMD_VTY *vty)
{
	ULONG ulRet = OS_OK;
	CHAR *pBuildrun = NULL;

	ulRet = BDN_SystemBuildRun(&pBuildrun);
	if (OS_OK != ulRet)
	{
		return;
	}
	
	vty_printf(vty, "%s\r\n", pBuildrun);

	free(pBuildrun);
}




