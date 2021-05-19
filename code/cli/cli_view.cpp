
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
#include "cli_type.h"
#include "cli_view.h"

/*
视图能力
user(>)
  -system(])
     - diagnose(diagnose)
*/

#define CMD_VIEW_SONS_NUM 128

typedef struct cmd_view_node_st {
	ULONG id;
	CHAR name[CMD_MAX_VIEW_LEN];
    CHAR alias[CMD_MAX_VIEW_LEN];
	struct cmd_view_node_st *parent;
	struct cmd_view_node_st **sons;
	ULONG sons_num;
}CMD_VIEW_NODE_S;

CMD_VIEW_NODE_S *g_view_tree = NULL; /* view tree */

CMD_VIEW_NODE_S *cmd_view_get(CMD_VIEW_NODE_S *view, ULONG view_id)
{
	CMD_VIEW_NODE_S *view_ = NULL;

	if (NULL == view) {
		return view;
	}

	if (view->id == view_id) {
		return view;
	}

	for (int i = 0; i < view->sons_num; i++) {
		view_ = cmd_view_get(view->sons[i], view_id);
		if (NULL != view_) {
			return view_;
		}
	}

	return NULL;
}
ULONG cmd_view_get_alias_lenth(ULONG view_id)
{
	CMD_VIEW_NODE_S *view = cmd_view_get(g_view_tree, view_id);
	if (view == NULL) {
		return 0;
	}

	return strlen(view->alias);
}

CHAR *cmd_view_get_alias(ULONG view_id)
{
	CMD_VIEW_NODE_S *view = cmd_view_get(g_view_tree, view_id);
	if (view == NULL) {
		return NULL;
	}

	return view->alias;
}

CHAR *cmd_view_get_name(ULONG view_id)
{
	CMD_VIEW_NODE_S *view = cmd_view_get(g_view_tree, view_id);
	if (view == NULL) {
		return NULL;
	}

	return view->name;
}

ULONG cmd_view_get_parent_view_id(ULONG view_id)
{
	CMD_VIEW_NODE_S *view = NULL;
	CMD_VIEW_NODE_S *prev_view = NULL;

	view = cmd_view_get(g_view_tree, view_id);
	if (view == NULL) {
		return VIEW_NULL;
	}

	prev_view = view->parent;
	if (prev_view == NULL) {
		return VIEW_NULL;
	}

    return prev_view->id;
}

VOID vty_view_set(ULONG vtyId, ULONG view_id)
{
	CMD_VIEW_NODE_S *view = NULL;
	CMD_VTY_S *vty = NULL;

	vty = cmd_vty_getById(vtyId);
	if (vty == NULL) {
		return;
	}

	view = cmd_view_get(g_view_tree, view_id);
	if (view == NULL) {
		return;
	}

	if (view_id < VIEW_USER) {
		/* telnet vty offline */
		if (CMD_VTY_CONSOLE_ID != vty->vtyId) {
			vty_offline(vtyId);
		}
		return;
	}

	vty->view_id = view_id;
}

VOID vty_view_quit(ULONG vtyId)
{
	CMD_VIEW_NODE_S *view = NULL;
	CMD_VIEW_NODE_S *prev_view = NULL;
	CMD_VTY_S *vty = NULL;

	vty = cmd_vty_getById(vtyId);
	if (vty == NULL) {
		return;
	}

	view = cmd_view_get(g_view_tree, vty->view_id);
	if (view == NULL) {
		vty_offline(vtyId);
		return;
	}

	prev_view = view->parent;
	if (prev_view == NULL) {
		vty_offline(vtyId);
		return;
	}

	vty_view_set(vtyId, prev_view->id);
}

ULONG vty_get_current_viewid(ULONG vtyId)
{
	CMD_VTY_S *vty = cmd_vty_getById(vtyId);
	if (NULL == vty) {
		return VIEW_NULL;
	}

	return vty->view_id;
}

ULONG cmd_view_regist(CHAR *view_name, CHAR *view_ais, ULONG view_id, ULONG parent_view_id)
{
	CMD_VIEW_NODE_S *view = NULL;
	CMD_VIEW_NODE_S *pSons = NULL;
	CMD_VIEW_NODE_S *pParent = NULL;

	if (VIEW_NULL != parent_view_id) {
		pParent = cmd_view_get(g_view_tree, parent_view_id);
		if (NULL == pParent) {
			CMD_DBGASSERT(0, "cmd_view_regist, no parent view %u.", parent_view_id);
			return CMD_ERR;
		}
	}

	view = (CMD_VIEW_NODE_S *)malloc(sizeof(CMD_VIEW_NODE_S));
	if (NULL == view) {
		CMD_DBGASSERT(0, "cmd_view_regist, malloc failed.");
		return CMD_ERR;
	}
	memset(view, 0, sizeof(CMD_VIEW_NODE_S));
	view->id = view_id;
	strcpy(view->name, view_name);
	strcpy(view->alias, view_ais);

	view->sons = (CMD_VIEW_NODE_S **)malloc(CMD_VIEW_SONS_NUM * sizeof(CMD_VIEW_NODE_S));
	if (NULL == view->sons) {
		CMD_DBGASSERT(0, "cmd_view_regist sons malloc failed");
		free(view);
		return CMD_ERR;
	}
	memset(view->sons, 0, CMD_VIEW_SONS_NUM * sizeof(CMD_VIEW_NODE_S));

	view->sons_num = 0;
	view->parent = pParent;

	if (NULL != pParent) {
		if (pParent->sons_num >= CMD_VIEW_SONS_NUM) {
			CMD_DBGASSERT(0, "cmd_view_regist sons num more than 100");
			free(view->sons);
			free(view);
			return CMD_ERR;
		}

		pParent->sons[pParent->sons_num++] = view;
	} else {
		g_view_tree = view;
	}

	return CMD_OK;
}

/* regist default view*/
VOID cmd_view_init()
{
	(void)cmd_view_regist("global", "", VIEW_GLOBAL, VIEW_NULL);
	(void)cmd_view_regist("user-view", "", VIEW_USER, VIEW_GLOBAL);
	(void)cmd_view_regist("system-view", "", VIEW_SYSTEM, VIEW_USER);
	(void)cmd_view_regist("diagnose-view", "diagnose", VIEW_DIAGNOSE, VIEW_SYSTEM);

	return;
}
