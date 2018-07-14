
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
#include "osp\debug\include\debug_center_inc.h"

#define EVENT_Debug(x, args...) debugcenter_print(MID_EVENT, x, args)
/*****************************************************************************
 Prototype    : EVENT_InitTbl
 Description  : 初始化事件table
 Input        : EVENT_NTF_NODE **ppstEvtTbl  
                ULONG ulSize                 
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2018/7/8
    Author       : Jungle
    Modification : Created function

*****************************************************************************/
ULONG EVENT_InitTbl(EVENT_NTF_NODE **ppstEvtTbl, ULONG ulSize)
{
	ULONG ulRet = OS_OK;
	EVENT_NTF_NODE *pstEvtTbl = NULL;

	if (NULL == ppstEvtTbl)
	{
		return OS_ERR;
	}
	
	pstEvtTbl = (EVENT_NTF_NODE *)malloc(ulSize * sizeof(EVENT_NTF_NODE));
	if (NULL == pstEvtTbl)
	{
		return OS_ERR;
	}
	memset(pstEvtTbl, 0, ulSize * sizeof(EVENT_NTF_NODE));

	*ppstEvtTbl = pstEvtTbl;
		
	return ulRet;
}

/*****************************************************************************
*   Prototype    : EVENT_RegistFunc
*   Description  : 提供事件回调注册函数
*   Input        : 
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
ULONG EVENT_RegistFunc(EVENT_NTF_NODE *pstEventTbl,
						CHAR *pModuleName, 
						ULONG eventId,
						ULONG priority,
						EVENT_CB_FUNC pfCallBackFunc)
{
	EVENT_NTF_NODE * pstTblHead = NULL;
	EVENT_NTF_NODE * pstNow = NULL;
	EVENT_NTF_NODE * pstPre = NULL;
	EVENT_NTF_NODE * pstEvtNtfNodeNew = NULL;

	if (NULL == pstEventTbl)
	{
		return OS_ERR;
	}

	pstTblHead = pstEventTbl + eventId;
		
	pstEvtNtfNodeNew = (EVENT_NTF_NODE *)malloc(sizeof(EVENT_NTF_NODE));
	if (NULL == pstEvtNtfNodeNew)
	{
		return OS_ERR;
	}

	pstEvtNtfNodeNew->pfCallBackFunc = pfCallBackFunc;
	pstEvtNtfNodeNew->priority = priority;
	pstEvtNtfNodeNew->eventId = eventId;
	strcpy(pstEvtNtfNodeNew->moduleName, pModuleName);

	pstNow = pstTblHead->pNext;
	pstPre = pstTblHead;

	if (NULL == pstNow)
	{
		pstPre->pNext = pstEvtNtfNodeNew;
		pstEvtNtfNodeNew->pNext = NULL;
		return OS_OK;
	}

	while (NULL != pstNow)
	{
		if (priority > pstNow->priority)
		{
			break;
		}

		pstPre = pstNow;
		pstNow = pstNow->pNext;
	}

	/* INSERT TAIL*/
	if (NULL == pstNow)
	{
		pstPre->pNext = pstEvtNtfNodeNew;
		pstEvtNtfNodeNew->pNext = NULL;
	}
	else
	{
		/* INSERT HEAD */
		pstEvtNtfNodeNew->pNext = pstNow;
		pstNow->pNext = pstEvtNtfNodeNew;
	}

#if 0
	pstTblHead = pstEventTbl + eventId;
	while (NULL != pstTblHead)
	{
		printf("\r\n Module:%s eventId=%u, priority=%u", pstTblHead->moduleName, pstTblHead->eventId, pstTblHead->priority);
		pstTblHead = pstTblHead->pNext;
	}
#endif

	return OS_OK;
}

/*****************************************************************************
*   Prototype    : EVENT_Notify
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
ULONG EVENT_Notify(EVENT_NTF_NODE *pstEventTbl, ULONG evtId, ULONG keyId, ULONG cmdId, VOID *pData, VOID **ppInfo)
{
	ULONG index  = 0;
	ULONG ret = OS_OK;
	EVENT_NTF_NODE * pstHead =  NULL;

	if (NULL == pstEventTbl)
	{
		return OS_ERR;
	}

	pstHead =  pstEventTbl + evtId;
	pstHead = pstHead->pNext;

	EVENT_Debug(DEBUG_TYPE_INFO, "EVENT_Notify [%s]...", pstHead->moduleName);
	
	while (NULL != pstHead)
	{
		EVENT_Debug(DEBUG_TYPE_INFO, "EVENT_Notify [%s]...", pstHead->moduleName);

		ret = pstHead->pfCallBackFunc(keyId, cmdId, pData, ppInfo);
		if (OS_OK != ret)
		{
			EVENT_Debug(DEBUG_TYPE_ERROR, "EVENT_Notify [%s] failed. (ret=%d)", pstHead->moduleName, ret);

			/* 遇错即返回 */
			return ret;
		}

		pstHead = pstHead->pNext;
	}

	return ret;
}

