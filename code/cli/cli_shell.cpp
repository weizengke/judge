

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
#include "cli_util.h"
#include "cli_def.h"
#include "cli_type.h"
#include "cli_vty.h"
#include "cli_vty_io.h"
#include "cli_shell.h"

#include "pdt_common_inc.h"
#include "ic/include/debug_center_inc.h"

#define CMD_debug(x, format, ...) debugcenter_print(MID_CMD, x, format, ##__VA_ARGS__)

CMD_NTF_NODE_S *g_cmd_reg_callback = NULL;	/* command process callback*/

ULONG cmd_get_vty_id(VOID *pRunMsg)
{
	CMD_RUNMSG_S *pstRunMsg = (CMD_RUNMSG_S *)pRunMsg;

	if (NULL == pstRunMsg) {
		return CMD_VTY_INVALID_ID;
	}

	return pstRunMsg->vtyId;
}

VOID *cmd_get_elem_by_index(VOID *pRunMsg, ULONG index)
{
	CMD_RUNMSG_S *pstRunMsg = (CMD_RUNMSG_S *)pRunMsg;

	if (NULL == pstRunMsg) {
		return NULL;
	}

	return (void *)(pstRunMsg->argv + index);
}

ULONG cmd_get_elem_num(VOID *pRunMsg)
{
	CMD_RUNMSG_S *pstRunMsg = (CMD_RUNMSG_S *)pRunMsg;

	if (NULL == pstRunMsg) {
		return 0;
	}

	return pstRunMsg->argc;
}

ULONG cmd_get_first_elem_tblid(VOID *pRunMsg)
{
	CMD_RUNMSG_S *pstRunMsg = (CMD_RUNMSG_S *)pRunMsg;
	CMD_RUNMSG_ELEM_S *pElem = NULL;

	if (NULL == pstRunMsg) {
		return CMD_ELEMID_NULL;
	}

	pElem = pstRunMsg->argv;

	return ((0x00FF0000 & pElem->ulElmtId) >> 16);
}

ULONG cmd_get_elemid(VOID *pElemMsg)
{
	CMD_RUNMSG_ELEM_S *pElem = (CMD_RUNMSG_ELEM_S *)pElemMsg;

	if (NULL == pElem) {
		return CMD_ELEMID_NULL;
	}

	return pElem->ulElmtId;
}

CHAR *cmd_get_elem_param(VOID *pElemMsg)
{
	CMD_RUNMSG_ELEM_S *pElem = (CMD_RUNMSG_ELEM_S *)pElemMsg;

	if (NULL == pElem) {
		return NULL;
	}

	return pElem->aszElmtArray;
}

ULONG cmd_get_ulong_param(VOID *pElemMsg)
{
	CMD_RUNMSG_ELEM_S *pElem = (CMD_RUNMSG_ELEM_S *)pElemMsg;

	if (NULL == pElem) {
		return 0xFFFFFFFF;
	}

	return atol(pElem->aszElmtArray);
}

VOID cmd_copy_string_param(VOID *pElemMsg, CHAR *param)
{
	CMD_RUNMSG_ELEM_S *pElem = (CMD_RUNMSG_ELEM_S *)pElemMsg;

	if (NULL == pElem) {
		return;
	}

	strcpy(param, pElem->aszElmtArray);

	return;
}

ULONG cmd_get_ip_ulong_param(VOID *pElemMsg)
{
	CMD_RUNMSG_ELEM_S *pElem = (CMD_RUNMSG_ELEM_S *)pElemMsg;

	if (NULL == pElem) {
		return 0xFFFFFFFF;
	}

	if (!cmd_string_is_ip(pElem->aszElmtArray)) {
		return 0xFFFFFFFF;
	}

	return cmd_ip_string_to_ulong(pElem->aszElmtArray);
}

