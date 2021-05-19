
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
#include "cli_core.h"
#include "cli_vty.h"
#include "cli_vty_io.h"
#include "cli_view.h"
#include "cli_history.h"
#include "cli_reg.h"
#include "cli_hook.h"
#include "cli_shell.h"

#include "pdt_common_inc.h"
#include "ic/include/debug_center_inc.h"

#if (OS_YES == OSP_MODULE_CLI)

#define CMD_debug(x, format, ...) debugcenter_print(MID_CMD, x, format, ##__VA_ARGS__)

CMD_VECTOR_S *g_cmd_vec = NULL;	/* command vector, every comnad has one */
CMD_ELMT_S g_cmd_elmt_cr = {CMD_ELEM_TYPE_END, CMD_ELEM_ID_CR, CMD_END, ""}; /* the command element of CR */

CMD_KEY_HANDLE_S g_cmd_key_handle[] = {
	{CMD_KEY_CODE_FILTER, 		cmd_handle_filter},
	{CMD_KEY_CODE_TAB, 			cmd_handle_tab},
	{CMD_KEY_CODE_ENTER, 		cmd_handle_enter},
	{CMD_KEY_CODE_QUEST, 		cmd_handle_quest},
	{CMD_KEY_CODE_UP, 			cmd_handle_up},
	{CMD_KEY_CODE_DOWN, 		cmd_handle_down},
	{CMD_KEY_CODE_LEFT, 		cmd_handle_left},
	{CMD_KEY_CODE_RIGHT, 		cmd_handle_right},
	{CMD_KEY_CODE_DELETE, 		cmd_handle_delete},
	{CMD_KEY_CODE_BACKSPACE, 	cmd_handle_backspace},
	{CMD_KEY_CODE_DEL_LASTWORD, cmd_hanlde_del_lastword},
	{CMD_KEY_CODE_INSERT, 		cmd_handle_insert},
	{CMD_KEY_CODE_MAX, 			NULL}
};

CMD_VECTOR_S *cmd_vector_init()
{
	CMD_VECTOR_S *v = (CMD_VECTOR_S *)malloc(sizeof(CMD_VECTOR_S));
	if (v == NULL) {
		return NULL;
	}
	memset(v, 0, sizeof(CMD_VECTOR_S));

	v->ulSize = 0;
	v->ppData = NULL;

	return v;
}

VOID cmd_vector_deinit(CMD_VECTOR_S *v, ULONG freeall)
{
	if (v == NULL) {
		return;
	}

	if (v->ppData) {
		if (freeall) {
			for (ULONG i = 0; i < cmd_vector_size(v); i++) {
				if (cmd_vector_get(v, i)) {
					free(cmd_vector_get(v, i));
				}
			}
		}

		free(v->ppData);
	}

	free(v);
}

CMD_VECTOR_S *cmd_vector_copy(CMD_VECTOR_S *v)
{
	ULONG size = 0;
	CMD_VECTOR_S *new_v = (CMD_VECTOR_S *)malloc(sizeof(CMD_VECTOR_S));
	if (NULL == new_v) {
		CMD_DBGASSERT(0, "cmd_vector_copy");
		return NULL;
	}
	memset(new_v, 0, sizeof(CMD_VECTOR_S));

	new_v->ulSize = v->ulSize;

	size = sizeof(void *) * (v->ulSize);
	new_v->ppData = (void **)malloc(size);
	if (NULL == new_v->ppData) {
		CMD_DBGASSERT(0, "cmd_vector_copy");
		return NULL;
	}
	memset(new_v->ppData, 0, size);

	memcpy(new_v->ppData, v->ppData, size);

	return new_v;
}

VOID cmd_vector_insert(CMD_VECTOR_S *v, VOID *val)
{
	ULONG size = sizeof(void *) * (v->ulSize + 1);

	v->ppData = (void **)realloc(v->ppData, size);
	if (!v->ppData) {
		CMD_debug(CMD_DEBUG_ERROR, "cmd_vector_insert, no enough memory for data.");
		CMD_DBGASSERT(0, "cmd_vector_insert, no enough memory for data.");
		return;
	}

	memset(&v->ppData[v->ulSize], 0, sizeof(void *) * 1);

	v->ppData[v->ulSize] = val;
	v->ulSize += 1;

	return;
}

VOID cmd_vector_insert_cr(CMD_VECTOR_S *v)
{
	CHAR *string_cr = (char *)malloc(sizeof(CMD_END));
	if (NULL == string_cr) {
		CMD_DBGASSERT(0, "cmd_vector_insert_cr");
		return;
	}

	memcpy(string_cr, CMD_END, sizeof(CMD_END));
	cmd_vector_insert(v, string_cr);
}

CMD_VECTOR_S *cmd_vector_new()
{
	return cmd_vector_init();
}

VOID cmd_vector_free(CMD_VECTOR_S **pVec)
{
	cmd_vector_deinit(*pVec, 1);
	*pVec = NULL;
}

CMD_VECTOR_S *cmd_str2vec(CHAR *string)
{
	ULONG str_len = 0;
	CHAR *cur = NULL;
	CHAR *start = NULL;
	CHAR *token = NULL;
	CMD_VECTOR_S *vec = NULL;

	if (string == NULL) {
		return NULL;
	}

	cur = string;

	/* skip prefix space */
	while (*cur == ' ' && *cur != '\0') {
		cur++;
	}

	if (*cur == '\0') {
		return NULL;
	}

	vec = cmd_vector_init();
	if (NULL == vec) {
		return NULL;
	}

	while (1) {
		start = cur;
		while (!(*cur == ' ' || *cur == '\r' || *cur == '\n') && *cur != '\0') {
			cur++;
		}

		str_len = cur - start;
		token = (CHAR *)malloc(sizeof(CHAR) * (str_len + 1));
		if (NULL == token) {
			CMD_debug(CMD_DEBUG_ERROR, "In cmd_str2vec, There is no memory for param token.");
			return NULL;
		}

		memcpy(token, start, str_len);
		*(token + str_len) = '\0';
		cmd_vector_insert(vec, (void *)token);

		while ((*cur == ' ' || *cur == '\n' || *cur == '\r') && *cur != '\0') {
			cur++;
		}

		if (*cur == '\0') {
			return vec;
		}
	}
}

CMD_VECTOR_S *cmd_cmd2vec(CMD_VECTOR_S *pVec, ULONG *pCmdLine, ULONG n)
{
	CMD_VECTOR_S *vec = NULL;
	CMD_ELMT_S *pstCmdElem = NULL;

	if (NULL == pVec || NULL == pCmdLine) {
		CMD_DBGASSERT(0, "cmd cmd2vec, pVec or pCmdLine is null.");
		return NULL;
	}

	vec = cmd_vector_init();
	if (NULL == vec) {
		CMD_debug(CMD_DEBUG_ERROR, "cmd_cmd2vec, cmd_vector_init failed.");
		return NULL;
	}

	for (ULONG i = 0; i < n; i++) {
		if (0 == pCmdLine[i]) {
			CMD_DBGASSERT(0, "cmd cmd2vec, cmdline index cannot be 0.");
			return NULL;
		}

		if (pCmdLine[i] - 1 >= cmd_vector_size(pVec)) {
			CMD_debug(CMD_DEBUG_ERROR, "cmd cmd2vec, cmdline index(%u) cannot more than %u.",
					  pCmdLine[i], cmd_vector_size(pVec));

			CMD_DBGASSERT(0,
						  "cmd cmd2vec, cmdline index(%u) cannot more than %u.",
						  pCmdLine[i], cmd_vector_size(pVec));
			return NULL;
		}

		pstCmdElem = (CMD_ELMT_S *)cmd_vector_get(pVec, pCmdLine[i] - 1);

		CMD_ELMT_S *pstCmdElmtTmp = NULL;
		pstCmdElmtTmp = (CMD_ELMT_S *)malloc(sizeof(CMD_ELMT_S));
		if (NULL == pstCmdElmtTmp) {
			return NULL;
		}

		pstCmdElmtTmp->ulElmtId = pstCmdElem->ulElmtId;
		pstCmdElmtTmp->eElmtType = pstCmdElem->eElmtType;
		strcpy(pstCmdElmtTmp->szElmtName, pstCmdElem->szElmtName);
		strcpy(pstCmdElmtTmp->szElmtHelp, pstCmdElem->szElmtHelp);
		pstCmdElmtTmp->pfElmtCheckFunc = pstCmdElem->pfElmtCheckFunc;
		pstCmdElmtTmp->pfElmtHelpFunc = pstCmdElem->pfElmtHelpFunc;

		cmd_vector_insert(vec, (void *)pstCmdElmtTmp);
	}

	cmd_vector_insert(vec, (void *)&g_cmd_elmt_cr);

	return vec;
}

