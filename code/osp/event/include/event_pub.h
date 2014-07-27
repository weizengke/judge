#ifndef _EVENT_PUB_H_
#define _EVENT_PUB_H_

typedef struct EVENT_Ntf_Node
{
	char moduleName[256];
	int  eventId;
	int  cmdId;
	int  priority;
	int  (*pfCallBackFunc)(int evtId, int cmdId, void *pData, void **ppInfo);

	struct EVENT_Ntf_Node *pNext;
};


enum EVENT_NTF_E
{
	EVENT_NTF_NONE,
	EVENT_NTF_JUDGE,
	EVENT_NTF_SQL,
	EVENT_NTF_CMD,

	EVENT_NTF_MAX
};

enum EVENT_NTF_CMD_E
{
	EVENT_NTF_CMD_NONE,

	EVENT_NTF_CMD_MAX
};


extern int EVENT_RegistFunc(char *pModuleName, int  eventId, int cmdId, int  priority,
								int  (*pfCallBackFunc)(int evtId, int cmdId, void *pData, void **ppInfo));
extern int EVENT_Ntf_Notify(int evtId, int cmdId, void *pData, void **ppInfo);

extern void EVENT_Ntf_Show();


#endif