ULONG cmd_run_notify(CMD_LINE_S *pstCmdLine, CMD_VTY_S *vty, CHAR **argv, ULONG argc)
{
	ULONG index = 0;
	ULONG isize = 0;
	ULONG ret = CMD_OK;
	ULONG argcNum = 0;
	CMD_RUNMSG_S *pstRunMsg = NULL;
	CMD_RUNMSG_ELEM_S *pstRunMsgElem = NULL;
	CMD_NTF_NODE_S *pstHead = g_cmd_reg_callback;

	isize = sizeof(CMD_RUNMSG_S) + argc * sizeof(CMD_RUNMSG_ELEM_S);

	pstRunMsg = (CMD_RUNMSG_S *)malloc(isize);
	if (NULL == pstRunMsg)
	{
		return CMD_ERR;
	}
	memset(pstRunMsg, 0, isize);

	pstRunMsg->vtyId = vty->vtyId;
	pstRunMsg->argc = argc;
	pstRunMsg->argv = (CMD_RUNMSG_ELEM_S *)(pstRunMsg + 1);
	pstRunMsgElem = (CMD_RUNMSG_ELEM_S *)(pstRunMsg + 1);

	for (ULONG i = 0; i < argc; i++)
	{
		CMD_ELMT_S *cmd_elem = (CMD_ELMT_S *)cmd_vector_get(pstCmdLine->pstElmtVec, i);

		CMD_debug(CMD_DEBUG_FUNC,
				  "cmd_run_notify: 0x%x, %d, %s, %s, %s, %d",
				  cmd_elem->ulElmtId, cmd_elem->eElmtType, cmd_elem->szElmtName, cmd_elem->szElmtHelp, argv[i], argc);

		if (CMD_ELEMID_NULL == cmd_elem->ulElmtId)
		{
			continue;
		}

		pstRunMsgElem->ulElmtId = cmd_elem->ulElmtId;
		if (strlen(argv[i]) >= 128)
		{
			free(pstRunMsg);
			return CMD_ERR;
		}

		strcpy(pstRunMsgElem->aszElmtArray, argv[i]);
		pstRunMsgElem++;
		argcNum++;
	}

	pstRunMsg->argc = argcNum;

	CMD_debug(CMD_DEBUG_FUNC, "cmd_run_notify, argcNum=%u", argcNum);

	while (NULL != pstHead)
	{
		if (pstCmdLine->ulMid != pstHead->ulMid)
		{
			pstHead = pstHead->pNext;
			continue;
		}

		ret = pstHead->pfCallBackFunc((VOID *)pstRunMsg);

		break;
	}

	free(pstRunMsg);

	return ret;
}

ULONG cmd_regcallback(ULONG ulMid, ULONG (*pfCallBackFunc)(VOID *pRcvMsg))
{
	CMD_NTF_NODE_S *pstNow = NULL;
	CMD_NTF_NODE_S *pstPre = NULL;
	CMD_NTF_NODE_S *pstEvtNtfNodeNew = NULL;

	pstEvtNtfNodeNew = (CMD_NTF_NODE_S *)malloc(sizeof(CMD_NTF_NODE_S));
	if (NULL == pstEvtNtfNodeNew)
	{
		return CMD_ERR;
	}

	pstEvtNtfNodeNew->ulMid = ulMid;
	pstEvtNtfNodeNew->pfCallBackFunc = pfCallBackFunc;

	if (NULL == g_cmd_reg_callback)
	{
		g_cmd_reg_callback = pstEvtNtfNodeNew;
		pstEvtNtfNodeNew->pNext = NULL;
		return CMD_OK;
	}

	pstNow = g_cmd_reg_callback;
	pstPre = pstNow;

	while (NULL != pstNow)
	{
		if (pfCallBackFunc == pstNow->pfCallBackFunc)
		{
			free(pstEvtNtfNodeNew);
			return CMD_OK;
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
			g_cmd_reg_callback = pstEvtNtfNodeNew;
		}
		else
		{
			pstPre->pNext = pstEvtNtfNodeNew;
		}
	}

	return CMD_OK;
}