ULONG cmd_get_command_string(CMD_LINE_S *pstLine, CHAR *pszCmdString, int iSize)
{
	ULONG ulLoop = 0;
	int iLen = 0;
	CMD_ELMT_S *pstElmt = NULL;

	if (NULL == pstLine ||
		NULL == pszCmdString)
	{
		return CMD_ERR;
	}

	for (ulLoop = 0; ulLoop < pstLine->ulElmtNum; ulLoop++)
	{
		if (0 != ulLoop)
		{
			iLen += sprintf(pszCmdString + iLen, " ");
		}

		pstElmt = (CMD_ELMT_S *)pstLine->pstElmtVec->ppData[ulLoop];

		iLen += sprintf(pszCmdString + iLen, pstElmt->szElmtName);
	}

	return CMD_OK;
}

static ULONG cmd_match_unique_string(CMD_ELMT_S *pstCmdElem, CHAR *str, ULONG size)
{
	ULONG i;

	for (i = 0; i < size; i++)
	{
		if (strcmp(pstCmdElem[i].szElmtName, str) == 0)
		{
			return 0;
		}
	}

	return 1;
}

static ULONG cmd_match_unique_elmtid(CMD_ELMT_S *pstCmdElem, ULONG ulElmtId, ULONG size)
{
	ULONG i;

	for (i = 0; i < size; i++)
	{
		CMD_debug(CMD_DEBUG_FUNC, "cmd_match_unique_elmtid. (ulElmtId=0x%x, 0x%x)",
				  pstCmdElem[i].ulElmtId,
				  ulElmtId);

		if (pstCmdElem[i].ulElmtId == ulElmtId)
		{
			return 0;
		}
	}

	return 1;
}

ULONG cmd_get_nth_elem_pos(CHAR *string, ULONG n, ULONG *pos)
{
	ULONG m = 0;
	CHAR *cur, *pre, *start;

	*pos = 0;

	// empty string
	if (string == NULL)
	{
		return CMD_ERR;
	}

	cur = string;
	pre = NULL;

	// skip white spaces
	while (*cur == ' ' && *cur != '\0')
	{
		pre = cur;
		cur++;
	}

	// only white spaces
	if (*cur == '\0')
	{
		return CMD_ERR;
	}

	while (1)
	{
		start = cur;
		while (!(*cur == ' ' || *cur == '\r' || *cur == '\n') &&
			   *cur != '\0')
		{
			pre = cur;
			cur++;
		}

		if (n == m)
		{
			*pos = (int)(start - string);
			break;
		}
		else
		{
			m++;
		}

		while ((*cur == ' ' || *cur == '\n' || *cur == '\r') &&
			   *cur != '\0')
		{
			pre = cur;
			cur++;
		}

		if (*cur == '\0')
		{
			/* BEGIN: Added by weizengke, 2014/3/9 修复命令行不完全时，错误位置提示不准确的问题 */
			*pos = (int)(cur - string);

			if (*pre != ' ')
			{
				*pos += 1;
			}

			break;
		}
	}

	return CMD_OK;
}

VOID cmd_output_missmatch(CMD_VTY_S *vty, ULONG ulNoMatchPos)
{
	ULONG i = 0;
	ULONG n = 0;
	ULONG view_len = cmd_view_get_alias_lenth(vty->view_id);
	ULONG pos_arrow = strlen(cmd_get_sysname()) + view_len + 1;

	/* 系统名与视图之间有一个链接符- */
	if (view_len > 0)
	{
		pos_arrow += 1;
	}

	(VOID) cmd_get_nth_elem_pos(vty->szBuffer, ulNoMatchPos, &n);
	pos_arrow += n;

	for (i = 0; i < pos_arrow; i++)
	{
		cmd_vty_printf(vty, " ");
	}

	cmd_vty_printf(vty, "^\r\nError: Unrecognized command at '^' position.\r\n");
}


ULONG cmd_elem_is_para_type(CMD_ELEM_TYPE_E type)
{
	switch (type)
	{
	case CMD_ELEM_TYPE_INTEGER:
	case CMD_ELEM_TYPE_STRING:
	case CMD_ELEM_TYPE_IP:
	case CMD_ELEM_TYPE_MAC:
		return CMD_YES;
	default:
		break;
	}

	return CMD_NO;
}

ULONG cmd_match_command_integer(CHAR *icmd, CMD_ELMT_S *pstElmt)
{
	ULONG a = 0;
	ULONG b = 0;
	CHAR *pleft = NULL;
	CHAR *pright = NULL;
	CHAR *pline = NULL;
	CHAR type_string[256] = {0};
	ULONG icmd_i = 0;

	if (icmd == NULL || pstElmt == NULL)
	{
		return CMD_ERR;
	}

	pleft = strchr(pstElmt->szElmtName, '<');
	pline = strchr(pstElmt->szElmtName, '-');
	pright = strchr(pstElmt->szElmtName, '>');

	if (pleft == NULL || pline == NULL || pright == NULL)
	{
		return CMD_ERR;
	}

	sscanf(pstElmt->szElmtName, "%[A-Z]<%u-%u>", type_string, &a, &b);
	if (0 != strcmp(type_string, CMD_INTEGER))
	{
		return CMD_ERR;
	}

	CMD_debug(CMD_DEBUG_INFO, "%s<%u-%u>", type_string, a, b);

	/* icmd must only has digit */
	if (TRUE == cmd_string_isdigit(icmd))
	{
		/* ULONG is not larger than 4294967295 */
		if (strlen(icmd) > 10
			|| (strlen(icmd) == 10 && strcmp(icmd,"4294967295") > 0))
		{
			return CMD_ERR;
		}

		/* check range is valid */
		icmd_i = atol(icmd);
		if (icmd_i >= a && icmd_i <= b)
		{
			return CMD_OK;
		}
	}

	return CMD_ERR;
}

ULONG cmd_match_command_string(CHAR *icmd, CMD_ELMT_S *pstElmt)
{
	ULONG a = 0;
	ULONG b = 0;
	CHAR *pleft = NULL;
	CHAR *pright = NULL;
	CHAR *pline = NULL;
	CHAR type_string[256] = {0};
	ULONG icmd_len = 0;

	if (icmd == NULL || pstElmt == NULL)
	{
		return CMD_ERR;
	}

	pleft = strchr(pstElmt->szElmtName, '<');
	pline = strchr(pstElmt->szElmtName, '-');
	pright = strchr(pstElmt->szElmtName, '>');

	if (pleft == NULL || pline == NULL || pright == NULL)
	{
		return CMD_ERR;
	}

	sscanf(pstElmt->szElmtName, "%[A-Z]<%u-%u>", type_string, &a, &b);
	if (0 != strcmp(type_string, CMD_STRING))
	{
		return CMD_ERR;
	}

	CMD_debug(CMD_DEBUG_INFO, "%s<%u-%u>", type_string, a, b);

	/* 检查整形参数是否在范围 */
	icmd_len = (ULONG)strlen(icmd);
	if (icmd_len >= a && icmd_len <= b)
	{
		return CMD_OK;
	}

	return CMD_ERR;
}

ULONG cmd_match_command_ip(CHAR *icmd, CMD_ELMT_S *pstElmt)
{

	if (icmd == NULL || pstElmt == NULL)
	{
		return CMD_ERR;
	}

	if (0 != strcmp(pstElmt->szElmtName, CMD_IP))
	{
		return CMD_ERR;
	}

	if (cmd_string_is_ip(icmd))
	{
		return CMD_OK;
	}
	else
	{
		return CMD_ERR;
	}
}

ULONG cmd_match_command_param(CHAR *icmd, CMD_ELMT_S *pstElmt)
{
	ULONG ulRet = CMD_ERR;

	if (icmd == NULL || pstElmt == NULL)
	{
		return CMD_ERR;
	}

	if (NULL != pstElmt->pfElmtCheckFunc)
	{
		return pstElmt->pfElmtCheckFunc(icmd);
	}

	//CMD_debug(CMD_DEBUG_INFO, "cmd_match_command_param. icmd=%s, pszElmtName=%s",icmd, pstElmt->pszElmtName);

	switch (pstElmt->eElmtType)
	{
	case CMD_ELEM_TYPE_INTEGER:
		ulRet = cmd_match_command_integer(icmd, pstElmt);

		break;

	case CMD_ELEM_TYPE_STRING:
		ulRet = cmd_match_command_string(icmd, pstElmt);

		break;

	case CMD_ELEM_TYPE_IP:
		ulRet = cmd_match_command_ip(icmd, pstElmt);

		break;

	case CMD_ELEM_TYPE_MAC:
		break;

	default:
		break;
	}

	return ulRet;
}

