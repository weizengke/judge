#ifndef __CLI_HOOK_H__
#define __CLI_HOOK_H__

typedef struct tagCMDHook_S
{
	VOID (*pfKeyPageUp)();
	VOID (*pfKeyPageDown)();
	CHAR* (*pfGetSysname)();
	int (*pfSocketsend)(ULONG s, char *buf, int len, int flags);
	int (*pfSocketrecv)(ULONG s, char *buf, int len, int flag);
	int (*pfSocketclose)(ULONG s);
}CMDHOOK_S;

extern CHAR *cmd_get_sysname();
extern VOID cmd_page_up();
extern VOID cmd_page_down();
extern int cmd_socket_send(ULONG s, char *buf, int len, int flags);
extern int cmd_socket_recv(ULONG s, char *buf, int len, int flags);
extern int cmd_socket_close(ULONG s);
extern VOID CMD_HOOK_RegCallback_PageUp(VOID (*pfKeyPageUp)());
extern VOID CMD_HOOK_RegCallback_PageDown(VOID (*pfKeyPageDown)());
extern VOID CMD_HOOK_RegCallback_GetSysname(CHAR *(*pfGetSysname)());
extern VOID CMD_HOOK_RegCallback_socketsend(int (*pfSocketsend)(ULONG s, char *buf, int len, int flags));
extern VOID CMD_HOOK_RegCallback_Socketrecv(int (*pfSocketrecv)(ULONG s, char *buf, int len, int flags));
extern VOID CMD_HOOK_RegCallback_Socketclose(int (*pfSocketclose)(ULONG s));

#endif