
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
#include "osp\event\include\event_pub.h"

struct EVENT_Ntf_Node *g_pstEventNtfList[EVENT_NTF_MAX];

/*****************************************************************************
*   Prototype    : EVENT_RegistFunc
*   Description  : 提供事件回调注册函数
*   Input        : char *pModuleName
*                  int  eventId
				   int  cmdId
*                  int  priority
*                  int  (*pfCallBackFunc)(int evtId
*                  int cmdId
*                  void *pData
*                  void **ppInfo)
*   Output       : None
*   Return Value : int
*   Calls        :
*   Called By    :
*
*   History:
*
*       1.  Date         : 2014/7/27
*           Author       : weizengke
*           Modification : Created function
*
*****************************************************************************/
int EVENT_RegistFunc(char *pModuleName, int  eventId, int cmdId, int  priority,
								int  (*pfCallBackFunc)(int evtId, int cmdId, void *pData, void **ppInfo))
{
	struct EVENT_Ntf_Node * pstNow = NULL;
	struct EVENT_Ntf_Node * pstPre = NULL;
	struct EVENT_Ntf_Node * pstEvtNtfNodeNew = NULL;

	pstEvtNtfNodeNew = (struct EVENT_Ntf_Node *)malloc(sizeof(struct EVENT_Ntf_Node));
	if (NULL == pstEvtNtfNodeNew)
	{
		return OS_ERR;
	}

	pstEvtNtfNodeNew->pfCallBackFunc = pfCallBackFunc;
	pstEvtNtfNodeNew->priority = priority;
	pstEvtNtfNodeNew->cmdId    = cmdId;
	pstEvtNtfNodeNew->eventId = eventId;
	strcpy(pstEvtNtfNodeNew->moduleName, pModuleName);

	if (NULL == g_pstEventNtfList[eventId])
	{
		g_pstEventNtfList[eventId] = pstEvtNtfNodeNew;
		pstEvtNtfNodeNew->pNext = NULL;
		pdt_debug_print("EVENT_RegistFunc [%s] ok...", pModuleName);
		return OS_OK;
	}

	pstNow = g_pstEventNtfList[eventId];
	pstPre = pstNow;

	while (NULL != pstNow)
	{
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
			g_pstEventNtfList[eventId] = pstEvtNtfNodeNew;
		}
		else
		{
			pstPre->pNext = pstEvtNtfNodeNew;
		}

	}

	pdt_debug_print("End EVENT_RegistFunc %s ok...", pModuleName);

	return OS_OK;
}

/*****************************************************************************
*   Prototype    : EVENT_Ntf_Show
*   Description  : Show All CallBack active
*   Input        : None
*   Output       : None
*   Return Value : void
*   Calls        :
*   Called By    :
*
*   History:
*
*       1.  Date         : 2014/7/27
*           Author       : weizengke
*           Modification : Created function
*
*****************************************************************************/
void EVENT_Ntf_Show()
{
	int index  = 0;
	for (index = 0; index < EVENT_NTF_MAX; index++)
	{
		struct EVENT_Ntf_Node * pstHead =  g_pstEventNtfList[index];
		while (NULL != pstHead)
		{
			pdt_debug_print("Mod:%s, Evt:%d, Cmd:%d, fpCBFunc:0x%x, pri:%d",
					pstHead->moduleName, pstHead->eventId, pstHead->cmdId, pstHead->pfCallBackFunc, pstHead->priority);

			pstHead = pstHead->pNext;
		}
	}
}

/*****************************************************************************
*   Prototype    : EVENT_Ntf_Notify
*   Description  : 事件通知函数
*   Input        : int evtId
*                  int cmdId
*                  void *pData
*                  void **ppInfo
*   Output       : None
*   Return Value : int
*   Calls        :
*   Called By    :
*
*   History:
*
*       1.  Date         : 2014/7/27
*           Author       : weizengke
*           Modification : Created function
*
*****************************************************************************/
int EVENT_Ntf_Notify(int evtId, int cmdId, void *pData, void **ppInfo)
{
	int index  = 0;
	int ret = OS_OK;

	if (evtId >= EVENT_NTF_MAX)
	{
		return OS_ERR;
	}

	struct EVENT_Ntf_Node * pstHead =  g_pstEventNtfList[evtId];

	while (NULL != pstHead)
	{
		if (EVENT_NTF_CMD_NONE != pstHead->cmdId)
		{
			if (cmdId != pstHead->cmdId)
			{
				pstHead = pstHead->pNext;
				continue;
			}
		}

		pdt_debug_print("EVENT_Ntf_Notify [%s]...", pstHead->moduleName);

		ret = pstHead->pfCallBackFunc(evtId, cmdId, pData, ppInfo);
		if (OS_OK != ret)
		{
			pdt_debug_print("EVENT_Ntf_Notify [%s] failed. (ret=%d)", pstHead->moduleName, ret);
		}

		pstHead = pstHead->pNext;
	}

}