static ULONG cmd_filter_command(CMD_VTY_S *vty, CHAR *cmd, CMD_VECTOR_S *v, ULONG index)
{
	ULONG i;
	ULONG match_cmd = CMD_ERR;
	CMD_LINE_S *pstCmdLine;
	CMD_ELMT_S *pstCmdElem;

	if (cmd == NULL || 0 == strlen(cmd))
	{
		return CMD_ERR;
	}

	/* <CR> 不参与过滤，防止命令行子串也属于命令行时误过 */
	if (0 == strcmp(cmd, CMD_END))
	{
		return CMD_OK;
	}

	for (i = 0; i < cmd_vector_size(v); i++)
	{
		if ((pstCmdLine = (CMD_LINE_S *)cmd_vector_get(v, i)) != NULL)
		{
			if (pstCmdLine->ulViewId != VIEW_GLOBAL
				&& pstCmdLine->ulViewId != vty->view_id)
			{
				continue;
			}

			if (index >= cmd_vector_size(pstCmdLine->pstElmtVec))
			{
				cmd_vector_get(v, i) = NULL;
				continue;
			}

			pstCmdElem = (CMD_ELMT_S *)cmd_vector_get(pstCmdLine->pstElmtVec, index);

			/* match STRING , INTEGER */
			if (CMD_OK != cmd_match_command_param(cmd, pstCmdElem))
			{
				if (util_strnicmp(cmd, pstCmdElem->szElmtName, strlen(cmd)) != 0)
				{
#if 0
					CMD_debug(CMD_DEBUG_INFO, "cmd_filter_command set null. "
								"(cmd=%s, szElmtName=%s)",
								vty->szBuffer, pstCmdElem->szElmtName);
#endif

					cmd_vector_get(v, i) = NULL;
					continue;
				}
			}

			/* BEGIN: Added by weizengke, 2013/11/19 for support unkown cmd pos*/
			match_cmd = CMD_OK;
			/* END:   Added by weizengke, 2013/11/19 */
		}
	}

	CMD_debug(CMD_DEBUG_INFO, "cmd_filter_command. (cmd=%s, match_res=%u)", vty->szBuffer, match_cmd);

	return match_cmd;
}

ULONG cmd_match_command(CMD_VECTOR_S *icmd_vec, CMD_VTY_S *vty,
						CMD_ELMT_S *pstMatchCmdElem, ULONG *pulMatchNum)
{
	ULONG i = 0;
	ULONG isize = 0;
	ULONG size = 0;
	CMD_VECTOR_S *cmd_vec_copy = cmd_vector_copy(g_cmd_vec);

	if (NULL == icmd_vec || NULL == cmd_vec_copy)
	{
		return CMD_NO_MATCH;
	}

	isize = cmd_vector_size(icmd_vec) - 1;

	/* scan all command vectors, set null while not match. */
	for (i = 0; i < isize; i++)
	{
		if (CMD_OK != cmd_filter_command(vty, (CHAR *)cmd_vector_get(icmd_vec, i), cmd_vec_copy, i))
		{
			*pulMatchNum = 0;
			return CMD_NO_MATCH;
		}
	}

	for (i = 0; i < cmd_vector_size(cmd_vec_copy); i++)
	{
		CMD_LINE_S *pstCmdLine = NULL;
		pstCmdLine = (CMD_LINE_S *)cmd_vector_get(cmd_vec_copy, i);

		if (pstCmdLine != NULL)
		{
			if (pstCmdLine->pstElmtVec == NULL)
			{
				continue;
			}

			if (pstCmdLine->ulViewId != VIEW_GLOBAL
				&& pstCmdLine->ulViewId != vty->view_id)
			{
				continue;
			}

			if (isize >= cmd_vector_size(pstCmdLine->pstElmtVec))
			{
				cmd_vector_get(cmd_vec_copy, i) = NULL;
				continue;
			}

			CMD_ELMT_S *pstCmdElem = (CMD_ELMT_S *)cmd_vector_get(pstCmdLine->pstElmtVec, isize);
			CHAR *str = (CHAR *)cmd_vector_get(icmd_vec, isize);

			if (str == NULL || util_strnicmp(str, pstCmdElem->szElmtName, strlen(str)) == 0)
			{
				if (cmd_match_unique_string(pstMatchCmdElem, pstCmdElem->szElmtName, size))
				{
					memcpy(&pstMatchCmdElem[size++], pstCmdElem, sizeof(CMD_ELMT_S));
				}
			}
		}
	}

	cmd_vector_deinit(cmd_vec_copy, 0);

	/* No command matched */
	if (size == 0)
	{
		*pulMatchNum = size;
		return CMD_NO_MATCH;
	}

	/* Only one command matched */
	if (size == 1)
	{
		*pulMatchNum = size;
		return CMD_FULL_MATCH;
	}

	/* List matched */
	*pulMatchNum = size;
	return CMD_LIST_MATCH;
}

