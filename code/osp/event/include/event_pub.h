#ifndef _EVENT_PUB_H_
#define _EVENT_PUB_H_

typedef ULONG (*EVENT_CB_FUNC)(ULONG keyId, ULONG cmdId, VOID *pData, VOID **ppInfo);

typedef struct tagEVENT_Ntf_Node
{
	CHAR moduleName[256];
	ULONG  eventId;
	ULONG  priority;
	EVENT_CB_FUNC pfCallBackFunc;

	struct tagEVENT_Ntf_Node *pNext;
}EVENT_NTF_NODE;

extern ULONG EVENT_InitTbl(EVENT_NTF_NODE **ppstEvtTbl, ULONG ulSize);
extern ULONG EVENT_RegistFunc(EVENT_NTF_NODE *pstEventTbl,
						CHAR *pModuleName, 
						ULONG eventId,
						ULONG priority,
						EVENT_CB_FUNC pfCallBackFunc);
extern ULONG EVENT_Notify(EVENT_NTF_NODE *pstEventTbl, ULONG keyId, ULONG evtId, ULONG cmdId, VOID *pData, VOID **ppInfo);

#endif
