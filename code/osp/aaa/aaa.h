#ifndef _AAA_H_
#define _AAA_H_

#include "osp\event\include\event_pub.h"

#define AAA_USER_MAX_NUM 16 

enum AAA_SERVICE_TYPE_E {
	AAA_SERVICE_TYPE_NONE,
	AAA_SERVICE_TYPE_TELNET,
	AAA_SERVICE_TYPE_FTP,
	
	AAA_SERVICE_TYPE_MAX,
};

#define AAA_MASK_GET(a, x)    (( a >> x) & 1)
#define AAA_MASK_SET(a, x)    ( a |= ( 1 << x ) )
#define AAA_MASK_CLEAR(a, x)  ( a &= ~( 1 << x) )

typedef struct tagAAA_USER_S {
	ULONG used;
	ULONG level;
	ULONG service_type; /* service type bit map */
	char user_name[32];
	char user_psw[32];
}AAA_USER_S;

enum AAA_EVT_E {
	AAA_EVT_NONE,
	AAA_EVT_USER_ADD,
	AAA_EVT_USER_DEL,
	AAA_EVT_USER_PSW_CHANGE,
	AAA_EVT_USER_SERVICE_TELNET_DEL,
	AAA_EVT_USER_SERVICE_FTP_DEL,
	AAA_EVT_MAX,
};

extern ULONG AAA_EvtRegistFunc(CHAR *pModuleName, ULONG eventId, ULONG priority, EVENT_CB_FUNC pfCallBackFunc);
extern ULONG AAA_AddUser(ULONG vtyId, char *uname, char *psw, ULONG ulServiceType);
extern ULONG AAA_DelUser(ULONG vtyId, char *uname, ULONG ulServiceType);
extern ULONG AAA_UserAuth(char *uname, char *psw, ULONG type);

#endif