ULONG cmd_complete_command(CMD_VECTOR_S *icmd_vec, CMD_VTY_S *vty,
						   CMD_ELMT_S *pstCmdElem, ULONG *pulMatchNum, ULONG *pulNoMatchPos)
{

	ULONG i = 0;
	CMD_VECTOR_S *cmd_vec_copy = cmd_vector_copy(g_cmd_vec);
	ULONG ulMatchNum = 0;
	CHAR *str = NULL;
	CMD_ELMT_S *pstCmdElemTmp = NULL;
	CMD_LINE_S *pstCmdLine = NULL;

	if (icmd_vec == NULL || vty == NULL || pstCmdElem == NULL || pulMatchNum == NULL)
	{
		return CMD_ERR;
	}

	*pulNoMatchPos = CMD_NULL_DWORD;

	/* scan all command vectors, set null while not match. */
	/* BEGIN: Modified by weizengke, 2013/10/4   PN:循环过滤每一个向 */
	for (i = 0; i < cmd_vector_size(icmd_vec); i++)
	{
		if (CMD_OK != cmd_filter_command(vty, (CHAR *)cmd_vector_get(icmd_vec, i), cmd_vec_copy, i))
		{
			/* BEGIN: Added by weizengke, 2013/11/19 这里可以优化，不命中可以不需要再匹配 */
			/* 保存在第几个命令字无法匹 */
			*pulNoMatchPos = (*pulNoMatchPos == CMD_NULL_DWORD) ? (i) : (*pulNoMatchPos);

			CMD_debug(CMD_DEBUG_ERROR, "cmd_complete_command, cannot match at pos %u", *pulNoMatchPos);

			cmd_vector_deinit(cmd_vec_copy, 0);

			return CMD_OK;
		}
	}
	/* END:   Modified by weizengke, 2013/10/4   PN:None */

	for (i = 0; i < cmd_vector_size(cmd_vec_copy); i++)
	{
		pstCmdLine = (CMD_LINE_S *)cmd_vector_get(cmd_vec_copy, i);

		if (pstCmdLine != NULL)
		{
			if (cmd_vector_size(icmd_vec) - 1 >= cmd_vector_size(pstCmdLine->pstElmtVec))
			{
				cmd_vector_get(cmd_vec_copy, i) = NULL;
				continue;
			}

			{
				char szBuf[CMD_BUFFER_SIZE] = {0};
				(VOID) cmd_get_command_string(pstCmdLine, szBuf, CMD_BUFFER_SIZE);
				CMD_debug(CMD_DEBUG_FUNC, "cmd_complete_command, ulIndex=%u, command=%s.", pstCmdLine->ulIndex, szBuf);
			}

			if (pstCmdLine->ulViewId != VIEW_GLOBAL
				&& pstCmdLine->ulViewId != vty->view_id)
			{
				continue;
			}

			str = (CHAR *)cmd_vector_get(icmd_vec, cmd_vector_size(icmd_vec) - 1);
			pstCmdElemTmp = (CMD_ELMT_S *)cmd_vector_get(pstCmdLine->pstElmtVec, cmd_vector_size(icmd_vec) - 1);
			if (pstCmdElemTmp == NULL)
			{
				continue;
			}

			CMD_debug(CMD_DEBUG_FUNC, "cmd_complete_command, ulElmtId=0x%x, eElmtType=%u, str=%s, szElmtName=%s, pfElmtHelpFunc=0x%x",
					  pstCmdElemTmp->ulElmtId, pstCmdElemTmp->eElmtType, str, pstCmdElemTmp->szElmtName, pstCmdElemTmp->pfElmtHelpFunc);

			/* BEGIN: Added by weizengke, 2013/11/19 */
			/* match STRING , INTEGER
			   bug: ip complete
			*/
			if (NULL == pstCmdElemTmp->pfElmtHelpFunc)
			{
				/* match param */
				if (CMD_YES == cmd_elem_is_para_type(pstCmdElemTmp->eElmtType))
				{
					if (0 == util_strnicmp(str, CMD_END, strlen(str))
						|| 0 == util_strnicmp(pstCmdElemTmp->szElmtName, CMD_END, strlen(pstCmdElemTmp->szElmtName))
						|| CMD_OK == cmd_match_command_param(str, pstCmdElemTmp))
					{
						CMD_debug(CMD_DEBUG_FUNC, "cmd_complete_command, eElmtType=%u, pszElmtName=%s, str=%s",
								  pstCmdElemTmp->eElmtType,
								  pstCmdElemTmp->szElmtName,
								  str);

						if (cmd_match_unique_elmtid(pstCmdElem, pstCmdElemTmp->ulElmtId, ulMatchNum))
						{
							memcpy(&pstCmdElem[ulMatchNum], pstCmdElemTmp, sizeof(CMD_ELMT_S));
							ulMatchNum++;
						}
					}
					else
					{
						CMD_debug(CMD_DEBUG_FUNC, "cmd_complete_command, other, eElmtType=%u, pszElmtName=%s",
								  pstCmdElemTmp->eElmtType,
								  pstCmdElemTmp->szElmtName);
					}
				}
				/* END:   Added by weizengke, 2013/11/19 */
				else
				{
					CMD_debug(CMD_DEBUG_FUNC, "cmd_complete_command, key, eElmtType=%u, pszElmtName=%s",
							  pstCmdElemTmp->eElmtType,
							  pstCmdElemTmp->szElmtName);

					/* match key */
					if (0 == util_strnicmp(str, CMD_END, strlen(str)) /* 无任何输入时输入 */
						|| util_strnicmp(str, pstCmdElemTmp->szElmtName, strlen(str)) == 0)
					{
						/* get only one if more than one  */
						if (cmd_match_unique_string(pstCmdElem, pstCmdElemTmp->szElmtName, ulMatchNum))
						{
							memcpy(&pstCmdElem[ulMatchNum], pstCmdElemTmp, sizeof(CMD_ELMT_S));
							ulMatchNum++;
						}
					}
				}
			}
			else
			{
				ULONG ulRet = 0;
				CMD_ELMTHELP_S *pstCmdElmtHelp = NULL;
				ULONG ulNum = 0;

				ulRet = pstCmdElemTmp->pfElmtHelpFunc(NULL, &pstCmdElmtHelp, &ulNum);
				if (0 == ulRet)
				{
					ULONG ulLoop = 0;
					for (ulLoop = 0; ulLoop < ulNum; ulLoop++)
					{
						strcpy(pstCmdElem[ulMatchNum].szElmtName, pstCmdElmtHelp[ulLoop].szElmtName);
						strcpy(pstCmdElem[ulMatchNum].szElmtHelp, pstCmdElmtHelp[ulLoop].szElmtHelp);
						ulMatchNum++;
					}

					free(pstCmdElmtHelp);
				}
			}
		}
	}

	cmd_vector_deinit(cmd_vec_copy, 0);

	CMD_debug(CMD_DEBUG_FUNC, "cmd_complete_command, ulMatchNum=%u", ulMatchNum);

	/* BEGIN: Added by weizengke, 2013/10/27 sort for ? complete */
	if (ulMatchNum > 0)
	{
		int j;
		for (i = 0; i < ulMatchNum - 1; i++)
		{
			for (j = i; j < ulMatchNum; j++)
			{
				CMD_ELMT_S stCmdElem_;
				
				if (0 == util_strnicmp(pstCmdElem[i].szElmtName, CMD_END, strlen(pstCmdElem[i].szElmtName))
					|| ( 1 == util_stricmp(pstCmdElem[i].szElmtName, pstCmdElem[j].szElmtName)
					&& 0 != util_strnicmp(pstCmdElem[j].szElmtName, CMD_END, strlen(pstCmdElem[j].szElmtName)))
					)
				{
					memcpy(&stCmdElem_, &pstCmdElem[i], sizeof(CMD_ELMT_S));
					memcpy(&pstCmdElem[i], &pstCmdElem[j], sizeof(CMD_ELMT_S));
					memcpy(&pstCmdElem[j], &stCmdElem_, sizeof(CMD_ELMT_S));
				}
			}
		}
	}
	/* END: Added by weizengke, 2013/10/27 sort for ? complete */

	*pulMatchNum = ulMatchNum;

	return CMD_OK;
}

ULONG cmd_execute_command(CMD_VECTOR_S *icmd_vec, CMD_VTY_S *vty,
						  CMD_ELMT_S **ppstCmdElem, ULONG *pulMatchNum, ULONG *pulNoMatchPos)
{
	ULONG i;
	CMD_VECTOR_S *cmd_vec_copy = cmd_vector_copy(g_cmd_vec);
	CMD_LINE_S *pstMatchCmdLine = NULL;
	ULONG ulMatchNum = 0;

	*pulNoMatchPos = CMD_NULL_DWORD;

	CMD_debug(CMD_DEBUG_INFO, "cmd_execute_command. (cmd=%s, view_id=%u)", vty->szBuffer, vty->view_id);

	for (i = 0; i < cmd_vector_size(icmd_vec); i++)
	{
		if (CMD_OK != cmd_filter_command(vty, (CHAR *)cmd_vector_get(icmd_vec, i), cmd_vec_copy, i))
		{
			/* BEGIN: Added by weizengke, 2013/11/19 不命中可以不需要再匹配 */
			/* 保存在第几个命令字无法匹 */
			*pulNoMatchPos = (*pulNoMatchPos == CMD_NULL_DWORD) ? (i) : (*pulNoMatchPos);

			CMD_debug(CMD_DEBUG_ERROR, "cmd_execute_command. not match at pos %u.", *pulNoMatchPos);

			cmd_vector_deinit(cmd_vec_copy, 0);

			return CMD_NO_MATCH;
		}
	}

	if (*pulNoMatchPos == CMD_NULL_DWORD)
	{
		/* BEGIN: Added by weizengke, 2014/3/9 修复命令行不完全时，错误位置提示不准确的问题 */
		*pulNoMatchPos = cmd_vector_size(icmd_vec) - 1;

		for (i = 0; i < cmd_vector_size(cmd_vec_copy); i++)
		{
			char *str;
			CMD_ELMT_S *pstCmdElem;
			CMD_LINE_S *pstCmdLine = NULL;

			pstCmdLine = (CMD_LINE_S *)cmd_vector_get(cmd_vec_copy, i);
			if (pstCmdLine != NULL)
			{
				if (pstCmdLine->ulViewId != VIEW_GLOBAL
				&& pstCmdLine->ulViewId != vty->view_id)
				{
					continue;
				}

				str = (CHAR *)cmd_vector_get(icmd_vec, cmd_vector_size(icmd_vec) - 1);
				pstCmdElem = (CMD_ELMT_S *)cmd_vector_get(pstCmdLine->pstElmtVec, cmd_vector_size(icmd_vec) - 1);

				/* modified for command without argv */
				if (cmd_vector_size(icmd_vec) == cmd_vector_size(pstCmdLine->pstElmtVec))
				{
					/* BEGIN: Added by weizengke, 2013/10/5   PN:for support STRING<a-b> & INTEGER<a-b> */
					if (CMD_OK == cmd_match_command_param(str, pstCmdElem) ||
						str != NULL && util_strnicmp(str, pstCmdElem->szElmtName, strlen(str)) == 0)
					{
						/* BEGIN: Added by weizengke, 2013/10/6   PN:command exec ambigous, return the last elem (not the <CR>) */
						ppstCmdElem[ulMatchNum] = (CMD_ELMT_S *)cmd_vector_get(pstCmdLine->pstElmtVec, cmd_vector_size(icmd_vec) - 2);
						/* END:   Added by weizengke, 2013/10/6   PN:None */

						ulMatchNum++;
						pstMatchCmdLine = pstCmdLine;
					}
				}
			}
		}
	}

	*pulMatchNum = ulMatchNum;

	cmd_vector_deinit(cmd_vec_copy, 0);

	CMD_debug(CMD_DEBUG_INFO, "cmd_execute_command. (cmd=%s, ulMatchNum=%u)", vty->szBuffer, ulMatchNum);

	if (ulMatchNum == 0)
	{
		return CMD_NO_MATCH;
	}

	/* BEGIN: Added by weizengke, 2013/10/6   PN:command exec ambigous */
	if (ulMatchNum > 1)
	{
		return CMD_ERR_AMBIGOUS;
	}
	/* END:   Added by weizengke, 2013/10/6   PN:None */

	/* BEGIN: Added by weizengke, 2013/10/5  for support argv. PN:push param into function stack */
	ULONG argc = 0;
	CHAR *argv[CMD_MAX_CMD_NUM];

	if (NULL == pstMatchCmdLine || pstMatchCmdLine->pstElmtVec == NULL)
	{
		return CMD_NO_MATCH;
	}

	for (i = 0, argc = 0; i < pstMatchCmdLine->ulElmtNum; i++)
	{
		CMD_ELMT_S *pstCmdElem = (CMD_ELMT_S *)cmd_vector_get(pstMatchCmdLine->pstElmtVec, i);
		if (NULL == pstCmdElem)
		{
			return CMD_NO_MATCH;
		}

		/* <CR> no need to push */
		if (pstCmdElem->eElmtType == CMD_ELEM_TYPE_END)
		{
			continue;
		}

		/* push param to argv and full command*/
		if (CMD_YES == cmd_elem_is_para_type(pstCmdElem->eElmtType))
		{
			argv[argc++] = (CHAR *)cmd_vector_get(icmd_vec, i);
		}
		else
		{
			argv[argc++] = pstCmdElem->szElmtName;
		}
	}
	/* END:   Added by weizengke, 2013/10/5   PN:None */

	/* execute command */
	(VOID) cmd_run_notify(pstMatchCmdLine, vty, argv, argc);

	return CMD_FULL_MATCH;
}

