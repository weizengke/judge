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

#include "icli.h"
#include "cli_def.h"
#include "cli_util.h"
#include "cli_type.h"
#include "cli_vty.h"
#include "cli_vty_io.h"
#include "cli_core.h"
#include "cli_view.h"
#include "cli_history.h"
#include "cli_reg.h"
#include "cli_hook.h"

#define CMD_CON_NAME "CON"
#define CMD_CFM_NAME "CFM"
CMD_VTY_S *g_telnet_vty[CMD_VTY_MAXUSER_NUM];  /* telnet vty list */
CMD_VTY_S *g_con_vty;     /* console vty */
CMD_VTY_S *g_cfm_vty;     /* cfm vty */

VOID cmd_vty_init(CMD_VTY_S *vty)
{
	vty->vtyId = vty->vtyId;
	vty->used = 0;
	vty->view_id = VIEW_USER;
	vty->ulBufMaxLen = CMD_BUFFER_SIZE;
	vty->ulUsedLen = vty->ulCurrentPos = 0;
	vty->ulhpos = vty->ulhNum = 0;

	vty->ucKeyTypePre = CMD_KEY_CODE_INSERT;
	vty->ucKeyTypeNow = CMD_KEY_CODE_INSERT;
	memset(vty->tabbingString, 0, CMD_MAX_CMD_ELEM_SIZE);
	memset(vty->tabString, 0, CMD_MAX_CMD_ELEM_SIZE);

	vty->user.socket = INVALID_SOCKET;
	vty->user.state = CMD_VTY_STATE_IDLE;
	//vty->user.type = CMD_VTY_TYPE_CON;
	vty->user.terminal_debugging = 0;
	memset(vty->user.user_name, 0, sizeof(vty->user.user_name));
	//vty->user.lastAccessTime = time(NULL);

	return;
}

CMD_VTY_S *cmd_vty_new(ULONG vtyId, CHAR *name, ULONG type)
{
	CMD_VTY_S *vty = (CMD_VTY_S *)malloc(sizeof(CMD_VTY_S));
	if (vty == NULL) {
		return NULL;
	}
	memset(vty, 0, sizeof(CMD_VTY_S));

	vty->vtyId = vtyId;
	vty->user.type = type;
	if (name) {
		strcpy(vty->user.user_name, name);
	}	

	cmd_vty_init(vty);
	
	return vty;
}

VOID cmd_vty_telnet_init() 
{	
	for (int i = 0; i < CMD_VTY_MAXUSER_NUM; i++) {
		g_telnet_vty[i] = cmd_vty_new(i, NULL, CMD_VTY_TYPE_TELNET);
	}	
}

VOID cmd_vty_console_init()
{
	g_con_vty = cmd_vty_new(CMD_VTY_CONSOLE_ID, CMD_CON_NAME, CMD_VTY_TYPE_CON);
	g_con_vty->used = 1;
	vty_set_state(CMD_VTY_CONSOLE_ID, CMD_VTY_STATE_ACCESS);
}

VOID cmd_vty_cfm_init()
{
	g_cfm_vty = cmd_vty_new(CMD_VTY_CFM_ID, CMD_CFM_NAME, CMD_VTY_TYPE_CON);
	g_cfm_vty->used = 1;
	vty_set_state(CMD_VTY_CFM_ID, CMD_VTY_STATE_ACCESS);
}

/* vty */
CMD_VTY_S *cmd_vty_getById(ULONG vtyId)
{
	if (CMD_VTY_CONSOLE_ID == vtyId) {
		return g_con_vty;
	} else if (CMD_VTY_CFM_ID == vtyId) {
		return g_cfm_vty;
	} else {
		if (vtyId >= CMD_VTY_MAXUSER_NUM) {
			return NULL;
		}
		return g_telnet_vty[vtyId];
	}
}

CMD_VTY_S *cmd_get_idle_vty()
{
    /* only get telnet vty */
	for (ULONG i = 0; i < CMD_VTY_MAXUSER_NUM; i++) {
        CMD_VTY_S *vty = cmd_vty_getById(i);
		if (vty != NULL && vty->used == 0) {
			vty->used = 1;
			return vty;
		}
	}

	return NULL;
}

ULONG cmd_vty_is_used(ULONG vtyId)
{
	CMD_VTY_S *vty = cmd_vty_getById(vtyId);
    if (vty == NULL) {
        return 0;
    }

	return vty->used;
}

time_t vty_get_last_accesstime(ULONG vtyId)
{
	CMD_VTY_S *vty = cmd_vty_getById(vtyId);
	if (NULL == vty) {
		return 0;
	}

	return vty->user.lastAccessTime;
}

ULONG vty_get_socket(ULONG vtyId)
{
	CMD_VTY_S *vty = cmd_vty_getById(vtyId);
	if (NULL == vty) {
		return INVALID_SOCKET;
	}

	return vty->user.socket;
}

ULONG vty_set_socket(ULONG vtyId, ULONG socket)
{
	CMD_VTY_S *vty = cmd_vty_getById(vtyId);
	if (NULL == vty) {
		return CMD_ERR;
	}

	vty->user.socket = socket;

	return CMD_OK;
}

ULONG vty_set_state(ULONG vtyId, ULONG state)
{
	CMD_VTY_S *vty = cmd_vty_getById(vtyId);
	if (NULL == vty) {
		return CMD_ERR;
	}

	vty->user.state = state;

	return CMD_OK;
}

ULONG vty_get_state(ULONG vtyId)
{
	CMD_VTY_S *vty = cmd_vty_getById(vtyId);
	if (NULL == vty) {
		return 0;
	}

	return vty->user.state;
}

VOID vty_offline(ULONG vtyId)
{
	CMD_VTY_S *vty = cmd_vty_getById(vtyId);
	if (NULL == vty) {
		return;
	}

	if (INVALID_SOCKET != vty->user.socket) {
		if (cmd_socket_close(vty->user.socket) == -1) {
		}
		vty->user.socket = INVALID_SOCKET;
	}

    cmd_vty_free_histories(vty);

	cmd_vty_init(vty);
}

VOID vty_offline_by_username(CHAR *pszName)
{
	if (NULL == pszName){
		return;
	}

	for (int i = 0; i < CMD_VTY_MAXUSER_NUM; i++){
        CMD_VTY_S *vty = cmd_vty_getById(i);
		if (vty != NULL && vty->used && vty->user.state) {
			if (0 == strcmp(pszName, vty->user.user_name)) {
				vty_offline(vty->vtyId);
			}
		}
	}
}

VOID vty_offile_all()
{
	int i;

	for (i == 0; i < CMD_VTY_MAXUSER_NUM; i++){
		vty_offline(i);
	}
}

VOID vty_go(ULONG vtyId)
{
	CMD_VTY_S *vty = cmd_vty_getById(vtyId);
	if (NULL == vty) {
		return;
	}

	cmd_fsm(vty);

	return;
}

