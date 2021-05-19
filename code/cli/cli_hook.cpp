#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef WIN32

#if CYGWIN
#include <ncurses.h>
#else
#include <conio.h>
#endif

#include <io.h>
#include <winsock2.h>
#endif

#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#ifdef _LINUX_
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <termios.h>
#include <assert.h>
#endif

#include "kernel.h"
#include "util/util.h"

#include "icli.h"
#include "cli_def.h"
#include "cli_type.h"
#include "cli_core.h"
#include "cli_reg.h"
#include "cli_hook.h"

CHAR g_CmdSysname[CMD_SYSNAME_SIZE] = "system";
CMDHOOK_S g_cmd_hook = {0};	 /* hook function */

CHAR *cmd_get_sysname()
{
	if (NULL == g_cmd_hook.pfGetSysname) {
		return g_CmdSysname;
	}

	return g_cmd_hook.pfGetSysname();
}

VOID cmd_page_up() 
{
	if (NULL != g_cmd_hook.pfKeyPageUp) {
		g_cmd_hook.pfKeyPageUp();
	}
}

VOID cmd_page_down() 
{
	if (NULL != g_cmd_hook.pfKeyPageDown) {
		g_cmd_hook.pfKeyPageDown();
	}
}

int cmd_socket_send(ULONG s, char *buf, int len, int flags) 
{
	if (NULL != g_cmd_hook.pfSocketsend) {
		return g_cmd_hook.pfSocketsend(s, buf, len, flags);
	}
	return send(s, buf, len, flags);
}

int cmd_socket_recv(ULONG s, char *buf, int len, int flags) 
{
	if (NULL != g_cmd_hook.pfSocketrecv) {
		return g_cmd_hook.pfSocketrecv(s, buf, len, flags);
	}
	return recv(s, buf, len, flags);
}

int cmd_socket_close(ULONG s) 
{
	if (NULL != g_cmd_hook.pfSocketclose) {
		return g_cmd_hook.pfSocketclose(s);
	}
	return closesocket(s);
}

VOID CMD_HOOK_RegCallback_PageUp(VOID (*pfKeyPageUp)())
{
	g_cmd_hook.pfKeyPageUp = pfKeyPageUp;
}

VOID CMD_HOOK_RegCallback_PageDown(VOID (*pfKeyPageDown)())
{
	g_cmd_hook.pfKeyPageDown = pfKeyPageDown;
}

VOID CMD_HOOK_RegCallback_GetSysname(CHAR *(*pfGetSysname)())
{
	g_cmd_hook.pfGetSysname = pfGetSysname;
}

VOID CMD_HOOK_RegCallback_socketsend(int (*pfSocketsend)(ULONG s, char *buf, int len, int flags))
{
	g_cmd_hook.pfSocketsend = pfSocketsend;
}
VOID CMD_HOOK_RegCallback_Socketrecv(int (*pfSocketrecv)(ULONG s, char *buf, int len, int flags))
{
	g_cmd_hook.pfSocketrecv = pfSocketrecv;
}
VOID CMD_HOOK_RegCallback_Socketclose(int (*pfSocketclose)(ULONG s))
{
	g_cmd_hook.pfSocketclose = pfSocketclose;
}