VOID cmd_install_command(ULONG mid, ULONG cmd_view, CHAR *cmd_string, CMD_VECTOR_S *pVec)
{
	ULONG iIndex[128] = {0};
	ULONG iLoop = 0;
	ULONG n = 0;
	CHAR *cur = cmd_string;
	static ULONG ulStaticIndex = 1;

	if (NULL == pVec) {
		return;
	}

	/* 计算线索表达式数 */
	for (; *cur != '\0';) {
		if (*cur >= '0' && *cur <= '9') {
			n *= 10;
			n += *cur - '0';
		} else if (n > 0) {
			iIndex[iLoop++] = n;
			n = 0;
		}
		cur++;
	}

	if (g_cmd_vec == NULL) {
		CMD_debug(CMD_DEBUG_ERROR, "Command Vector Not Exist");
		return;
	}

	CMD_LINE_S *pstCmdLine = NULL;

	pstCmdLine = (CMD_LINE_S *)malloc(sizeof(CMD_LINE_S));
	if (NULL == pstCmdLine) {
		return;
	}
	memset(pstCmdLine, 0, sizeof(CMD_LINE_S));

	pstCmdLine->ulIndex = ulStaticIndex++;
	pstCmdLine->ulMid = mid;
	pstCmdLine->ulViewId = cmd_view;
	pstCmdLine->pstElmtVec = cmd_cmd2vec(pVec, iIndex, iLoop);
	if (NULL == pstCmdLine->pstElmtVec) {
		free(pstCmdLine);
		CMD_debug(CMD_DEBUG_ERROR, "cmd_install_command, pstElmtVec is null");
		return;
	}
	pstCmdLine->ulElmtNum = cmd_vector_size(pstCmdLine->pstElmtVec);

	{
		char szBuf[CMD_BUFFER_SIZE] = {0};
		(VOID) cmd_get_command_string(pstCmdLine, szBuf, CMD_BUFFER_SIZE);
		CMD_debug(CMD_DEBUG_INFO, "cmd_install_command: index=%u, command=%s.", pstCmdLine->ulIndex, szBuf);
	}

	cmd_vector_insert(g_cmd_vec, pstCmdLine);

	return;
}
/* end cmd */

/* resolve */

/* Note: Added by weizengke, 2013/10/04 insert a word into the tail of input buffer */
VOID cmd_insert_word(CMD_VTY_S *vty, CHAR *str)
{
	strcat(vty->szBuffer, str);
	vty->ulCurrentPos += (int)strlen(str);
	vty->ulUsedLen += (int)strlen(str);
}

/* Note: Added by weizengke, 2013/10/04 delete the last word from input buffer */
VOID cmd_delete_word(CMD_VTY_S *vty)
{
	int pos = strlen(vty->szBuffer);

	while (pos > 0 && vty->szBuffer[pos - 1] != ' ')
	{
		pos--;
	}

	/* BEGIN: Added by weizengke, 2014/8/3 */
	//vty->buffer[pos] = '\0';
	memset(&vty->szBuffer[pos], 0, sizeof(vty->szBuffer) - pos);
	/* END:   Added by weizengke, 2014/8/3 */

	vty->ulCurrentPos = strlen(vty->szBuffer);
	vty->ulUsedLen = strlen(vty->szBuffer);
}

/* Note: Added by weizengke, 2014/3/23 delete the last word at current pos from input buffer*/
VOID cmd_delete_word_ctrl_W_ex(CMD_VTY_S *vty)
{
	/* 删除光标所在当前或之前elem */
	int start_pos = 0;
	int end_pos = 0;
	int len = strlen(vty->szBuffer);
	int pos = vty->ulCurrentPos;

	CMD_debug(CMD_DEBUG_INFO, "ctrl_W:cur_poscur_pos = %d buffer_len = %d", pos, len);

	if (pos == 0)
	{
		return;
	}

	/* ignore suffix-space */
	if (vty->szBuffer[pos] == ' ' || vty->szBuffer[pos] == '\0')
	{
		end_pos = pos;

		pos--;
		/* 往回找第一个非空字 */
		while (pos >= 0 && vty->szBuffer[pos] == ' ')
		{
			pos--;
		}

		if (pos == 0)
		{
			start_pos = 0;
		}
		else
		{
			/* 继续往回找第一个空格或命令 */
			while (pos >= 0 && vty->szBuffer[pos] != ' ')
			{
				pos--;
			}

			start_pos = pos + 1;
		}
	}
	else
	{
		/* 分别往左右找空 */
		while (vty->szBuffer[pos + 1] != ' ')
		{
			/* BEGIN: Added by weizengke, 2014/4/5 当光标位于命令行最后一个元素中间，再执行CTRL+W，出现异常显 https://github.com/weizengke/jungle/issues/2 */
			if (vty->szBuffer[pos + 1] == '\0')
				break;
			/* END:   Added by weizengke, 2014/4/5 */

			pos++;
		}

		/* BEGIN: Modified by weizengke, 2014/4/5 https://github.com/weizengke/jungle/issues/2 */
		end_pos = pos;
		/* END:   Modified by weizengke, 2014/4/5 */

		pos = vty->ulCurrentPos;
		while (vty->szBuffer[pos] != ' ')
		{
			pos--;
		}

		start_pos = pos + 1;
	}

	int len_last = strlen(&vty->szBuffer[end_pos]);

	memcpy(&vty->szBuffer[start_pos], &vty->szBuffer[end_pos], strlen(&vty->szBuffer[end_pos]));
	memset(&vty->szBuffer[start_pos + len_last], 0, sizeof(vty->szBuffer) - (start_pos + len_last));

	vty->ulCurrentPos -= (vty->ulCurrentPos - start_pos);
	vty->ulUsedLen -= (end_pos - start_pos);

	CMD_debug(CMD_DEBUG_INFO, "ctrl+w end: buffer=%s, start_pos=%d end_pos=%d len_last=%d cur_pos=%d used_len=%d",
			  vty->szBuffer, start_pos, end_pos, len_last, vty->ulCurrentPos, vty->ulUsedLen);
}

ULONG cmd_run(CMD_VTY_S *vty)
{
	CMD_VECTOR_S *v = NULL;
	CMD_ELMT_S *pstCmdElem[CMD_MAX_MATCH_SIZE] = {0};
	ULONG ulMatchNum = 0;
	ULONG ulMatchType = CMD_NO_MATCH;
	ULONG ulNoMatchPos = CMD_NULL_DWORD;
	ULONG view_id_ = VIEW_NULL;
	ULONG view_id_pre = VIEW_NULL;

	v = cmd_str2vec(vty->szBuffer);
	if (v == NULL)
	{
		return 1;
	}

	/* BEGIN: Added by weizengke, 2013/10/5   PN:for cmd end with <CR> */
	cmd_vector_insert_cr(v);
	/* END:   Added by weizengke, 2013/10/5   PN:None */

	view_id_pre = vty->view_id;

	ulMatchType = cmd_execute_command(v, vty, pstCmdElem, &ulMatchNum, &ulNoMatchPos);

	//CMD_debug(CMD_DEBUG_INFO, "cmd_run. (cmd=%s, match_type=%u)", vty->szBuffer, ulMatchType);

	view_id_ = vty->view_id;

	/* 回退上一级视图执 */
	while (ulMatchType != CMD_FULL_MATCH)
	{
		ULONG ulMatchNum_ = 0;
		ULONG ulNoMatchPos_ = CMD_NULL_DWORD;
		ULONG view_id__ = VIEW_NULL;

		view_id__ = cmd_view_get_parent_view_id(vty->view_id);

		if (VIEW_NULL == view_id__ || VIEW_USER == view_id__ || VIEW_GLOBAL == view_id__)
		{
			vty->view_id = view_id_;
			break;
		}

		vty->view_id = view_id__;
		view_id_pre = vty->view_id;
		CMD_debug(CMD_DEBUG_FUNC, "Return back to view '%s'.", cmd_view_get_name(vty->view_id));

		ulMatchType = cmd_execute_command(v, vty, pstCmdElem, &ulMatchNum_, &ulNoMatchPos_);
	}
#if 0
	CMD_debug(CMD_DEBUG_FUNC, "Execute command '%s' on view '%s'. (result:%u, username=%s, vtyId=%u)",
			  vty->szBuffer, cmd_view_get_name(view_id_pre), ulMatchType, vty->user.user_name, vty->vtyId);

	write_log(0, "Execute command '%s' on view '%s'. (result:%u, username=%s, vtyId=%u)",
		vty->szBuffer, cmd_view_get_name(view_id_pre), ulMatchType, vty->user.user_name, vty->vtyId);
#endif

	if (CMD_FULL_MATCH != ulMatchType)
	{
		return 1;
	}

	return 0;
}

ULONG cmd_pub_run(CHAR *szCmdBuf)
{
	ULONG ret = 0;
	CMD_VTY_S *vty = cmd_vty_getById(CMD_VTY_CFM_ID);

	if (NULL == szCmdBuf) {
		return 1;
	}

	if (strlen(szCmdBuf) > CMD_BUFFER_SIZE) {
		CMD_debug(CMD_DEBUG_ERROR,
				  "cmd_pub_run. input(%u) command length larger then %u",
				  strlen(szCmdBuf), CMD_BUFFER_SIZE);
		return 1;
	}

	strcpy(vty->szBuffer, szCmdBuf);

	ret = cmd_run(vty);

	vty->ulCurrentPos = vty->ulUsedLen = 0;
	memset(vty->szBuffer, 0, sizeof(vty->szBuffer));

	return ret;
}

VOID cmd_outprompt(CMD_VTY_S *vty)
{
	if (vty->view_id == VIEW_USER) {
		cmd_vty_printf(vty, "%s>", cmd_get_sysname());
		return;
	}

	if (vty->view_id == VIEW_SYSTEM) {
		cmd_vty_printf(vty, "%s]", cmd_get_sysname());
		return;
	}

	CHAR *ais_name = cmd_view_get_alias(vty->view_id);
	if (ais_name == NULL) {
		cmd_vty_printf(vty, "%s>", cmd_get_sysname());
		return;
	} else {
		cmd_vty_printf(vty, "%s-%s]", cmd_get_sysname(), ais_name);
	}
}

VOID cmd_handle_filter(CMD_VTY_S *vty)
{
	return;
}

VOID cmd_handle_tab(CMD_VTY_S *vty)
{
	ULONG i = 0;
	CMD_VECTOR_S *v = NULL;
	CMD_ELMT_S *pstCmdElem = NULL;
	ULONG ulMatchNum = 0;
	ULONG ulMatchType = CMD_NO_MATCH;
	ULONG isNeedMatch = 1; /* 非TAB场景(无空 ，不需要匹 */
	CHAR *last_word = NULL;

	if (strlen(vty->szBuffer) < vty->ulCurrentPos)
	{
		CMD_DBGASSERT(0, "cmd_resolve_tab. (buffer=%s[%u], ulCurrentPos=%u, ulUsedLen=%u, ulBufMaxLen=%u)",
					  vty->szBuffer, strlen(vty->szBuffer), vty->ulCurrentPos, vty->ulUsedLen, vty->ulBufMaxLen);
		return;
	}

	/*
	1: 取pos 之前的buf
	2: 需要覆盖当前光标后的buf
	*/
	/* BEGIN: Added by weizengke, 2013/11/17 bug for left and tab*/
	memset(&(vty->szBuffer[vty->ulCurrentPos]), 0, strlen(vty->szBuffer) - vty->ulCurrentPos);
	vty->ulUsedLen = strlen(vty->szBuffer);
	/* END:   Added by weizengke, 2013/11/17 */

	CMD_debug(CMD_DEBUG_FUNC, "cmd_handle_tab in:szBuffer=%s, ulUsedLen=%d, ulCurrentPos=%d", vty->szBuffer, vty->ulUsedLen, vty->ulCurrentPos);

	if (vty->ucKeyTypePre == CMD_KEY_CODE_TAB)
	{
		cmd_delete_word(vty);
		cmd_insert_word(vty, vty->tabbingString);
	}
	else
	{
		memset(vty->tabString, 0, sizeof(vty->tabString));
	}

	v = cmd_str2vec(vty->szBuffer);
	if (v == NULL)
	{
		/*
		v = cmd_vector_init();
		cmd_vector_insert(v, '\0');
		*/
		isNeedMatch = 0;
	}

	if (' ' == vty->szBuffer[vty->ulUsedLen - 1])
	{
		isNeedMatch = 0;
	}

	pstCmdElem = (CMD_ELMT_S *)malloc(CMD_MAX_MATCH_SIZE * sizeof(CMD_ELMT_S));
	if (NULL == pstCmdElem)
	{
		return;
	}
	memset(pstCmdElem, 0, CMD_MAX_MATCH_SIZE * sizeof(CMD_ELMT_S));

	if (1 == isNeedMatch && NULL != v)
	{
		ulMatchType = cmd_match_command(v, vty, pstCmdElem, &ulMatchNum);

		last_word = (CHAR *)cmd_vector_get(v, cmd_vector_size(v) - 1);

		if (vty->ucKeyTypePre != CMD_KEY_CODE_TAB)
		{
			strcpy(vty->tabbingString, last_word);
		}

		cmd_vector_deinit(v, 1);
	}

	cmd_vty_printf(vty, "%s", CMD_ENTER);
	switch (ulMatchType)
	{
	case CMD_NO_MATCH:
		cmd_outprompt(vty);
		cmd_vty_printf(vty, "%s", vty->szBuffer);
		break;
	case CMD_FULL_MATCH:
		cmd_delete_word(vty);
		if (0 != ulMatchNum)
		{
			cmd_insert_word(vty, pstCmdElem[0].szElmtName);
		}
		/* BEGIN: Added by weizengke, 2013/10/14 for full match then next input*/
		cmd_insert_word(vty, " ");
		/* END:   Added by weizengke, 2013/10/14 */
		cmd_outprompt(vty);
		cmd_vty_printf(vty, "%s", vty->szBuffer);

		/* BEGIN: Added by weizengke, 2013/10/27 PN: fix the bug of CMD_FULL_MATCH and then continue TAB*/
		memset(vty->tabString, 0, sizeof(vty->tabString));
		memset(vty->tabbingString, 0, sizeof(vty->tabbingString));
		/* END:   Added by weizengke, 2013/10/27 */

		break;
	case CMD_LIST_MATCH:
		/* first TAB result */
		if (vty->ucKeyTypePre != CMD_KEY_CODE_TAB)
		{
			memset(vty->tabString, 0, sizeof(vty->tabString));
			strcpy(vty->tabString, pstCmdElem[0].szElmtName);
		}
		else
		{
			for (i = 0; i < ulMatchNum; i++)
			{
				if (0 == strcmp(vty->tabString, pstCmdElem[i].szElmtName))
				{
					break;
				}
			}

			i++;

			if (i == ulMatchNum)
			{
				i = 0;
			}

			memset(vty->tabString, 0, sizeof(vty->tabString));
			strcpy(vty->tabString, pstCmdElem[i].szElmtName);
		}

		cmd_delete_word(vty);
		cmd_insert_word(vty, vty->tabString);

		cmd_outprompt(vty);
		cmd_vty_printf(vty, "%s", vty->szBuffer);
		break;
	default:
		break;
	}

	free(pstCmdElem);

	CMD_debug(CMD_DEBUG_FUNC, "cmd_handle_tab out:szBuffer=%s, ulUsedLen=%d, ulCurrentPos=%d", vty->szBuffer, vty->ulUsedLen, vty->ulCurrentPos);

	return;
}

VOID cmd_handle_enter(CMD_VTY_S *vty)
{
	CMD_ELMT_S *pstCmdElem[CMD_MAX_MATCH_SIZE];
	ULONG ulMatchNum = 0;
	ULONG ulMatchType = CMD_NO_MATCH;
	CMD_VECTOR_S *v = NULL;
	ULONG i = 0;
	ULONG ulNoMatchPos = CMD_NULL_DWORD;
	ULONG view_id_ = VIEW_NULL;
	ULONG view_id_pre = VIEW_NULL;

	v = cmd_str2vec(vty->szBuffer);
	if (v == NULL)
	{
		cmd_vty_printf(vty, "%s", CMD_ENTER);
		cmd_outprompt(vty);
		return;
	}

	/* BEGIN: Added by weizengke, 2013/10/5   PN:for cmd end with <CR> */
	cmd_vector_insert_cr(v);
	/* END:   Added by weizengke, 2013/10/5   PN:None */

	cmd_vty_printf(vty, "%s", CMD_ENTER);

	view_id_pre = vty->view_id;

	ulMatchType = cmd_execute_command(v, vty, pstCmdElem, &ulMatchNum, &ulNoMatchPos);

	view_id_ = vty->view_id;

	while (ulMatchType == CMD_NO_MATCH)
	{
		ULONG ulMatchNum_ = 0;
		ULONG ulNoMatchPos_ = CMD_NULL_DWORD;
		ULONG view_id__ = VIEW_NULL;

		view_id__ = cmd_view_get_parent_view_id(vty->view_id);

		if (VIEW_NULL == view_id__ || VIEW_USER == view_id__ || VIEW_GLOBAL == view_id__)
		{
			vty->view_id = view_id_;
			break;
		}

		vty->view_id = view_id__;
		view_id_pre = vty->view_id;
		CMD_debug(CMD_DEBUG_FUNC, "Return back to view '%s'.", cmd_view_get_name(vty->view_id));

		ulMatchType = cmd_execute_command(v, vty, pstCmdElem, &ulMatchNum_, &ulNoMatchPos_);
	}

	CMD_debug(CMD_DEBUG_FUNC, "Execute command '%s' on view '%s'. (result:%u, username=%s, vtyId=%u)",
			  vty->szBuffer, cmd_view_get_name(view_id_pre), ulMatchType, vty->user.user_name, vty->vtyId);

	write_log(0, "Execute command '%s' on view '%s'. (result:%u, username=%s, vtyId=%u)",
		vty->szBuffer, cmd_view_get_name(view_id_pre), ulMatchType, vty->user.user_name, vty->vtyId);

	/* add to history cmd */
	cmd_vty_add_history(vty);

	if (ulMatchType == CMD_NO_MATCH)
	{
		cmd_output_missmatch(vty, ulNoMatchPos);
	}

	if (ulMatchType == CMD_ERR_AMBIGOUS)
	{
		if (ulMatchNum)
		{
			cmd_vty_printf(vty, "Command '%s' anbigous follow:%s", vty->szBuffer, CMD_ENTER);

			for (i = 0; i < ulMatchNum; i++)
			{
				if (ANOTHER_LINE(i))
				{
					cmd_vty_printf(vty, "%s", CMD_ENTER);
				}

				cmd_vty_printf(vty, " %-25s", pstCmdElem[i]->szElmtName);
			}

			cmd_vty_printf(vty, "%s", CMD_ENTER);
			/* del 10-29
			cmd_outprompt(vty);
			cmd_vty_printf(vty, "%s", vty->buffer);
			*/
			vty->ulCurrentPos = vty->ulUsedLen = 0;
			memset(vty->szBuffer, 0, vty->ulBufMaxLen);
			cmd_outprompt(vty);
		}
	}
	else
	{
		vty->ulCurrentPos = vty->ulUsedLen = 0;
		memset(vty->szBuffer, 0, vty->ulBufMaxLen);
		cmd_outprompt(vty);
	}
}

VOID cmd_handle_quest(CMD_VTY_S *vty)
{
	CMD_VECTOR_S *v = NULL;
	CMD_ELMT_S *pstCmdElem = NULL;
	ULONG ulMatchNum = 0;
	ULONG i = 0;
	ULONG ulNoMatchPos = CMD_NULL_DWORD;
	BOOL bShowHelphead = FALSE;

	/* BEGIN: Added by weizengke, 2013/10/4   PN:need print '?' */
	cmd_put_one(vty, '?');
	/* END:   Added by weizengke, 2013/10/4   PN:need print '?' */

	if (strlen(vty->szBuffer) < vty->ulCurrentPos)
	{
		CMD_DBGASSERT(0, "cmd_resolve_quest. (buffer=%s[%u], ulCurrentPos=%u, ulUsedLen=%u, ulBufMaxLen=%u)",
					  vty->szBuffer, strlen(vty->szBuffer), vty->ulCurrentPos, vty->ulUsedLen, vty->ulBufMaxLen);
		return;
	}

	/*
	1: 取pos 之前的buf
	2: 需要覆盖当前光标后的buf
	*/
	/* BEGIN: Added by weizengke, 2013/11/17 bug for left and tab*/
	memset(&(vty->szBuffer[vty->ulCurrentPos]), 0, strlen(vty->szBuffer) - vty->ulCurrentPos);
	vty->ulUsedLen = strlen(vty->szBuffer);
	/* END:   Added by weizengke, 2013/11/17 */

	v = cmd_str2vec(vty->szBuffer);
	if (v == NULL)
	{
		/* 没有输入时，提示当前视图 */
		bShowHelphead = TRUE;

		v = cmd_vector_init();
		if (NULL == v)
		{
			return;
		}

		cmd_vector_insert_cr(v);
	}
	else if (' ' == vty->szBuffer[vty->ulUsedLen - 1])
	{
		cmd_vector_insert_cr(v);
	}

	pstCmdElem = (CMD_ELMT_S *)malloc(CMD_MAX_MATCH_SIZE * sizeof(CMD_ELMT_S));
	if (NULL == pstCmdElem)
	{
		return;
	}
	memset(pstCmdElem, 0, CMD_MAX_MATCH_SIZE * sizeof(CMD_ELMT_S));

	cmd_complete_command(v, vty, pstCmdElem, &ulMatchNum, &ulNoMatchPos);

	cmd_vty_printf(vty, "%s", CMD_ENTER);
	if (ulMatchNum > 0)
	{
		/* 没有输入时，提示当前视图 */
		if (TRUE == bShowHelphead)
		{
			cmd_vty_printf(vty, "%s commands:\r\n", cmd_view_get_name(vty->view_id));
		}

		for (i = 0; i < ulMatchNum; i++)
		{
			cmd_vty_printf(vty, " %-25s%s\r\n", pstCmdElem[i].szElmtName, pstCmdElem[i].szElmtHelp);
		}

		cmd_outprompt(vty);
		cmd_vty_printf(vty, "%s", vty->szBuffer);
	}
	else
	{
		cmd_output_missmatch(vty, ulNoMatchPos);
		cmd_outprompt(vty);
		cmd_vty_printf(vty, "%s", vty->szBuffer);
	}

	cmd_vector_deinit(v, 0);

	free(pstCmdElem);

	CMD_debug(CMD_DEBUG_FUNC, "cmd_resolve_quest. (ulMatchNum=%u, ulNoMatchPos=%u)", ulMatchNum, ulNoMatchPos);

	return;
}

/* bug of up twice with last key is not up, the hpos not restart */
VOID cmd_handle_up(CMD_VTY_S *vty)
{
	CHAR *history = cmd_vty_get_prev_history(vty);
	
	if (history == NULL) {
		return;
	}

	cmd_clear_line(vty);
	vty->ulCurrentPos = vty->ulUsedLen = strlen(history);
	strcpy(vty->szBuffer, history);
	cmd_vty_printf(vty, "%s", vty->szBuffer);

	return;
}

VOID cmd_handle_down(CMD_VTY_S *vty)
{
	CHAR *history = cmd_vty_get_next_history(vty);
	
	if (history == NULL) {
		return;
	}

	cmd_clear_line(vty);
	vty->ulCurrentPos = vty->ulUsedLen = strlen(history);
	strcpy(vty->szBuffer, history);
	cmd_vty_printf(vty, "%s", vty->szBuffer);

	return;
}

VOID cmd_handle_left(CMD_VTY_S *vty)
{
	if (vty->ulCurrentPos <= 0)
	{
		return;
	}

	vty->ulCurrentPos--;
	cmd_back_one(vty);

	return;
}

VOID cmd_handle_right(CMD_VTY_S *vty)
{
	if (vty->ulCurrentPos >= vty->ulUsedLen)
	{
		return;
	}

	cmd_put_one(vty, vty->szBuffer[vty->ulCurrentPos]);
	vty->ulCurrentPos++;

	return;
}

VOID cmd_handle_delete(CMD_VTY_S *vty)
{
	ULONG i = 0;
	ULONG size = 0;

	if (vty->ulCurrentPos >= vty->ulUsedLen)
	{
		return;
	}

	cmd_delete_one(vty);

	size = vty->ulUsedLen - vty->ulCurrentPos;

	CMD_debug(CMD_DEBUG_FUNC, "cmd_resolve_delete. (ulUsedLen=%u, ulCurrentPos=%u)",
			  vty->ulUsedLen, vty->ulCurrentPos);

	CMD_DBGASSERT((vty->ulUsedLen >= vty->ulCurrentPos),
				  "cmd_resolve_delete. (ulUsedLen=%u, ulCurrentPos=%u)",
				  vty->ulUsedLen, vty->ulCurrentPos);

	memcpy(&vty->szBuffer[vty->ulCurrentPos], &vty->szBuffer[vty->ulCurrentPos + 1], size);

	vty->szBuffer[vty->ulUsedLen - 1] = '\0';

	for (i = 0; i < size; i++)
	{
		cmd_put_one(vty, vty->szBuffer[vty->ulCurrentPos + i]);
	}

	vty->ulUsedLen--;

	for (i = 0; i < size; i++)
	{
		cmd_back_one(vty);
	}

	return;
}

VOID cmd_handle_backspace(CMD_VTY_S *vty)
{
	ULONG i = 0;
	ULONG size = 0;

	if (vty->ulCurrentPos <= 0)
	{
		return;
	}

	size = vty->ulUsedLen - vty->ulCurrentPos;

	CMD_debug(CMD_DEBUG_FUNC, "cmd_handle_backspace. (ulUsedLen=%u, ulCurrentPos=%u)",
			  vty->ulUsedLen, vty->ulCurrentPos);

	CMD_DBGASSERT((vty->ulUsedLen >= vty->ulCurrentPos),
				  "cmd_handle_backspace. (ulUsedLen=%u, ulCurrentPos=%u)",
				  vty->ulUsedLen, vty->ulCurrentPos);

	vty->ulCurrentPos--;
	vty->ulUsedLen--;
	cmd_back_one(vty);

	memcpy(&vty->szBuffer[vty->ulCurrentPos], &vty->szBuffer[vty->ulCurrentPos + 1], size);
	vty->szBuffer[vty->ulUsedLen] = '\0';

	for (i = 0; i < size; i++)
	{
		cmd_put_one(vty, vty->szBuffer[vty->ulCurrentPos + i]);
	}

	cmd_put_one(vty, ' ');

	for (i = 0; i < size + 1; i++)
	{
		cmd_back_one(vty);
	}

	return;
}

VOID cmd_handle_insert(CMD_VTY_S *vty)
{
	ULONG i = 0;
	ULONG size = 0;

	if (vty->ulUsedLen >= vty->ulBufMaxLen)
	{
		CMD_debug(CMD_DEBUG_FUNC, "cmd_handle_insert, used=%u, vtyId=%uulUsedLen(%u)>=ulBufMaxLen(%u, ulCurrentPos=%u)",
				  vty->used, vty->vtyId, vty->ulUsedLen, vty->ulBufMaxLen, vty->ulCurrentPos);

		return;
	}

	if (vty->ulUsedLen < vty->ulCurrentPos)
	{
		CMD_DBGASSERT(0, "cmd_handle_insert. (ulUsedLen=%u, ulCurrentPos=%u)", vty->ulUsedLen, vty->ulCurrentPos);
		return;
	}

	size = vty->ulUsedLen - vty->ulCurrentPos;
	memcpy(&vty->szBuffer[vty->ulCurrentPos + 1], &vty->szBuffer[vty->ulCurrentPos], size);
	vty->szBuffer[vty->ulCurrentPos] = vty->c;

#if 0
	/* BEGIN: del by weizengke, 2013/11/17 */
	/* BEGIN: Added by weizengke, 2013/10/4   PN:bug for continue tab */
	vty->szBuffer[vty->ulCurrentPos + 1] = '\0';
	/* END:   Added by weizengke, 2013/10/4   PN:None */
	/* END: del by weizengke, 2013/11/17 */
#endif

	for (i = 0; i < size + 1; i++)
	{
		cmd_put_one(vty, vty->szBuffer[vty->ulCurrentPos + i]);
	}

	for (i = 0; i < size; i++)
	{
		cmd_back_one(vty);
	}

	vty->ulCurrentPos++;
	vty->ulUsedLen++;

	return;
}

/* 适配CTRL_W键，快速删除最后一个命令字 */
VOID cmd_hanlde_del_lastword(CMD_VTY_S *vty)
{
	ULONG i = 0;
	ULONG size = 0;

	CMD_debug(CMD_DEBUG_FUNC, "cmd_hanlde_del_lastword, ulCurrentPos=%d", vty->ulCurrentPos);

	if (vty->ulCurrentPos <= 0)
	{
		return;
	}

	cmd_delete_word_ctrl_W_ex(vty);

	cmd_vty_printf(vty, "%s", CMD_ENTER);
	cmd_outprompt(vty);
	cmd_vty_printf(vty, "%s", vty->szBuffer);

	/* BEGIN: Added by weizengke, 2014/3/23 support delete word form cur_pos*/
	for (i = 0; i < strlen(vty->szBuffer) - vty->ulCurrentPos; i++)
	{
		cmd_back_one(vty);
	}
	/* END:   Added by weizengke, 2014/3/23 */

	return;
}

VOID cmd_fsm(CMD_VTY_S *vty)
{
	if (NULL == vty) {
		return;
	}

	while ((vty->c = vty_getchar(vty)) > 0) {
		/* check vty is online */
		if (0 == vty->used) {
			break;
		}

		vty->user.lastAccessTime = time(NULL);
		vty->ucKeyTypeNow = cmd_parse(vty);

		if (vty->ucKeyTypeNow <= CMD_KEY_CODE_NONE || vty->ucKeyTypeNow >= CMD_KEY_CODE_MAX) {
			CMD_debug(CMD_DEBUG_ERROR, "Unknow key type, c=%c, ucKeyType=%u\n", vty->c, vty->ucKeyTypeNow);
			continue;
		}

		g_cmd_key_handle[vty->ucKeyTypeNow].pKeyCallbackfunc(vty);

		vty->ucKeyTypePre = vty->ucKeyTypeNow;

		/* not key type TAB, clear tabString */
		if (vty->ucKeyTypeNow != CMD_KEY_CODE_TAB) {
			memset(vty->tabString, 0, sizeof(vty->tabString));
			memset(vty->tabbingString, 0, sizeof(vty->tabbingString));
		}

		/* for debug */
		CMD_DBGASSERT(!(vty->ulBufMaxLen != CMD_BUFFER_SIZE
					  || vty->ulUsedLen > CMD_BUFFER_SIZE
					  || vty->ulCurrentPos > CMD_BUFFER_SIZE), 
					  "szBuffer=%s, ulUsedLen=%u, ulCurrentPos=%u, ulBufMaxLen=%u",
					  vty->szBuffer, vty->ulUsedLen, vty->ulCurrentPos, vty->ulBufMaxLen);
	}

	return;
}

int cmd_init()
{
	g_cmd_vec = cmd_vector_init();
	if (NULL == g_cmd_vec) {
		return CMD_ERR;
	}

	cmd_view_init();

	cmd_vty_console_init();
	cmd_vty_cfm_init();
	cmd_vty_telnet_init();

	return CMD_OK;
}

int cmd_main_entry(void *pEntry)
{
    extern ULONG SYSMNG_IsCfgRecoverOver();

	for (;;) {
		/* cmd run after cfg recover over */
		if (OS_NO == SYSMNG_IsCfgRecoverOver()) {
			Sleep(10);
			continue;
		}

		vty_go(CMD_VTY_CONSOLE_ID);
	}

	return 0;
}

#endif
