/*
	Code source reference from Zebra.

	Author: Jungle Wei
	Date  : 2013-10
	Description: A common command line module

	支持特性: TAB自动选择性补全, '?'联想 , CTRL+W 快速删除, 错误位置提示, 命令行带参数


				 如梦令·听

				枕头微冷微倾，
				两眼半张半醒。
				侧卧怎难寝，
				声轻太过安静？
				你听，
				你听，
				雨打风吹未停。

*/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>

#ifdef _LINUX_
#include <curses.h>
#include <termios.h>
#include <unistd.h>
#else
#include <conio.h>
#include <io.h>
#endif

#include "osp\command\include\icli.h"
#include "osp\command\include\command_def.h"
#include "osp\command\include\command_type.h"
#include "osp\command\include\command_core.h"

#include "product\include\pdt_common_inc.h"
#include "osp\debug\include\debug_center_inc.h"

CHAR g_CmdSysname[CMD_SYSNAME_SIZE] = "system";
CMD_VTY_S g_vty[CMD_VTY_MAXUSER_NUM]; /*telnet用户 */
CMD_VTY_S *g_con_vty; /* 串口用户 */
CMD_VTY_S *g_cfm_vty; /* 配置管理用户 */

CMD_VECTOR_S *cmd_vec;     /* 存储全局命令行向量 */
view_node_st *view_list;   /* 存储视图树 */
struct CMD_RUN_Ntf_Node *g_pstCMDRunNtfList = NULL;  /* 存储命令行处理注册钩子 */
CMD_VECTOR_S *elem_vector = NULL;  /* 定义命令字使用 */
CMD_ELEM_S g_elem_cr = {CMD_ELEM_TYPE_END, CMD_ELEM_ID_CR, CMD_END, ""}; /* 命令行结束符 */

key_handler_t key_resolver[] = {
	// resolve a whole line
	{ CMD_KEY_CODE_FILTER, 	cmd_resolve_filter },
	{ CMD_KEY_CODE_TAB, 	cmd_resolve_tab },
	{ CMD_KEY_CODE_ENTER, 	cmd_resolve_enter },
	{ CMD_KEY_CODE_QUEST, 	cmd_resolve_quest },
	{ CMD_KEY_CODE_UP, 		cmd_resolve_up },
	{ CMD_KEY_CODE_DOWN, 	cmd_resolve_down },
	// resolve in read buffer
	{ CMD_KEY_CODE_LEFT, 	cmd_resolve_left },
	{ CMD_KEY_CODE_RIGHT, 	cmd_resolve_right },
	{ CMD_KEY_CODE_DELETE,   cmd_resolve_delete },
	{ CMD_KEY_CODE_BACKSPACE, 	cmd_resolve_backspace },
	{ CMD_KEY_CODE_DEL_LASTWORD, cmd_resolve_del_lastword},

	{ CMD_KEY_CODE_NOTCARE, cmd_resolve_insert },

};

#define CMD_debug(x, args...) debugcenter_print(MID_CMD, x, args)

CHAR *cmd_get_sysname()
{
	extern char *PDT_GetSysname();
	
	return PDT_GetSysname();
}

ULONG cmd_regcallback(ULONG mId, ULONG (*pfCallBackFunc)(VOID *pRcvMsg))
{
	struct CMD_RUN_Ntf_Node * pstNow = NULL;
	struct CMD_RUN_Ntf_Node * pstPre = NULL;
	struct CMD_RUN_Ntf_Node * pstEvtNtfNodeNew = NULL;

	pstEvtNtfNodeNew = (struct CMD_RUN_Ntf_Node *)malloc(sizeof(struct CMD_RUN_Ntf_Node));
	if (NULL == pstEvtNtfNodeNew)
	{
		return CMD_ERR;
	}

	pstEvtNtfNodeNew->mId = mId;
	pstEvtNtfNodeNew->pfCallBackFunc = pfCallBackFunc;

	if (NULL == g_pstCMDRunNtfList)
	{
		g_pstCMDRunNtfList = pstEvtNtfNodeNew;
		pstEvtNtfNodeNew->pNext = NULL;
		return CMD_OK;
	}

	pstNow = g_pstCMDRunNtfList;
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
			g_pstCMDRunNtfList = pstEvtNtfNodeNew;
		}
		else
		{
			pstPre->pNext = pstEvtNtfNodeNew;
		}
	}

	return CMD_OK;
}


ULONG cmd_get_vty_id(VOID *pRunMsg)
{
	CMD_RUNMSG_S *pstRunMsg = (CMD_RUNMSG_S *)pRunMsg;

	if (NULL == pstRunMsg)
	{
		NULL;
	}

	return pstRunMsg->vtyId;
}

VOID *cmd_get_elem_by_index(VOID *pRunMsg, ULONG index)
{
	CMD_RUNMSG_S *pstRunMsg = (CMD_RUNMSG_S *)pRunMsg;

	if (NULL == pstRunMsg)
	{
		NULL;
	}

	return (void*)(pstRunMsg->argv + index);
}


ULONG cmd_get_elem_num(VOID *pRunMsg)
{
	CMD_RUNMSG_S *pstRunMsg = (CMD_RUNMSG_S *)pRunMsg;

	if (NULL == pstRunMsg)
	{
		NULL;
	}

	return pstRunMsg->argc;
}

ULONG cmd_get_first_elem_tblid(VOID *pRunMsg)
{
	CMD_RUNMSG_S *pstRunMsg = (CMD_RUNMSG_S *)pRunMsg;
	CMD_RUNMSG_ELEM_S *pElem = NULL;

	if (NULL == pstRunMsg)
	{
		return CMD_ELEMID_NULL;
	}

	pElem = pstRunMsg->argv;

	return ((0x00FF0000 & pElem->cmd_elemId) >> 16);

}

ULONG cmd_get_elemid(VOID *pElemMsg)
{
	CMD_RUNMSG_ELEM_S *pElem = (CMD_RUNMSG_ELEM_S *)pElemMsg;

	if (NULL == pElem)
	{
		0;
	}

	return pElem->cmd_elemId;
}

CHAR* cmd_get_elem_param(VOID *pElemMsg)
{
	CMD_RUNMSG_ELEM_S *pElem = (CMD_RUNMSG_ELEM_S *)pElemMsg;

	if (NULL == pElem)
	{
		0;
	}

	return pElem->cmd_param;
}

ULONG cmd_get_ulong_param(VOID *pElemMsg)
{
	CMD_RUNMSG_ELEM_S *pElem = (CMD_RUNMSG_ELEM_S *)pElemMsg;

	if (NULL == pElem)
	{
		0;
	}

	return atol(pElem->cmd_param);
}

VOID cmd_copy_string_param(VOID *pElemMsg, CHAR *param)
{
	CMD_RUNMSG_ELEM_S *pElem = (CMD_RUNMSG_ELEM_S *)pElemMsg;

	if (NULL == pElem)
	{
		0;
	}

	strcpy(param, pElem->cmd_param);

	return ;
}

ULONG cmd_run_notify(struct cmd_elem_st * cmd, struct cmd_vty *vty, CHAR **argv, ULONG argc)
{
	ULONG index  = 0;
	ULONG isize = 0;
	ULONG ret = CMD_OK;
	ULONG argcNum = 0;
	CMD_RUNMSG_S *pstRunMsg = NULL;
	CMD_RUNMSG_ELEM_S *pstRunMsgElem = NULL;
	struct CMD_RUN_Ntf_Node * pstHead =  g_pstCMDRunNtfList;

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
	pstRunMsgElem   = (CMD_RUNMSG_ELEM_S *)(pstRunMsg + 1);

	for (ULONG i = 0; i < argc; i++)
	{
		CMD_ELEM_S * cmd_elem = (CMD_ELEM_S *)cmd_vector_slot(cmd->para_vec, i);

		CMD_debug(DEBUG_TYPE_FUNC,
			"cmd_run_notify: 0x%x, %d, %s, %s, %s, %d",
			cmd_elem->elem_id, cmd_elem->elem_tpye, cmd_elem->para, cmd_elem->desc, argv[i], argc);

		if (CMD_ELEMID_NULL == cmd_elem->elem_id)
		{
			continue;
		}

 		pstRunMsgElem->cmd_elemId = cmd_elem->elem_id;
		if (strlen(argv[i]) >= 128)
		{
			free(pstRunMsg);
			return CMD_ERR;
		}

		strcpy(pstRunMsgElem->cmd_param, argv[i]);
		pstRunMsgElem++;
		argcNum++;

	}

	pstRunMsg->argc = argcNum;

	CMD_debug(CMD_DEBUG_FUNC, "cmd_run_notify, argcNum=%u", argcNum);

	while (NULL != pstHead)
	{
		if (cmd->mid != pstHead->mId)
		{
			pstHead = pstHead->pNext;
			continue;
		}

		ret = pstHead->pfCallBackFunc((VOID*)pstRunMsg);

		break;
	}

	free(pstRunMsg);

	return ret;
}

VOID vty_printf(ULONG vtyId, CHAR *format, ...)
{	
	ULONG ret = 0;
	char buffer[BUFSIZE] = {0};
	struct cmd_vty *vty = NULL;

	vty = cmd_vty_getById(vtyId);
	if (NULL == vty)
	{
		return;
	}
		
	va_list args;
	va_start(args, format);

	if (CMD_VTY_CONSOLE_ID == vtyId)
	{
		vprintf(format, args);
	}
	else
	{
		vsnprintf(buffer, BUFSIZE, format, args);
		send(vty->user.socket, buffer, strlen(buffer),0);	
	}
	
	va_end(args);

	return;
}

VOID cmd_vty_printf(struct cmd_vty *vty, CHAR *format, ...)
{	
	ULONG ret = 0;
	char buffer[BUFSIZE] = {0};

	va_list args;
	va_start(args, format);

	if (CMD_VTY_CONSOLE_ID == vty->vtyId)
	{
		vprintf(format, args);
	}
	else
	{
		vsnprintf(buffer, BUFSIZE, format, args);
		send(vty->user.socket, buffer, strlen(buffer),0);		
	}
	
	va_end(args);

	return;
}

VOID vty_print2all(CHAR *format, ...)
{
	ULONG ret = 0;
	char buffer[BUFSIZE] = {0};
	
	va_list args;
	va_start(args, format);

	/* com */
	vprintf(format, args);

	/* vty */
	vsnprintf(buffer, BUFSIZE, format, args);
	for (int i = 0; i < CMD_VTY_MAXUSER_NUM; i++)
	{
		if (g_vty[i].used
			&& g_vty[i].user.state
			&& g_vty[i].user.terminal_debugging)
		{
			send(g_vty[i].user.socket, buffer, strlen(buffer),0);
		}
	}
	
	va_end(args);
}


VOID cmd_delete_one(struct cmd_vty *vty)
{
	if (CMD_VTY_CONSOLE_ID == vty->vtyId)
	{
		cmd_vty_printf(vty, " \b");
	}
	else
	{
		cmd_vty_printf(vty, "  \b");
	}
}

VOID cmd_back_one(struct cmd_vty *vty)
{
	cmd_vty_printf(vty, "\b");
}

VOID cmd_put_one(struct cmd_vty *vty, CHAR c)
{
	cmd_vty_printf(vty, "%c", c);
}

ULONG cmd_getch()
{
	ULONG c = 0;
	#ifdef _LINUX_
	struct termios org_opts, new_opts;
	ULONG res = 0;
	//-----  store old settings -----------
	res = tcgetattr(STDIN_FILENO, &org_opts);
	assert(res == 0);
	//---- set new terminal parms --------
	memcpy(&new_opts, &org_opts, sizeof(new_opts));
	new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
	tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
	c = getchar();
	//------  restore old settings ---------
	res = tcsetattr(STDIN_FILENO, TCSANOW, &org_opts);
	assert(res == 0);
	#else
	c = getch();
	#endif

	return c;
}

int vty_getch(struct cmd_vty *vty)
{
	int ret = 0;
	CHAR buff[1] = {0};

	/* 串口模式 */
	if (CMD_VTY_CONSOLE_ID == vty->vtyId)
	{
		buff[0] = cmd_getch();
	}
	else
	{	
		/* telnet模式 */
		ret = recv(vty->user.socket, (char*)buff, 1, 0);

		if (ret <= 0)
		{
			return 0;
		}
	}

	//CMD_debug(CMD_DEBUG_INFO, "vty_getch. (c=0x%x)", buff[0]);
	
	return buff[0];
}


/*****************************************************************************
 Prototype    : cmd_elem_is_para_type
 Description  : 检查命令元素是否是一个参数
 Input        : CMD_ELEM_TYPE_E type
 Output       : None
 Return Value :
 Calls        :
 Called By    :

  History        :
  1.Date         : 2013/10/5
    Author       : weizengke
    Modification : Created function

*****************************************************************************/
ULONG cmd_elem_is_para_type(CMD_ELEM_TYPE_E type)
{
	switch(type)
	{
		case CMD_ELEM_TYPE_INTEGER:
		case CMD_ELEM_TYPE_STRING:
			return CMD_YES;
		default:
			break;
	}

	return CMD_NO;
}

/* vty */
struct cmd_vty *cmd_vty_getById(ULONG vtyId)
{	
	/* 串口 */
	if (CMD_VTY_CONSOLE_ID == vtyId)
	{
		return g_con_vty;
	}
	/* 配置管理 */
	else if (CMD_VTY_CFM_ID == vtyId)
	{
		return g_cfm_vty;
	}
	else
	{
		if (vtyId >= CMD_VTY_MAXUSER_NUM)
		{
			return NULL;
		}
		
		return &g_vty[vtyId];
	}	
}

VOID cmd_vty_init(struct cmd_vty *vty)
{
	vty->vtyId = vty->vtyId;
	vty->used = 0;
	vty->view_id = VIEW_USER;
	vty->buf_len = CMD_BUFFER_SIZE;
	vty->used_len = vty->cur_pos = 0;
	vty->hpos = vty->hindex = 0;

	vty->inputMachine_prev = CMD_KEY_CODE_NOTCARE;
	vty->inputMachine_now = CMD_KEY_CODE_NOTCARE;
	memset(vty->tabbingString, 0, CMD_MAX_CMD_ELEM_SIZE);
	memset(vty->tabString, 0, CMD_MAX_CMD_ELEM_SIZE);
	vty->tabStringLenth = 0;

	vty->user.socket = INVALID_SOCKET;
	vty->user.state = 0;
	vty->user.terminal_debugging = 0;
	memset(vty->user.user_name, 0, sizeof(vty->user.user_name));
	//vty->user.lastAccessTime = time(NULL);

	return ;
}

VOID cmd_vty_deinit(struct cmd_vty *vty)
{
	ULONG i;

	if (!vty)
	{
		return;
	}

	for (i = 0; i < HISTORY_MAX_SIZE; i++)
	{
		if (vty->history[i] != NULL)
		{
			free(vty->history[i]);
			vty->history[i] = NULL;
		}
	}

	free(vty);
}

VOID cmd_vty_add_history(struct cmd_vty *vty)
{
	ULONG idx =  vty->hindex ? vty->hindex - 1 : HISTORY_MAX_SIZE - 1;

	/* if same as previous command, then ignore */
	if (vty->history[idx] &&
		!strcmp(vty->buffer, vty->history[idx]))
	{
		vty->hpos = vty->hindex;
		return;
	}

	/* insert into history tail */
	if (vty->history[vty->hindex])
	{
		free(vty->history[vty->hindex]);
	}

	vty->history[vty->hindex] = strdup(vty->buffer);
	vty->hindex = (vty->hindex + 1) == HISTORY_MAX_SIZE ? 0 : vty->hindex + 1;
	vty->hpos = vty->hindex;

}

/* vector */
ULONG cmd_vector_fetch(CMD_VECTOR_S *v)
{
	ULONG fetch_idx;

	for (fetch_idx = 0; fetch_idx < v->used_size; fetch_idx++)
	{
		if (v->data[fetch_idx] == NULL)
		{
			break;
		}
	}

	while (v->size < fetch_idx + 1)
	{
		v->data = (void**)realloc(v->data, sizeof(void *) * v->size * 2);
		if (!v->data)
		{
			CMD_debug(CMD_DEBUG_ERROR, "In cmd_vector_fetch, Not Enough Memory For data");
			return -1;
		}

		memset(&v->data[v->size], 0, sizeof(void *) * v->size);
		v->size *= 2;
	}

	return fetch_idx;
}

CMD_VECTOR_S *cmd_vector_init(ULONG size)
{
	CMD_VECTOR_S *v = (CMD_VECTOR_S *)calloc(1, sizeof(CMD_VECTOR_S));
	if (v == NULL)
	{
		return NULL;
	}

	if (size == 0)
	{
		size = 1;
	}

	v->data = (void**)calloc(1, sizeof(void *) * size);
	if (v->data == NULL)
	{
		CMD_debug(CMD_DEBUG_ERROR, "In cmd_vector_init, Not Enough Memory For data");
		free(v);
		return NULL;
	}

	v->used_size = 0;
	v->size = size;

	return v;
}

VOID cmd_vector_deinit(CMD_VECTOR_S *v, ULONG freeall)
{
	if (v == NULL)
	{
		return;
	}

	if (v->data)
	{
		if (freeall)
		{
			ULONG i;
			for (i = 0; i < cmd_vector_max(v); i++)
			{
				if (cmd_vector_slot(v, i))
				{
					free(cmd_vector_slot(v, i));
				}
			}
		}

		free(v->data);
	}

	free(v);
}

CMD_VECTOR_S *cmd_vector_copy(CMD_VECTOR_S *v)
{
	ULONG size;
	CMD_VECTOR_S *new_v = (CMD_VECTOR_S *)calloc(1, sizeof(CMD_VECTOR_S));
	if (NULL == new_v)
	{
		CMD_DBGASSERT(0, "cmd_vector_copy");
		return NULL;
	}

	new_v->used_size = v->used_size;
	new_v->size = v->size;

	size = sizeof(void *) * (v->size);
	new_v->data = (void**)calloc(1, sizeof(void *) * size);
	memcpy(new_v->data, v->data, size);

	return new_v;
}

VOID cmd_vector_insert(CMD_VECTOR_S *v, VOID *val)
{
	ULONG idx;

	idx = cmd_vector_fetch(v);
	v->data[idx] = val;
	if (v->used_size <= idx)
	{
		v->used_size = idx + 1;
	}
}

/*
	仅用于用户输入末尾补<CR>，保存在cmd_vector_t->data
*/
VOID cmd_vector_insert_cr(CMD_VECTOR_S *v)
{
	CHAR *string_cr = NULL;
	string_cr = (char*)malloc(sizeof(CMD_END));
	if (NULL == string_cr)
	{
		CMD_DBGASSERT(0,"cmd_vector_insert_cr");
		return;
	}

	memcpy(string_cr, CMD_END, sizeof(CMD_END));
	cmd_vector_insert(v, string_cr); /* cmd_vector_insert(v, CMD_END); // bug of memory free(<CR>), it's static memory*/
}

CMD_VECTOR_S *cmd_vector_new()
{
	return cmd_vector_init(1);
}

VOID cmd_vector_free(CMD_VECTOR_S **pVec)
{
	cmd_vector_deinit(*pVec, 1);
	*pVec = NULL;
}

CMD_VECTOR_S *cmd_str2vec(CHAR *string)
{
	ULONG str_len;
	CHAR *cur, *start, *token;
	CMD_VECTOR_S *vec;

	// empty string
	if (string == NULL)
	{
		return NULL;
	}

	cur = string;
	// skip white spaces
	while (isspace((int) *cur) && *cur != '\0')
	{
		cur++;
	}

	// only white spaces
	if (*cur == '\0')
	{
		return NULL;
	}

	// copy each command pieces into vector
	vec = cmd_vector_init(1);
	while (1)
	{
		start = cur;
		while (!(isspace((int) *cur) || *cur == '\r' || *cur == '\n') &&
				*cur != '\0')
		{
			cur++;
		}

		str_len = cur - start;
		token = (char *)malloc(sizeof(char) * (str_len + 1));
		if (NULL == token)
		{
			CMD_debug(CMD_DEBUG_ERROR, "In cmd_str2vec, There is no memory for param token.");
			return NULL;
		}

		memcpy(token, start, str_len);
		*(token + str_len) = '\0';
		cmd_vector_insert(vec, (void *)token);

		while((isspace ((int) *cur) || *cur == '\n' || *cur == '\r') &&
			*cur != '\0')
		{
			cur++;
		}

		if (*cur == '\0')
		{
			return vec;
		}

	}
}

CMD_VECTOR_S *cmd_cmd2vec(CMD_VECTOR_S * pVec, ULONG *pCmdLine, ULONG n)
{
	CMD_VECTOR_S *vec=NULL;
	struct para_desc *desc = NULL;
	
	if(NULL == pVec
		|| NULL == pCmdLine)
	{
		return NULL;
	}

	vec = cmd_vector_init(1);

	for (ULONG i = 0; i < n; i++)
	{	
		if (0 == pCmdLine[i])
		{
			CMD_DBGASSERT(0, "cmd cmd2vec, cmdline index cannot be 0.");
			return NULL;
		}

		if (pCmdLine[i] - 1 >= cmd_vector_max(pVec))
		{
			CMD_DBGASSERT(0, "cmd cmd2vec, cmdline index(%u) cannot more than %u.", pCmdLine[i], cmd_vector_max(pVec));
			return NULL;
		}
		
		desc = (struct para_desc *)cmd_vector_slot(pVec, pCmdLine[i] - 1);

		CMD_ELEM_S * cmd_elem = NULL;
		cmd_elem = (CMD_ELEM_S *)malloc(sizeof(CMD_ELEM_S));
		if (NULL == cmd_elem)
		{
			return NULL;
		}

		cmd_elem->elem_id = desc->elem_id;
		cmd_elem->elem_tpye = desc->elem_tpye;

		cmd_elem->para = (char *)malloc(strlen(desc->para) + 1);
		if (NULL == cmd_elem->para)
		{
			free(cmd_elem);
			return NULL;
		}
		memset(cmd_elem->para, 0, strlen(desc->para) + 1);
		strcpy(cmd_elem->para, desc->para);

		cmd_elem->desc = (char *)malloc(strlen(desc->desc) + 1);
		if (NULL == cmd_elem->desc)
		{
			free(cmd_elem->para);
			free(cmd_elem);
			return NULL;
		}
		memset(cmd_elem->desc, 0, strlen(desc->desc) + 1);
		strcpy(cmd_elem->desc, desc->desc);

		cmd_vector_insert(vec, (void *)cmd_elem);
	}

	cmd_vector_insert(vec, (void *)&g_elem_cr);

	return vec;
}

/* end vector */


/* cmd */
static ULONG match_unique_string(struct para_desc **match, CHAR *str, ULONG size)
{
	ULONG i;

	for (i = 0; i < size; i++)
	{
		if (match[i]->para != NULL && strcmp(match[i]->para, str) == 0)
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
	while (isspace((int) *cur) && *cur != '\0')
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
		while (!(isspace((int) *cur) || *cur == '\r' || *cur == '\n') &&
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

		while((isspace ((int) *cur) || *cur == '\n' || *cur == '\r') &&
			*cur != '\0')
		{
			pre = cur;
			cur++;
		}

		if (*cur == '\0')
		{
			/* BEGIN: Added by weizengke, 2014/3/9 修复命令行不完全时，错误位置提示不准确的问题 */
			*pos = (int)(cur - string);

			if (!isspace ((int) *pre))
			{
				*pos += 1;
			}

			break;
		}

	}

	return CMD_OK;
}

/*****************************************************************************
*   Prototype    : cmd_output_missmatch
*   Description  : Output miss match command position
*   Input        : cmd_vty *vty
*                  int nomath_pos
*   Output       : None
*   Return Value : void
*   Calls        :
*   Called By    :
*
*   History:
*
*       1.  Date         : 2013/11/19
*           Author       : weizengke
*           Modification : Created function
*
*****************************************************************************/
VOID cmd_output_missmatch(cmd_vty *vty, ULONG nomath_pos)
{
	ULONG i = 0;
	ULONG n = 0;
	ULONG view_len = cmd_view_getaislenth(vty);
	ULONG pos_arrow = strlen(cmd_get_sysname()) + view_len + 1;

	/* 系统名与视图之间有一个链接符- */
	if (view_len > 0)
	{
		pos_arrow += 1;
	}

	(VOID)cmd_get_nth_elem_pos(vty->buffer, nomath_pos, &n);
	pos_arrow += n;

	for (i=0;i<pos_arrow;i++)
	{
		cmd_vty_printf(vty, " ");
	}

	cmd_vty_printf(vty, "^\r\nError: Unrecognized command at '^' position.\r\n");

}

/*  get range form INTEGER<a-b> STRING<a-b>
	type for INTEGER or STRING
*/
ULONG cmd_get_range_symbol(CHAR *string, ULONG *type, ULONG *a, ULONG *b)
{
	CHAR *pleft = NULL;
	CHAR *pright = NULL;
	CHAR *pline = NULL;
	CHAR type_string[256] = {0};
	/*
		请确保string有效性
	*/

	if (string == NULL || a == NULL || b == NULL)
	{
		return CMD_ERR;
	}

	pleft  = strchr(string, '<');
	pline  = strchr(string, '-');
	pright = strchr(string, '>');

	if (pleft == NULL || pline == NULL || pright == NULL)
	{
		return CMD_ERR;
	}

	*a = 0;
	*b = 0;
	*type = CMD_ELEM_TYPE_VALID;

	sscanf(string,"%[A-Z]<%u-%u>", type_string, a, b);

	if (strcmp(type_string, CMD_STRING) == 0)
	{
		*type = CMD_ELEM_TYPE_STRING;
	}

	if (strcmp(type_string, CMD_INTEGER) == 0)
	{
		*type = CMD_ELEM_TYPE_INTEGER;
	}

	CMD_debug(CMD_DEBUG_INFO, "%s<%u-%u>",type_string, *a, *b);

	return CMD_OK;

}

/* 检查字符串是不是只包含数字 */
ULONG cmd_string_isdigit(CHAR *string)
{
	ULONG i = 0;
	if (string == NULL)
	{
		return CMD_ERR;
	}

	/* ULONG最大是4294967295 */
	if (strlen(string) > 10
		|| (strlen(string) == 10 && strcmp(string,"4294967295") > 0))
	{
		return CMD_ERR;
	}

	for (i = 0; i < (int)strlen(string); i++)
	{
		if (!isdigit(*(string + i)))
		{
			CMD_debug(CMD_DEBUG_ERROR, "cmd_string_isdigit return error.");
			return CMD_ERR;
		}
	}

	CMD_debug(CMD_DEBUG_INFO, "cmd_string_isdigit return ok.");
	return CMD_OK;
}

/* match INTEGER or STRING */
ULONG cmd_match_special_string(CHAR *icmd, CHAR *dest)
{
	/*
	num

	string

	ohter

	*/
	ULONG dest_type = CMD_ELEM_TYPE_VALID;
	ULONG a = 0;
	ULONG b = 0;

	if (icmd == NULL || dest == NULL)
	{
		return CMD_ERR;
	}

	if (cmd_get_range_symbol(dest, &dest_type, &a, &b))
	{
		return CMD_ERR;
	}

	switch (dest_type)
	{
		case CMD_ELEM_TYPE_INTEGER:
			if (CMD_OK == cmd_string_isdigit(icmd))
			{
				ULONG icmd_i = atol(icmd);
				if (icmd_i >= a && icmd_i <=b)
				{
					return CMD_OK;
				}
			}
			break;
		case CMD_ELEM_TYPE_STRING:
			{
				ULONG icmd_len = (ULONG)strlen(icmd);
				if (icmd_len >= a && icmd_len <= b)
				{
					return CMD_OK;
				}
			}
			break;
		default:
			break;
	}


	return CMD_ERR;
}

static ULONG cmd_filter_command(struct cmd_vty *vty, CHAR *cmd, CMD_VECTOR_S *v, ULONG index)
{
	ULONG i;
	ULONG match_cmd = CMD_ERR;
	struct cmd_elem_st *elem;
	struct para_desc *desc;

	// For each command, check the 'index'th parameter if it matches the
	// 'index' paramter in command, set command vector which does not
	// match cmd NULL

	/* BEGIN: Added by weizengke, 2013/10/4   PN:check cmd valid*/
	if (cmd == NULL || 0 == strlen(cmd))
	{
		return CMD_ERR;
	}

	/* <CR> 不参与过滤，防止命令行子串也属于命令行时误过滤 */
	if (0 == strcmp(cmd, CMD_END))
	{
		return CMD_OK;
	}

	/* END:   Added by weizengke, 2013/10/4   PN:None */

	for (i = 0; i < cmd_vector_max(v); i++)
	{
		if ((elem = (struct cmd_elem_st*)cmd_vector_slot(v, i)) != NULL)
		{
			if (elem->view_id != VIEW_GLOBAL
				&& elem->view_id != vty->view_id)
			{
				continue;
			}

			if (index >= cmd_vector_max(elem->para_vec))
			{
				cmd_vector_slot(v, i) = NULL;
				continue;
			}

			desc = (struct para_desc *)cmd_vector_slot(elem->para_vec, index);

			/* match STRING , INTEGER */
			if (CMD_OK != cmd_match_special_string(cmd, desc->para))
			{
				if(strnicmp(cmd, desc->para, strlen(cmd)) != 0)
				{
					cmd_vector_slot(v, i) = NULL;
					continue;
				}
			}

			/* BEGIN: Added by weizengke, 2013/11/19 for support unkown cmd pos*/
			match_cmd = CMD_OK;
			/* END:   Added by weizengke, 2013/11/19 */
		}
	}

	CMD_debug(CMD_DEBUG_INFO, "cmd_filter_command. (cmd=%s, match_res=%u)", vty->buffer, match_cmd);

	return match_cmd;
}


ULONG cmd_match_command(CMD_VECTOR_S *icmd_vec, struct cmd_vty *vty,
		struct para_desc **match, ULONG *match_size, CHAR *lcd_str)
{
	ULONG i;
	CMD_VECTOR_S *cmd_vec_copy = cmd_vector_copy(cmd_vec);
	ULONG isize = 0;
	ULONG size = 0;

	CMD_NOUSED(vty);
	CMD_NOUSED(lcd_str);

	// Three steps to find matched commands in 'cmd_vec'
	// 1. for input command vector 'icmd_vec', check if it is matching cmd_vec
	// 2. store last matching command parameter in cmd_vec into match_vec
	// 3. according to match_vec, do no match, one match, part match, list match

	if (icmd_vec == NULL)
	{
		return CMD_NO_MATCH;
	}

	isize = cmd_vector_max(icmd_vec) - 1;

	// Step 1
	for (i = 0; i < isize; i++)
	{
		char *ipara = (char*)cmd_vector_slot(icmd_vec, i);
		cmd_filter_command(vty, ipara, cmd_vec_copy, i);
	}

	// Step 2
	// insert matched command word into match_vec, only insert the next word
	for(i = 0; i < cmd_vector_max(cmd_vec_copy); i++)
	{
		struct cmd_elem_st *elem = NULL;
		elem = (struct cmd_elem_st *)cmd_vector_slot(cmd_vec_copy, i);

		if(elem != NULL)
		{
			if (elem->para_vec == NULL)
			{
 				continue;
 			}

			if (elem->view_id != VIEW_GLOBAL
				&& elem->view_id != vty->view_id)
			{
				continue;
			}

			if (isize >= cmd_vector_max(elem->para_vec))
			{
				cmd_vector_slot(cmd_vec_copy, i) = NULL;
				continue;
			}

			struct para_desc *desc = (struct para_desc *)cmd_vector_slot(elem->para_vec, isize);
			char *str = (char*)cmd_vector_slot(icmd_vec, isize);

			if (str == NULL || strnicmp(str, desc->para, strlen(str)) == 0)
			{
				if (match_unique_string(match, desc->para, size))
					match[size++] = desc;
			}

		}

	}

	cmd_vector_deinit(cmd_vec_copy, 0);	// free cmd_vec_copy, no longer use

	// Step 3
	// No command matched
	if (size == 0)
	{
		*match_size = size;
		return CMD_NO_MATCH;
	}

	// Only one command matched
	if (size == 1)
	{
		*match_size = size;
		return CMD_FULL_MATCH;
	}

	*match_size = size;
	return CMD_LIST_MATCH;
}

/*****************************************************************************
 Prototype    : cmd_complete_command
 Description  : complete command　for ? complete
 Input        : CMD_VECTOR_S *icmd_vec  input cmd vector
                struct cmd_vty *vty     the input vty

 Output       : char **doc              the return doc
 Return Value :
 Calls        :
 Called By    :

  History        :
  1.Date         : 2013/10/4
    Author       : weizengke
    Modification : Created function

*****************************************************************************/
ULONG cmd_complete_command(CMD_VECTOR_S *icmd_vec, struct cmd_vty *vty,
									 struct para_desc **match, ULONG *match_size, ULONG *nomath_pos)
{


	ULONG i;
	CMD_VECTOR_S *cmd_vec_copy = cmd_vector_copy(cmd_vec);
	ULONG match_num = 0;

	char *str;
	struct para_desc *para_desc_;
	struct cmd_elem_st *elem;

	if (icmd_vec == NULL || vty == NULL || match == NULL || match_size == NULL)
	{
		return CMD_ERR;
	}

	// Two steps to find matched commands in 'cmd_vec'
	// 1. for input command vector 'icmd_vec', check if it is matching cmd_vec
	// 2. check last matching command parameter in cmd_vec match cmd_vec

	*nomath_pos = CMD_NULL_DWORD;

	// Step 1
	/* BEGIN: Modified by weizengke, 2013/10/4   PN:循环过滤每一个向量 */
	for (i = 0; i < cmd_vector_max(icmd_vec); i++)
	{
		if (CMD_OK != cmd_filter_command(vty, (char*)cmd_vector_slot(icmd_vec, i), cmd_vec_copy, i))
		{
			/* BEGIN: Added by weizengke, 2013/11/19 这里可以优化，不命中可以不需要再匹配了 */
			/* 保存在第几个命令字无法匹配 */
			*nomath_pos = (*nomath_pos == CMD_NULL_DWORD)?(i):(*nomath_pos);

			CMD_debug(CMD_DEBUG_ERROR, "cmd_complete_command, cannot match at pos %u", *nomath_pos);
			
			return CMD_OK;
		}
	}
	/* END:   Modified by weizengke, 2013/10/4   PN:None */

	// Step 2
	// insert matched command word into match_vec, only insert the next word
	/* BEGIN: Added by weizengke, 2013/10/4   PN:only the last vector we need */
	for(i = 0; i < cmd_vector_max(cmd_vec_copy); i++)
	{
		elem = (struct cmd_elem_st *)cmd_vector_slot(cmd_vec_copy, i);
	
		if(elem  != NULL)
		{
			if (cmd_vector_max(icmd_vec) - 1 >= cmd_vector_max(elem->para_vec))
			{
				cmd_vector_slot(cmd_vec_copy, i) = NULL;
				continue;
			}

			CMD_debug(CMD_DEBUG_FUNC, "cmd_complete_command, view_id=%d.", elem->view_id);

			if (elem->view_id != VIEW_GLOBAL
				&& elem->view_id != vty->view_id)
			{
				continue;
			}

			str = (char*)cmd_vector_slot(icmd_vec, cmd_vector_max(icmd_vec) - 1);
			para_desc_ = (struct para_desc*)cmd_vector_slot(elem->para_vec, cmd_vector_max(icmd_vec) - 1);
			if (para_desc_ == NULL)
			{
				continue;
			}

			/* BEGIN: Added by weizengke, 2013/11/19 */
			/* match STRING , INTEGER */
			if (CMD_OK == cmd_match_special_string(str, para_desc_->para))
			{
				match[match_num++] = para_desc_;

				continue;
			}
			/* END:   Added by weizengke, 2013/11/19 */

			/* match key */
			if (strnicmp(str, CMD_END, strlen(str)) == 0
			    ||(strnicmp(str, para_desc_->para, strlen(str)) == 0))
			{

				if (match_unique_string(match, para_desc_->para, match_num))
				{
					match[match_num++] = para_desc_;
				}
			}

		}
	}

	cmd_vector_deinit(cmd_vec_copy, 0);	// free cmd_vec_copy, no longer use

	CMD_debug(CMD_DEBUG_FUNC, "cmd_complete_command, match_num=%u", match_num);

	/* BEGIN: Added by weizengke, 2013/10/27 sort for ? complete */
	if (match_num > 0)
	{
		int j;
		for (i = 0; i < match_num - 1; i++)
		{
			for (j = i; j < match_num; j++)
			{
				struct para_desc *para_desc__ = NULL;
				if (0 == strnicmp(match[i]->para, CMD_END, strlen(match[i]->para))
					|| ( 1 == stricmp(match[i]->para, match[j]->para)
					&& 0 != strnicmp(match[j]->para, CMD_END, strlen(match[j]->para)))
					)
				{
					para_desc__ = match[i];
					match[i] =  match[j];
					match[j] = para_desc__;
				}
			}
		}
	}
	/* END: Added by weizengke, 2013/10/27 sort for ? complete */

	*match_size = match_num;

	return CMD_OK;
}

ULONG cmd_execute_command(CMD_VECTOR_S *icmd_vec, struct cmd_vty *vty, 
									struct para_desc **match, ULONG *match_size, ULONG *nomatch_pos)
{
	ULONG i;
	CMD_VECTOR_S *cmd_vec_copy = cmd_vector_copy(cmd_vec);
	struct cmd_elem_st *match_elem = NULL;
	ULONG match_num = 0;

	/*
	Two steps to find matched commands in 'cmd_vec'
	 1. for input command vector 'icmd_vec', check if it is matching cmd_vec
	 2. check last matching command parameter in cmd_vec match cmd_vec
	*/

	*nomatch_pos = CMD_NULL_DWORD;

	CMD_debug(CMD_DEBUG_INFO, "cmd_execute_command. (cmd=%s, view_id=%u)", vty->buffer, vty->view_id);

	/* Step 1 */
	for (i = 0; i < cmd_vector_max(icmd_vec); i++)
	{
		if (CMD_OK != cmd_filter_command(vty, (char*)cmd_vector_slot(icmd_vec, i), cmd_vec_copy, i))
		{
			/* BEGIN: Added by weizengke, 2013/11/19 不命中可以不需要再匹配了 */
			/* 保存在第几个命令字无法匹配 */
			*nomatch_pos = (*nomatch_pos == CMD_NULL_DWORD)?(i):(*nomatch_pos);

			CMD_debug(CMD_DEBUG_ERROR, "cmd_execute_command. not match at pos %u.", *nomatch_pos);
			
			return CMD_NO_MATCH;
		}
	}

	/* Step 2 */
	/*  insert matched command word into match_vec, only insert the next word */
	if (*nomatch_pos == CMD_NULL_DWORD)
	{
		/* BEGIN: Added by weizengke, 2014/3/9 修复命令行不完全时，错误位置提示不准确的问题 */
		*nomatch_pos = cmd_vector_max(icmd_vec) - 1;

		for(i = 0; i < cmd_vector_max(cmd_vec_copy); i++)
		{
			char *str;
			struct para_desc *desc;
			struct cmd_elem_st *elem = NULL;

			elem = (struct cmd_elem_st *)cmd_vector_slot(cmd_vec_copy, i);
			if(elem != NULL)
			{
				if (elem->view_id != VIEW_GLOBAL
				&& elem->view_id != vty->view_id)
				{
					continue;
				}

				str = (char*)cmd_vector_slot(icmd_vec, cmd_vector_max(icmd_vec) - 1);
				desc = (struct para_desc *)cmd_vector_slot(elem->para_vec, cmd_vector_max(icmd_vec) - 1);

				/* modified for command without argv */
				if (cmd_vector_max(icmd_vec) == cmd_vector_max(elem->para_vec))
				{
					/* BEGIN: Added by weizengke, 2013/10/5   PN:for support STRING<a-b> & INTEGER<a-b> */
					if (CMD_OK == cmd_match_special_string(str, desc->para) ||
						str != NULL && strnicmp(str, desc->para, strlen(str)) == 0)
					{
						/* BEGIN: Added by weizengke, 2013/10/6   PN:command exec ambigous, return the last elem (not the <CR>) */
						match[match_num] = (struct para_desc *)cmd_vector_slot(elem->para_vec, cmd_vector_max(icmd_vec) - 2);
						/* END:   Added by weizengke, 2013/10/6   PN:None */

						match_num++;
						match_elem = elem;

					}
				}
			}
		}

	}

	*match_size = match_num;

	cmd_vector_deinit(cmd_vec_copy, 0);

	CMD_debug(CMD_DEBUG_INFO, "cmd_execute_command. (cmd=%s, match_num=%u)", vty->buffer, match_num);

	/* check if exactly match */
	if (match_num == 0)
	{
		return CMD_NO_MATCH;
	}

	/* BEGIN: Added by weizengke, 2013/10/6   PN:command exec ambigous */
	if (match_num > 1)
	{
		return CMD_ERR_AMBIGOUS;
	}
	/* END:   Added by weizengke, 2013/10/6   PN:None */

	/* BEGIN: Added by weizengke, 2013/10/5  for support argv. PN:push param into function stack */
	/* now icmd_vec and match_elem will be the same vector ,can push into function stack */
	int argc = 0;
	char *argv[CMD_MAX_CMD_NUM];

	if (NULL == match_elem || match_elem->para_vec == NULL)
	{
		return CMD_NO_MATCH;
	}

	for (i = 0, argc = 0; i < match_elem->para_num; i ++)
	{
		struct para_desc  *desc = (struct para_desc *)cmd_vector_slot(match_elem->para_vec, i);
		if (NULL == desc)
		{
			return CMD_NO_MATCH;
		}

		/* <CR> no need to push */
		if (desc->elem_tpye == CMD_ELEM_TYPE_END)
		{
			continue;
		}

		/* push param to argv and full command*/
		if (CMD_YES == cmd_elem_is_para_type(desc->elem_tpye))
		{
			argv[argc++] = (char*)cmd_vector_slot(icmd_vec, i);
		}
		else
		{
			argv[argc++] = desc->para;
		}
	}
	/* END:   Added by weizengke, 2013/10/5   PN:None */

	/* execute command */
	(VOID)cmd_run_notify(match_elem, vty, argv, argc);

	return CMD_FULL_MATCH;
}


VOID cmd_install_command(ULONG mid, ULONG cmd_view, CHAR *cmd_string, CMD_VECTOR_S * pVec)
{
	ULONG iIndex[128] = {0};
	ULONG iLoop = 0;
	ULONG n = 0;
	CHAR *cur = cmd_string;

	if (NULL == pVec)
	{
		return;
	}
	
	/* 计算线索表达式数组 */
	for (;*cur != '\0';){
		if (*cur >= '0' && *cur <= '9'){
            n *= 10;n += *cur - '0';
        }else if (n > 0){
			iIndex[iLoop++] = n;n = 0;
		}
		cur++;
	}

	if (cmd_vec == NULL)
	{
		CMD_debug(CMD_DEBUG_ERROR, "Command Vector Not Exist");
		return;
	}

	struct cmd_elem_st *elem = NULL;

	elem = (struct cmd_elem_st *)malloc(sizeof(struct cmd_elem_st));
	if (NULL == elem)
	{
		return;
	}
	memset(elem, 0, sizeof(struct cmd_elem_st));

	elem->mid = mid;
	elem->view_id = cmd_view;
	elem->para_vec = cmd_cmd2vec(pVec, iIndex, iLoop);
    if (NULL == elem->para_vec)
    {
		free(elem);
        return;
    }
	elem->para_num = cmd_vector_max(elem->para_vec);

	cmd_vector_insert(cmd_vec, elem);

	return;
}

ULONG cmd_regelement_new(ULONG cmd_elem_id,
								CMD_ELEM_TYPE_E cmd_elem_type,
								CHAR *cmd_name,
								CHAR *cmd_help,
								CMD_VECTOR_S * pVec)
{
	CMD_ELEM_S * cmd_elem = NULL;

	cmd_elem = (CMD_ELEM_S *)malloc(sizeof(CMD_ELEM_S));
	if (NULL == cmd_elem)
	{
		return CMD_ERR;
	}

	cmd_elem->elem_id = cmd_elem_id;
	cmd_elem->elem_tpye = cmd_elem_type;

	cmd_elem->para = (char *)malloc(strlen(cmd_name) + 1);
	if (NULL == cmd_elem->para)
	{
		free(cmd_elem);
		return CMD_ERR;
	}
	memset(cmd_elem->para, 0, strlen(cmd_name) + 1);
	strcpy(cmd_elem->para, cmd_name);

	cmd_elem->desc = (char *)malloc(strlen(cmd_help) + 1);
	if (NULL == cmd_elem->desc)
	{
		free(cmd_elem->para);
		free(cmd_elem);
		return CMD_ERR;
	}
	memset(cmd_elem->desc, 0, strlen(cmd_help) + 1);
	strcpy(cmd_elem->desc, cmd_help);

	cmd_vector_insert(pVec, cmd_elem);

	return CMD_OK;
}
/* end cmd */

/* resolve */

/* Note: Added by weizengke, 2013/10/04 insert a word into the tail of input buffer */
VOID cmd_insert_word(struct cmd_vty *vty, CHAR *str)
{
	strcat(vty->buffer, str);
	vty->cur_pos += (int)strlen(str);
	vty->used_len += (int)strlen(str);
}

/* Note: Added by weizengke, 2013/10/04 delete the last word from input buffer */
VOID cmd_delete_word(struct cmd_vty *vty)
{
	int pos = strlen(vty->buffer);

	while (pos > 0 && vty->buffer[pos - 1] != ' ')
	{
		pos--;
	}

	/* BEGIN: Added by weizengke, 2014/8/3 */
	//vty->buffer[pos] = '\0';
	memset(&vty->buffer[pos], 0, sizeof(vty->buffer) - pos);
	/* END:   Added by weizengke, 2014/8/3 */

	vty->cur_pos = strlen(vty->buffer);
	vty->used_len = strlen(vty->buffer);
}

/* Note: Added by weizengke, 2013/10/04 delete the last word from input buffer*/
VOID cmd_delete_word_ctrl_W(struct cmd_vty *vty)
{
	/* 删除最后一个elem */

	int pos = strlen(vty->buffer);

	if (pos == 0)
	{
		return;
	}

	/* ignore suffix-space */
	while (vty->buffer[pos - 1] == ' ')
	{
		pos--;
	}

	/* del the last word */
	while (pos > 0 && vty->buffer[pos - 1] != ' ')
	{
		pos--;
	}

	/* BEGIN: Modified by weizengke, 2014/3/23, for https://github.com/weizengke/jungle/issues/1 */
	//vty->buffer[pos] = '\0';
	memset(&vty->buffer[pos], 0, sizeof(vty->buffer) - pos);
	/* END:   Modified by weizengke, 2014/3/23 */

	vty->cur_pos = strlen(vty->buffer);
	vty->used_len = strlen(vty->buffer);
}

/* Note: Added by weizengke, 2014/3/23 delete the last word at current pos from input buffer*/
VOID cmd_delete_word_ctrl_W_ex(struct cmd_vty *vty)
{
	/* 删除光标所在当前或之前elem */
	int start_pos = 0;
	int end_pos  = 0;
	int len = strlen(vty->buffer);
	int pos = vty->cur_pos;


	CMD_debug(CMD_DEBUG_INFO, "ctrl_W:cur_poscur_pos = %d buffer_len = %d", pos, len);

	if (pos == 0)
	{
		return;
	}

	/* ignore suffix-space */
	if (vty->buffer[pos] == ' ' || vty->buffer[pos] == '\0')
	{
		end_pos = pos;

		pos--;
		/* 往回找第一个非空字符 */
		while (pos  >= 0 && vty->buffer[pos] == ' ')
		{
			pos--;
		}

		if (pos == 0)
		{
			start_pos = 0;
		}
		else
		{
			/* 继续往回找第一个空格或命令头 */
			while (pos	>= 0 && vty->buffer[pos] != ' ')
			{
				pos--;
			}

			start_pos = pos + 1;

		}

	}
	else
	{
		/* 分别往左右找空格 */
		while (vty->buffer[pos + 1] != ' ')
		{
			/* BEGIN: Added by weizengke, 2014/4/5 当光标位于命令行最后一个元素中间，再执行CTRL+W，出现异常显示 https://github.com/weizengke/jungle/issues/2 */
			if (vty->buffer[pos + 1] == '\0') break;
			/* END:   Added by weizengke, 2014/4/5 */

			pos++;
		}

		/* BEGIN: Modified by weizengke, 2014/4/5 https://github.com/weizengke/jungle/issues/2 */
		end_pos = pos;
		/* END:   Modified by weizengke, 2014/4/5 */

		pos = vty->cur_pos;
		while (vty->buffer[pos] != ' ')
		{
			pos--;
		}

		start_pos = pos + 1;
	}

	int len_last = strlen(&vty->buffer[end_pos]);

	memcpy(&vty->buffer[start_pos], &vty->buffer[end_pos], strlen(&vty->buffer[end_pos]));
	memset(&vty->buffer[start_pos + len_last], 0, sizeof(vty->buffer)-(start_pos + len_last));


	vty->cur_pos -= (vty->cur_pos - start_pos);
	vty->used_len -= (end_pos - start_pos);

	CMD_debug(CMD_DEBUG_INFO, "ctrl+w end: buffer=%s, start_pos=%d end_pos=%d len_last=%d cur_pos=%d used_len=%d",
		vty->buffer,start_pos,end_pos,len_last,vty->cur_pos, vty->used_len);

}

ULONG cmd_pub_run(CHAR *szCmdBuf)
{
	ULONG iRet = 0;
	CMD_VTY_S *vty = g_cfm_vty;
	
	if (NULL == szCmdBuf)
	{
		return 1;
	}

	if (strlen(szCmdBuf) > CMD_BUFFER_SIZE)
	{
		CMD_debug(CMD_DEBUG_ERROR, "cmd_pub_run. input(%u) command length larger then %u", strlen(szCmdBuf), CMD_BUFFER_SIZE);
		return 1;
	}
	
	memcpy(vty->buffer, szCmdBuf, CMD_BUFFER_SIZE);
	vty->buf_len = CMD_BUFFER_SIZE;
	
	iRet = cmd_run(vty);
	
	vty->cur_pos = vty->used_len = 0;
	memset(vty->buffer, 0, vty->buf_len);

	return iRet;
}

ULONG cmd_run(struct cmd_vty *vty)
{
	CMD_VECTOR_S *v = NULL;
	struct para_desc *match[CMD_MAX_MATCH_SIZE] = {0};	// matched string
	ULONG match_size = 0;
	ULONG match_type = CMD_NO_MATCH;
	ULONG nomath_pos = CMD_NULL_DWORD;
	ULONG view_id_ = VIEW_NULL;

	v = cmd_str2vec(vty->buffer);
	if (v == NULL) {
		return 1;
	}

	/* BEGIN: Added by weizengke, 2013/10/5   PN:for cmd end with <CR> */
	cmd_vector_insert_cr(v);
	/* END:   Added by weizengke, 2013/10/5   PN:None */

	// do command
	match_type = cmd_execute_command(v, vty, match, &match_size, &nomath_pos);
	// add executed command into history

	CMD_debug(CMD_DEBUG_INFO, "cmd_run. (cmd=%s, match_type=%u)", vty->buffer, match_type);

	view_id_ = vty->view_id;

	/* 回退上一级视图执行 */
	while (match_type != CMD_FULL_MATCH)
	{
		ULONG match_size_ = 0;
		ULONG nomath_pos_ = CMD_NULL_DWORD;
		ULONG view_id__ = VIEW_NULL;
		
		view_id__ = vty_view_getParentViewId(vty->view_id);
		
		if (VIEW_NULL == view_id__
			|| VIEW_USER == view_id__
			|| VIEW_GLOBAL == view_id__)
		{
			vty->view_id = view_id_;
			break;
		}

		vty->view_id = view_id__;

		CMD_debug(CMD_DEBUG_FUNC, "Return back to view '%s'.", cmd_view_getViewName(vty->view_id));
		
		match_type = cmd_execute_command(v, vty, match, &match_size_, &nomath_pos_);
	}

	CMD_debug(CMD_DEBUG_FUNC, "* Execute command '%s' on view '%s'. (result:%u, username=%s, vtyId=%u)",
		vty->buffer, cmd_view_getViewName(vty->view_id), match_type, vty->user.user_name, vty->vtyId);
	
	write_log(0, "* Execute command '%s' on view '%s'. (result:%u, username=%s, vtyId=%u)",
		vty->buffer, cmd_view_getViewName(vty->view_id), match_type, vty->user.user_name, vty->vtyId);
	
	if (CMD_FULL_MATCH != match_type)
	{
		return 1;
	}

	return 0;
}


VOID cmd_read(struct cmd_vty *vty)
{
	int key_type;

	if (NULL == vty)
	{
		return ;
	}

	while((vty->c = vty_getch(vty)) != EOF)
	{			
		/* 用户检查是否断链 */
		if (0 == vty->used)
		{
			break;
		}

		/* 更新最后活动时间 */
		vty->user.lastAccessTime = time(NULL);

		/* step 1: get input key type */
		if (CMD_VTY_CONSOLE_ID == vty->vtyId)
		{
			key_type = cmd_resolve(vty);
		}
		else
		{
			key_type = cmd_resolve_vty(vty);
		}

		//CMD_debug(CMD_DEBUG_INFO, "cmd_read. (key_type=%d)", key_type);

		vty->inputMachine_now = key_type;

		if (key_type <= CMD_KEY_CODE_NONE || key_type > CMD_KEY_CODE_NOTCARE)
		{
			CMD_debug(CMD_DEBUG_ERROR, "Unidentify Key Type, c = %c, key_type = %d\n", vty->c, key_type);
			continue;
		}

		/* step 2: take actions according to input key */
		key_resolver[key_type].key_func(vty);
		vty->inputMachine_prev = vty->inputMachine_now;

		if (vty->inputMachine_now != CMD_KEY_CODE_TAB)
		{
			memset(vty->tabString,0,sizeof(vty->tabString));
			memset(vty->tabbingString,0,sizeof(vty->tabbingString));
			vty->tabStringLenth = 0;
		}

		if (vty->buf_len != CMD_BUFFER_SIZE
			|| vty->used_len > CMD_BUFFER_SIZE
			|| vty->cur_pos > CMD_BUFFER_SIZE)
		{
			CMD_DBGASSERT(0, "cmd_read, buffer=%s, used_len=%u, cur_pos=%u, buf_len=%u",
				vty->buffer, vty->used_len, vty->cur_pos,  vty->buf_len);
		}		

	}

	return ;

}

VOID vty_go(ULONG vtyId)
{
	struct cmd_vty *vty = NULL;

	vty = cmd_vty_getById(vtyId);
	if (NULL == vty)
	{
		return;
	}

	cmd_read(vty);

	return ;
}

/* Note: Added by weizengke, 2013/10 clear current line by cur-pos */
VOID cmd_clear_line(struct cmd_vty *vty)
{
	ULONG size = vty->used_len - vty->cur_pos;

	CMD_DBGASSERT((vty->used_len - vty->cur_pos >= 0), "cmd_clear_line");
	
	while (size--)
	{
		cmd_put_one(vty, ' ');
	}

	while (vty->used_len)
	{
		vty->used_len--;
		cmd_back_one(vty);
		cmd_put_one(vty, ' ');
		cmd_back_one(vty);
	}

	memset(vty->buffer, 0, HISTORY_MAX_SIZE);
}

struct cmd_vty * cmd_get_idlevty()
{
	ULONG i = 0;

	for (i == 0; i < CMD_VTY_MAXUSER_NUM; i++)
	{
		if (g_vty[i].used == 0)
		{
			g_vty[i].used = 1;
			return &g_vty[i];
		}
	}

	return NULL;
}

ULONG cmd_get_idle_vty()
{
	ULONG i = 0;

	for (i == 0; i < CMD_VTY_MAXUSER_NUM; i++)
	{
		if (g_vty[i].used == 0)
		{
			g_vty[i].used = 1;

			return i;
		}
	}

	return CMD_VTY_INVALID_ID;
}

VOID vty_offline(ULONG vtyId)
{
	ULONG i = 0;
	struct cmd_vty *vty = NULL;
	
	CMD_debug(CMD_DEBUG_FUNC, "vty_offline. (vtyId=%u)", vtyId);

	vty = cmd_vty_getById(vtyId);
	if (NULL == vty)
	{
		return;
	}

	closesocket(vty->user.socket);
	
	for (i = 0; i < HISTORY_MAX_SIZE; i++)
	{
		if (vty->history[i] != NULL)
		{
			free(vty->history[i]);
			vty->history[i] = NULL;
		}
	}
	
	cmd_vty_init(vty);

}

ULONG vty_set_socket(ULONG vtyId, ULONG socket)
{
	struct cmd_vty *vty = NULL;
	
	vty = cmd_vty_getById(vtyId);
	if (NULL == vty)
	{
		return CMD_ERR;
	}

	vty->user.socket = socket;

	return CMD_OK;
}

VOID cmd_outprompt(struct cmd_vty *vty)
{
	//CMD_debug(CMD_DEBUG_INFO, "cmd_outprompt. (view_id=%d, sysname=%s)", vty->view_id, cmd_get_sysname());

	if (vty->view_id == VIEW_USER)
	{
		cmd_vty_printf(vty, "%s>", cmd_get_sysname());
		return;
	}

	if (vty->view_id == VIEW_SYSTEM)
	{
		cmd_vty_printf(vty, "%s]", cmd_get_sysname());
		return;
	}

	view_node_st * view = NULL;
	view = cmd_view_lookup(view_list, vty->view_id);
	if (view == NULL)
	{
		cmd_vty_printf(vty, "%s>", cmd_get_sysname());
		return ;
	}

	cmd_vty_printf(vty, "%s-%s]", cmd_get_sysname(), view->view_ais_name);

}

ULONG cmd_view_getaislenth(struct cmd_vty *vty)
{
	view_node_st * view = NULL;

	view = cmd_view_lookup(view_list, vty->view_id);
	if (view == NULL)
	{
		return 0;
	}

	return strlen(view->view_ais_name);
}

CHAR* cmd_view_getAis(ULONG view_id)
{
	view_node_st * view = NULL;

	view = cmd_view_lookup(view_list, view_id);
	if (view == NULL)
	{
		return NULL;
	}

	return view->view_ais_name;
}

CHAR* cmd_view_getViewName(ULONG view_id)
{
	view_node_st * view = NULL;

	view = cmd_view_lookup(view_list, view_id);
	if (view == NULL)
	{
		return NULL;
	}

	return view->view_name;
}

ULONG vty_view_getParentViewId(ULONG view_id)
{
	view_node_st * view = NULL;
	view_node_st * prev_view = NULL;

	view = cmd_view_lookup(view_list, view_id);
	if (view == NULL)
	{
		return VIEW_NULL;
	}

	prev_view = view->pParent;
	if (prev_view == NULL)
	{
		return VIEW_NULL;
	}

	CMD_debug(CMD_DEBUG_FUNC, "vty_view_getParentViewId. (prev_view=%s, view_id=%u)", prev_view->view_name, prev_view->view_id);

	return prev_view->view_id;
}

view_node_st * cmd_view_lookup(view_node_st *view, ULONG view_id)
{
	view_node_st * view_son = NULL;
	view_node_st * view_ = NULL;

	if (NULL == view)
	{
		CMD_debug(CMD_DEBUG_ERROR, "cmd_view_lookup NULL. (view_id=%d)", view_id);
		return view;
	}

	//CMD_debug(CMD_DEBUG_FUNC, "cmd_view_lookup. (view(%d, view_name=%s, sons=%d), view_id=%d)", view->view_id, view->view_name, view->view_son_num, view_id);

	if (view->view_id == view_id)
	{
		//CMD_debug(CMD_DEBUG_ERROR, "cmd_view_lookup found. (view_id=%d, view_name=%s, sons=%d)", view->view_id, view->view_name, view->view_son_num);
		return view;
	}

	for(int i = 0; i < view->view_son_num; i++)
	{
		view_son  = view->ppSons[i];

		//CMD_debug(CMD_DEBUG_FUNC, "cmd_view_lookup sons. (view_son->view_id=%d, view_son->view_name=%s)", view_son->view_id, view_son->view_name);
		view_ = cmd_view_lookup(view_son, view_id);
		if (NULL != view_)
		{
			return view_;
		}
	}

	//CMD_debug(CMD_DEBUG_FUNC, "cmd_view_lookup not found. (view_id=%d)", view_id);

	return NULL;
}

VOID vty_view_set(ULONG vtyId, ULONG view_id)
{	
	view_node_st * view = NULL;
	struct cmd_vty *vty = NULL;

	vty = cmd_vty_getById(vtyId);
	if (NULL == vty)
	{
		return;
	}
	
	view = cmd_view_lookup(view_list, view_id);
	if (view == NULL)
	{
		return;
	}

	CMD_debug(CMD_DEBUG_FUNC, "vty_view_set. (view_id=%d)", view_id);

	if (view_id < VIEW_USER)
	{
		/* telnet 用户下线 */
		if (CMD_VTY_CONSOLE_ID != vty->vtyId)
		{
			vty_offline(vtyId);
		}

		return;
	}

	vty->view_id = view_id;
}


VOID vty_view_quit(ULONG vtyId)
{
	view_node_st * view = NULL;
	view_node_st * prev_view = NULL;
	struct cmd_vty *vty = NULL;

	vty = cmd_vty_getById(vtyId);
	if (NULL == vty)
	{
		return;
	}

	view = cmd_view_lookup(view_list, vty->view_id);
	if (view == NULL)
	{
		/* 用户离线 */
		vty_offline(vtyId);
		return;
	}

	prev_view = view->pParent;
	if (prev_view == NULL)
	{
		vty_offline(vtyId);
		return;
	}

	vty_view_set(vtyId, prev_view->view_id);

}

ULONG vty_get_current_viewid(ULONG vtyId)
{
	struct cmd_vty *vty = NULL;

	vty = cmd_vty_getById(vtyId);
	if (NULL == vty)
	{
		return VIEW_NULL;
	}

	return vty->view_id;

}

ULONG cmd_view_regist(CHAR *view_name, CHAR *view_ais, ULONG view_id, ULONG parent_view_id)
{
	view_node_st * view = NULL;
	view_node_st * pSons = NULL;
	view_node_st * pParent = NULL;

	if (VIEW_NULL != parent_view_id)
	{
		pParent = cmd_view_lookup(view_list, parent_view_id);
		if (NULL == pParent)
		{
			CMD_DBGASSERT(0, "cmd_view_regist, no parent view %u.", parent_view_id);
			return CMD_ERR;
		}
	}

	view = (view_node_st *)malloc(sizeof(view_node_st));
	if (NULL == view)
	{
		CMD_DBGASSERT(0, "cmd_view_regist, malloc failed.");
		return CMD_ERR;
	}
	memset(view, 0, sizeof(view_node_st));
	view->view_id = view_id;
	strcpy(view->view_name, view_name);
	strcpy(view->view_ais_name, view_ais);

	view->ppSons = (view_node_st **)malloc(CMD_VIEW_SONS_NUM * sizeof(view_node_st));
	if (NULL == view->ppSons)
	{
		CMD_DBGASSERT(0, "cmd_view_regist sons malloc failed");
		free(view);
		return CMD_ERR;
	}
	memset(view->ppSons, 0, CMD_VIEW_SONS_NUM * sizeof(view_node_st));

	view->view_son_num = 0;
	view->pParent = pParent;

	if (NULL != pParent)
	{
		if (pParent->view_son_num >= CMD_VIEW_SONS_NUM)
		{
			CMD_DBGASSERT(0, "cmd_view_regist sons num more than 100");
			free(view);
			free(view->ppSons);
			return CMD_ERR;
		}

		pParent->ppSons[pParent->view_son_num++] = view;
	}
	else
	{
		view_list = view;
	}


	return CMD_OK;
}

/*
视图能力
user(>)
  -system(])
     - diagnose(diagnose)
*/

/* 注册默认视图 */
VOID cmd_view_init()
{
	(void)cmd_view_regist("global", "", VIEW_GLOBAL, VIEW_NULL);
	(void)cmd_view_regist("user-view", "", VIEW_USER, VIEW_GLOBAL);
	(void)cmd_view_regist("system-view", "", VIEW_SYSTEM, VIEW_USER);
	(void)cmd_view_regist("diagnose-view", "diagnose", VIEW_DIAGNOSE, VIEW_SYSTEM);

	return;
}

/* ------------------ Interface Function ----------------- */
ULONG cmd_resolve_vty(struct cmd_vty *vty)
{
	CHAR c = vty->c;
	ULONG key_type = CMD_KEY_CODE_NOTCARE;	// default is not special key
	
	switch (c) 
	{
		case 0x1b:
			c = vty_getch(vty);
			if (0x5b == c)
			{
				c = vty_getch(vty);
				vty->c = c;
			}
			
			switch (c)
			{
				case 0x41:
					key_type = CMD_KEY_CODE_UP;
					break;
				case 0x42:
					key_type = CMD_KEY_CODE_DOWN;
					break;
				case 0x43:
					key_type = CMD_KEY_CODE_RIGHT;
					break;
				case 0x44:
					key_type = CMD_KEY_CODE_LEFT;
					break;
				default:
					key_type = CMD_KEY_CODE_FILTER;
					break;
			}

			break;
		case CMD_KEY_DELETE_VTY:
			key_type = CMD_KEY_CODE_DELETE;
			break;
		case CMD_KEY_BACKSPACE:  /*  */
			key_type = CMD_KEY_CODE_BACKSPACE;
			break;
		case CMD_KEY_SPACE:
			/* Linux 下空格后回车无法tab补全与'?'联想 待修复*/
			break;
		case CMD_KEY_CTRL_W:
			/* del the last elem */
			key_type = CMD_KEY_CODE_DEL_LASTWORD;
			break;

		case CMD_KEY_TAB:
			key_type = CMD_KEY_CODE_TAB;
			break;
		//case CMD_KEY_LF: 
		case CMD_KEY_CR: 
			key_type = CMD_KEY_CODE_ENTER;
			break;
		case CMD_KEY_QUEST:
			key_type = CMD_KEY_CODE_QUEST;
			break;
		default:
			/* BEGIN: Added by weizengke, 2014/4/6 filter CTRL+a ~ z */
			if (c >= 0x1 && c <= 0x1d)
			{
				key_type = CMD_KEY_CODE_FILTER;
			}
			else
			{
			}
			/* END:   Added by weizengke, 2014/4/6 */
			break;
	}

	return key_type;
}
/* end key*/

/* ------------------ Interface Function ----------------- */
ULONG cmd_resolve(struct cmd_vty *vty)
{
	CHAR c = vty->c;
	ULONG key_type = CMD_KEY_CODE_NOTCARE;	// default is not special key

	switch (c) {
	case CMD_KEY_ARROW1:
		c = cmd_getch();
		#ifdef _LINUX_
		if (c == CMD_KEY_ARROW2)
		{
			c = cmd_getch();
		#endif
			switch (c)
			{
				case CMD_KEY_UP:
					key_type = CMD_KEY_CODE_UP;
					break;
				case CMD_KEY_DOWN:
					key_type = CMD_KEY_CODE_DOWN;
					break;
				case CMD_KEY_RIGHT:
					key_type = CMD_KEY_CODE_RIGHT;
					break;
				case CMD_KEY_LEFT:
					key_type = CMD_KEY_CODE_LEFT;
					break;
				case CMD_KEY_DELETE:
					key_type = CMD_KEY_CODE_DELETE;
					break;
				/* BEGIN: Added by weizengke, 2014/4/6 support page up & down*/
				case CMD_KEY_PGUP:
					{
						extern HWND g_hWnd;
						::SendMessage(g_hWnd,WM_VSCROLL,MAKEWPARAM(SB_PAGEUP, 0),NULL);
						
						key_type = CMD_KEY_CODE_FILTER;
					}
					break;
				case CMD_KEY_PHDN:
					{
						extern HWND g_hWnd;
						::SendMessage(g_hWnd,WM_VSCROLL,MAKEWPARAM(SB_PAGEDOWN, 0),NULL);

						key_type = CMD_KEY_CODE_FILTER;
					}
					break;
				/* END:   Added by weizengke, 2014/4/6 */
				default:
					key_type = CMD_KEY_CODE_FILTER;
					break;
			}
		#ifdef _LINUX_
		}
		#endif
		break;
#ifndef _LINUX_  /* windwos */
	case CMD_KEY_ARROW2:
		c = cmd_getch();
		switch (c)
		{
			case CMD_KEY_UP:
				key_type = CMD_KEY_CODE_UP;
				break;
			case CMD_KEY_DOWN:
				key_type = CMD_KEY_CODE_DOWN;
				break;
			case CMD_KEY_RIGHT:
				key_type = CMD_KEY_CODE_RIGHT;
				break;
			case CMD_KEY_LEFT:
				key_type = CMD_KEY_CODE_LEFT;
				break;
			default:
				key_type = CMD_KEY_CODE_FILTER;
				break;
		}
	break;
#endif
	case CMD_KEY_BACKSPACE:  /*  */
		key_type = CMD_KEY_CODE_BACKSPACE;
		break;
	case CMD_KEY_SPACE:
	case CMD_KEY_CTRL_H:
		/* Linux 下空格后回车无法tab补全与'?'联想 待修复*/
		break;
	case CMD_KEY_CTRL_W:
		/* del the last elem */
		key_type = CMD_KEY_CODE_DEL_LASTWORD;
		break;
	case CMD_KEY_TAB:
		key_type = CMD_KEY_CODE_TAB;
		break;
	case CMD_KEY_LF:
	case CMD_KEY_CR: 
		key_type = CMD_KEY_CODE_ENTER;
		break;
	case CMD_KEY_QUEST:
		key_type = CMD_KEY_CODE_QUEST;
		break;
	default:
		/* BEGIN: Added by weizengke, 2014/4/6 filter CTRL+a ~ z */
		if (c >= 0x1 && c <= 0x1d)
		{
			key_type = CMD_KEY_CODE_FILTER;
		}
		/* END:   Added by weizengke, 2014/4/6 */
		break;
	}

	return key_type;
}
/* end key*/

VOID cmd_resolve_filter(struct cmd_vty *vty)
{
	return;
}

/*****************************************************************************
 函 数 名  : cmd_resolve_tab
 功能描述  : 适配TAB键，命令补全
 输入参数  : struct cmd_vty *vty
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年11月17日
    作    者   : weizengke
    修改内容   : 新生成函数

*****************************************************************************/
VOID cmd_resolve_tab(struct cmd_vty *vty)
{
	ULONG i;
	CMD_VECTOR_S *v;
	struct para_desc *match[CMD_MAX_MATCH_SIZE] = {0};	// matched string
	ULONG match_size = 0;
	ULONG match_type = CMD_NO_MATCH;
	ULONG isNeedMatch = 1;   /* 非TAB场景(无空格)，不需要匹配 */
	CHAR lcd_str[1024] = {0};	// if part match, then use this
	CHAR *last_word = NULL;

	if (strlen(vty->buffer) < vty->cur_pos)
	{
		CMD_DBGASSERT(0, "cmd_resolve_tab. (buffer=%s[%u], cur_pos=%u, used_len=%u, buf_len=%u)",
			vty->buffer, strlen(vty->buffer), vty->cur_pos, vty->used_len, vty->buf_len);
		return;
	}

	/*
	1: 取pos 之前的buf
	2: 需要覆盖当前光标后的buf
	*/
	/* BEGIN: Added by weizengke, 2013/11/17 bug for left and tab*/
	memset(&(vty->buffer[vty->cur_pos]), 0 ,strlen(vty->buffer) - vty->cur_pos);
	vty->used_len = strlen(vty->buffer);
	/* END:   Added by weizengke, 2013/11/17 */

	CMD_debug(CMD_DEBUG_FUNC,"tab in:used_len=%d, pos=%d\r\n",vty->used_len, vty->cur_pos);

	if (vty->inputMachine_prev == CMD_KEY_CODE_TAB)
	{
		cmd_delete_word(vty);
		cmd_insert_word(vty, vty->tabbingString);
	}
	else
	{
		memset(vty->tabString,0,sizeof(vty->tabString));
		vty->tabStringLenth = 0;
	}

	v = cmd_str2vec(vty->buffer);
	if (v == NULL)
	{
		/*
		v = cmd_vector_init(1);
		cmd_vector_insert(v, '\0');
		*/
		isNeedMatch = 0;
	}

	if (isspace((int)vty->buffer[vty->used_len - 1]))
	{
		isNeedMatch = 0;
	}

	if (1 == isNeedMatch && NULL != v)
	{
		match_type = cmd_match_command(v, vty, match, &match_size, lcd_str);

		last_word = (char*)cmd_vector_slot(v, cmd_vector_max(v) - 1);

		if (vty->inputMachine_prev != CMD_KEY_CODE_TAB)
		{
			strcpy(vty->tabbingString, last_word);
		}

		cmd_vector_deinit(v, 1);
	}

	cmd_vty_printf(vty, "%s", CMD_ENTER);
	switch (match_type) {
		case CMD_NO_MATCH:
			cmd_outprompt(vty);
			cmd_vty_printf(vty, "%s", vty->buffer);
			break;
		case CMD_FULL_MATCH:
			cmd_delete_word(vty);
			if (NULL != match[0])
			{
				cmd_insert_word(vty, match[0]->para);
			}
			/* BEGIN: Added by weizengke, 2013/10/14 for full match then next input*/
			cmd_insert_word(vty, " ");
			/* END:   Added by weizengke, 2013/10/14 */
			cmd_outprompt(vty);
			cmd_vty_printf(vty, "%s", vty->buffer);

			/* BEGIN: Added by weizengke, 2013/10/27 PN: fix the bug of CMD_FULL_MATCH and then continue TAB*/
			memset(vty->tabString,0,sizeof(vty->tabString));
			memset(vty->tabbingString,0,sizeof(vty->tabbingString));
			vty->tabStringLenth = 0;
			/* END:   Added by weizengke, 2013/10/27 */

			break;
		case CMD_PART_MATCH:
			/*  delete at 2013-10-05, CMD_PART_MATCH will never reach, CMD_LIST_MATCH instead.

				case like this:
					disable , display
					>di  TAB
					>dis  ==> CMD_PART_MATCH
			*/
			cmd_delete_word(vty);
			cmd_insert_word(vty, lcd_str);
			cmd_outprompt(vty);
			cmd_vty_printf(vty, "%s", vty->buffer);
			break;
		case CMD_LIST_MATCH:

			if (vty->inputMachine_prev != CMD_KEY_CODE_TAB)
			{
				memset(vty->tabString,0,sizeof(vty->tabString));
				strcpy(vty->tabString, match[0]->para);
				vty->tabStringLenth = strlen(vty->tabString);

				/* cmd_vty_printf(vty, "%s", CMD_ENTER); */
			}
			else
			{
				for (i = 0; i < match_size; i++)
				{
					if (0 == strcmp(vty->tabString, match[i]->para))
					{
						break;
					}
				}

				if (i == match_size)
				{
					CMD_debug(CMD_DEBUG_ERROR, "TAB for completing command. bug of tab continue. (tabString=%s)", vty->tabString);
				}

				i++;

				if (i == match_size)
				{
					i = 0;
				}

				memset(vty->tabString,0,sizeof(vty->tabString));
				strcpy(vty->tabString, match[i]->para);
				vty->tabStringLenth = strlen(vty->tabString);

			}

			/*for (i = 0; i < match_size; i++) {
				if (ANOTHER_LINE(i))
					cmd_vty_printf(vty, "%s", CMD_ENTER);
				cmd_vty_printf(vty, "%-25s", match[i]->para);
			}
			*/

			cmd_delete_word(vty);
			cmd_insert_word(vty, vty->tabString);

			cmd_outprompt(vty);
			cmd_vty_printf(vty, "%s", vty->buffer);
			break;
		default:
			break;
	}

	CMD_debug(CMD_DEBUG_FUNC,"tab out:used_len=%d, pos=%d",vty->used_len, vty->cur_pos);
	
}

/*****************************************************************************
 函 数 名  : cmd_resolve_enter
 功能描述  : 适配回车执行命令行
 输入参数  : struct cmd_vty *vty
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年11月17日
    作    者   : weizengke
    修改内容   : 新生成函数

bugs:
	1:	display loopback-detect brief
		display loopback brief

		<Jungle-config>dis loop brief
		Command 'dis loop brief ' anbigous follow:
		 brief                     brief
		<Jungle-config>

	2:
		display loopback-detect brief
		display loopback brief
		disable loopback-detect

		<Jungle-config>dis loopback
		Command 'dis loopback' anbigous follow:
		 loopback                  loopback-detect
		<Jungle-config>

*****************************************************************************/
VOID cmd_resolve_enter(struct cmd_vty *vty)
{
	struct para_desc *match[CMD_MAX_MATCH_SIZE];	// matched string
	ULONG match_size = 0;
	ULONG match_type = CMD_NO_MATCH;
	CMD_VECTOR_S *v;
	ULONG i = 0;
	ULONG nomath_pos = CMD_NULL_DWORD;
	ULONG view_id_ = VIEW_NULL;
	
	v = cmd_str2vec(vty->buffer);
	if (v == NULL) {
		cmd_vty_printf(vty, "%s", CMD_ENTER); /* 串口才需要回车 */
		cmd_outprompt(vty);
		return;
	}

	/* BEGIN: Added by weizengke, 2013/10/5   PN:for cmd end with <CR> */
	cmd_vector_insert_cr(v);
	/* END:   Added by weizengke, 2013/10/5   PN:None */

	cmd_vty_printf(vty, "%s", CMD_ENTER);

	// do command
	match_type = cmd_execute_command(v, vty, match, &match_size, &nomath_pos);

	view_id_ = vty->view_id;
		
	while (match_type == CMD_NO_MATCH)
	{
		ULONG match_size_ = 0;
		ULONG nomath_pos_ = CMD_NULL_DWORD;
		ULONG view_id__ = VIEW_NULL;
		
		view_id__ = vty_view_getParentViewId(vty->view_id);
		
		if (VIEW_NULL == view_id__
			|| VIEW_USER == view_id__
			|| VIEW_GLOBAL == view_id__)
		{
			vty->view_id = view_id_;
			break;
		}

		vty->view_id = view_id__;
		
		CMD_debug(CMD_DEBUG_FUNC, "Return back to view '%s'.", cmd_view_getViewName(vty->view_id));
		
		match_type = cmd_execute_command(v, vty, match, &match_size_, &nomath_pos_);
	}

	CMD_debug(CMD_DEBUG_FUNC, "Execute command '%s' on view '%s'. (result:%u, username=%s, vtyId=%u)",
		vty->buffer, cmd_view_getViewName(vty->view_id), match_type, vty->user.user_name, vty->vtyId);
	
	write_log(0, "Execute command '%s' on view '%s'. (result:%u, username=%s, vtyId=%u)",
		vty->buffer, cmd_view_getViewName(vty->view_id), match_type, vty->user.user_name, vty->vtyId);
	
	// add executed command into history
	cmd_vty_add_history(vty);

	if (match_type == CMD_NO_MATCH)
	{
		cmd_output_missmatch(vty, nomath_pos);
	}
	else if (match_type == CMD_ERR_ARGU)
	{
		cmd_vty_printf(vty, "Too Many Arguments%s"CMD_ENTER);
	}

	if (match_type == CMD_ERR_AMBIGOUS)
	{
		if (match_size)
		{
			cmd_vty_printf(vty, "Command '%s' anbigous follow:%s", vty->buffer, CMD_ENTER);

			for (i = 0; i < match_size; i++)
			{
				if (ANOTHER_LINE(i))
					cmd_vty_printf(vty, "%s", CMD_ENTER);
				cmd_vty_printf(vty, " %-25s", match[i]->para);
			}
			cmd_vty_printf(vty, "%s", CMD_ENTER);
			/* del 10-29
			cmd_outprompt(vty);
			cmd_vty_printf(vty, "%s", vty->buffer);
			*/
			vty->cur_pos = vty->used_len = 0;
			memset(vty->buffer, 0, vty->buf_len);
			cmd_outprompt(vty);
		}
	}
	else
	{
		// ready for another command
		vty->cur_pos = vty->used_len = 0;
		memset(vty->buffer, 0, vty->buf_len);
		cmd_outprompt(vty);
	}
}

/*****************************************************************************
 函 数 名  : cmd_resolve_quest
 功能描述  : 适配?字符，联想命令
 输入参数  : struct cmd_vty *vty
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年11月17日
    作    者   : weizengke
    修改内容   : 新生成函数

	1: 完全匹配输入时，只返回该命令的联想  2013-10-27 (未实现)

	2:bug:
		 display loopback
		 disable loopback-detect
	   >dis loop   =======> show what

*****************************************************************************/
VOID cmd_resolve_quest(struct cmd_vty *vty)
{
	CMD_VECTOR_S *v;
	struct para_desc *match[CMD_MAX_MATCH_SIZE];	// matched string
	ULONG match_size = 0;
	ULONG i = 0;
	ULONG nomath_pos = CMD_NULL_DWORD;

	/* BEGIN: Added by weizengke, 2013/10/4   PN:need print '?' */
	cmd_put_one(vty, '?');
	/* END:   Added by weizengke, 2013/10/4   PN:need print '?' */

	if (strlen(vty->buffer) < vty->cur_pos)
	{
		CMD_DBGASSERT(0, "cmd_resolve_quest. (buffer=%s[%u], cur_pos=%u, used_len=%u, buf_len=%u)",
			vty->buffer, strlen(vty->buffer), vty->cur_pos, vty->used_len, vty->buf_len);
		return;
	}
	
	/*
	1: 取pos 之前的buf
	2: 需要覆盖当前光标后的buf
	*/
	/* BEGIN: Added by weizengke, 2013/11/17 bug for left and tab*/
	memset(&(vty->buffer[vty->cur_pos]), 0 ,strlen(vty->buffer) - vty->cur_pos);
	vty->used_len = strlen(vty->buffer);
	/* END:   Added by weizengke, 2013/11/17 */

	v = cmd_str2vec(vty->buffer);
	if (v == NULL)
	{
		v = cmd_vector_init(1);
		cmd_vector_insert_cr(v);
	}
	else if (isspace((int)vty->buffer[vty->used_len - 1]))
	{
		cmd_vector_insert_cr(v);
	}

	cmd_complete_command(v, vty, match, &match_size, &nomath_pos);

	CMD_debug(CMD_DEBUG_FUNC, "cmd_resolve_quest. (match_size=%u, nomath_pos=%u)", match_size, nomath_pos);
	
	cmd_vty_printf(vty, "%s", CMD_ENTER);

	if (match_size > 0) {
		for (i = 0; i < match_size; i++) {
			cmd_vty_printf(vty, " %-25s%s\r\n", match[i]->para,match[i]->desc);
		}
		cmd_outprompt(vty);
		cmd_vty_printf(vty, "%s", vty->buffer);
	} else {

		cmd_output_missmatch(vty, nomath_pos);

		cmd_outprompt(vty);
		cmd_vty_printf(vty, "%s", vty->buffer);
	}

	cmd_vector_deinit(v, 0);
}


/* bug of up twice with last key is not up, the hpos not restart */
VOID cmd_resolve_up(struct cmd_vty *vty)
{
	ULONG try_idx = vty->hpos == 0 ? (HISTORY_MAX_SIZE - 1) : vty->hpos - 1;

	// if no history
	if (vty->history[try_idx] == NULL)
		return;
	vty->hpos = try_idx;

	// print history command
	cmd_clear_line(vty);
	strcpy(vty->buffer, vty->history[vty->hpos]);
	vty->cur_pos = vty->used_len = strlen(vty->history[vty->hpos]);
	cmd_vty_printf(vty, "%s", vty->buffer);
}

VOID cmd_resolve_down(struct cmd_vty *vty)
{
	ULONG try_idx = vty->hpos ==( HISTORY_MAX_SIZE - 1) ? 0 : vty->hpos + 1;

	// if no history
	if (vty->history[try_idx] == NULL)
		return;
	vty->hpos = try_idx;

	// print history command
	cmd_clear_line(vty);
	strcpy(vty->buffer, vty->history[vty->hpos]);
	vty->cur_pos = vty->used_len = strlen(vty->history[vty->hpos]);
	cmd_vty_printf(vty, "%s", vty->buffer);
}

// handle in read buffer, including left, right, delete, insert
VOID cmd_resolve_left(struct cmd_vty *vty)
{
	// already at leftmost, cannot move more
	if (vty->cur_pos <= 0)
	{
		return;
	}

	// move left one step
	vty->cur_pos--;
	cmd_back_one(vty);
}

VOID cmd_resolve_right(struct cmd_vty *vty)
{
	// already at rightmost, cannot move more
	if (vty->cur_pos >= vty->used_len)
	{
		return;
	}

	// move right one step
	cmd_put_one(vty, vty->buffer[vty->cur_pos]);
	vty->cur_pos++;
}

/*****************************************************************************
 函 数 名  : cmd_resolve_delete
 功能描述  : 适配Delete键，向前删除字符
 输入参数  : struct cmd_vty *vty
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年11月17日
    作    者   : weizengke
    修改内容   : 新生成函数

*****************************************************************************/
VOID cmd_resolve_delete(struct cmd_vty *vty)
{
	ULONG i, size;

	// no more to delete
	if (vty->cur_pos >= vty->used_len)
	{
		return;
	}

	/* del the  current char*/
	cmd_delete_one(vty);

	size = vty->used_len - vty->cur_pos;
	
	CMD_DBGASSERT((vty->used_len >= vty->cur_pos), 
		"cmd_resolve_delete. (used_len=%u, cur_pos=%u)",
		vty->used_len, vty->cur_pos);

	memcpy(&vty->buffer[vty->cur_pos], &vty->buffer[vty->cur_pos + 1], size);

	//vty->buffer[vty->used_len] = '\0';
	vty->buffer[vty->used_len - 1] = '\0';

	/* output the right chars */
	for (i = 0; i < size; i ++)
		cmd_put_one(vty, vty->buffer[vty->cur_pos + i]);

	vty->used_len--;

	/* back the cur_pos */
	for (i = 0; i < size; i++)
		cmd_back_one(vty);


}

VOID cmd_resolve_backspace(struct cmd_vty *vty)
{
	ULONG i, size;

	// no more to delete
	if (vty->cur_pos <= 0)
	{
		return;
	}

	size = vty->used_len - vty->cur_pos;
	
	CMD_DBGASSERT((vty->used_len >= vty->cur_pos), 
		"cmd_resolve_backspace. (used_len=%u, cur_pos=%u)",
		vty->used_len, vty->cur_pos);

	// delete char
	vty->cur_pos--;
	vty->used_len--;
	cmd_back_one(vty);

	// print left chars
	memcpy(&vty->buffer[vty->cur_pos], &vty->buffer[vty->cur_pos + 1], size);
	vty->buffer[vty->used_len] = '\0';
	for (i = 0; i < size; i ++)
		cmd_put_one(vty, vty->buffer[vty->cur_pos + i]);
	cmd_put_one(vty, ' ');
	for (i = 0; i < size + 1; i++)
		cmd_back_one(vty);

}

VOID cmd_resolve_insert(struct cmd_vty *vty)
{
	ULONG i, size;

	// no more to insert
	if (vty->used_len >= vty->buf_len)
	{
		CMD_debug(CMD_DEBUG_FUNC, "cmd_resolve_insert, used_len(%u)>=buf_len(%u, cur_pos=%u)",
			vty->used_len, vty->buf_len, vty->cur_pos);
	
		return;
	}
		
	if (vty->used_len < vty->cur_pos)
	{
		CMD_DBGASSERT(0, "cmd_resolve_insert. (used_len=%u, cur_pos=%u)", vty->used_len, vty->cur_pos);		
		return;
	}
	
	size = vty->used_len - vty->cur_pos;
	memcpy(&vty->buffer[vty->cur_pos + 1], &vty->buffer[vty->cur_pos], size);
	vty->buffer[vty->cur_pos] = vty->c;

#if 0
	/* BEGIN: del by weizengke, 2013/11/17 */
	/* BEGIN: Added by weizengke, 2013/10/4   PN:bug for continue tab */
	vty->buffer[vty->cur_pos + 1] = '\0';
	/* END:   Added by weizengke, 2013/10/4   PN:None */
	/* END: del by weizengke, 2013/11/17 */
#endif

	// print left chars, then back size
	for (i = 0; i < size + 1; i++)
		cmd_put_one(vty, vty->buffer[vty->cur_pos + i]);
	for (i = 0; i < size; i++)
		cmd_back_one(vty);

	vty->cur_pos++;
	vty->used_len++;

}

/*****************************************************************************
 函 数 名  : cmd_resolve_del_lastword
 功能描述  : 适配CTRL_W键，快速删除最后一个命令字
 输入参数  : struct cmd_vty *vty
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年11月17日
    作    者   : weizengke
    修改内容   : 新生成函数

*****************************************************************************/
VOID cmd_resolve_del_lastword(struct cmd_vty *vty)
{
	ULONG i, size;

	// no more to delete
	CMD_debug(CMD_DEBUG_FUNC, "\r\ncmd_resolve_del_lastword, cur_pos=%d",vty->cur_pos);
	
	if (vty->cur_pos <= 0)
		return;

	cmd_delete_word_ctrl_W_ex(vty);

	cmd_vty_printf(vty, "%s", CMD_ENTER);
	cmd_outprompt(vty);
	cmd_vty_printf(vty, "%s", vty->buffer);

	/* BEGIN: Added by weizengke, 2014/3/23 support delete word form cur_pos*/
	for (i = 0; i < strlen(vty->buffer) - vty->cur_pos; i++)
	{
		cmd_back_one(vty);
	}
	/* END:   Added by weizengke, 2014/3/23 */

}

int cmd_init()
{
	/* initial cmd vector */
	cmd_vec = cmd_vector_init(1);

	/* 视图初始化 */
	cmd_view_init();

	/* install cmd */
	//cmd_install();

	/* initial com_vty */
	g_con_vty = (struct cmd_vty *)calloc(1, sizeof(struct cmd_vty));
	if(g_con_vty == NULL)
	{
		return CMD_ERR;
	}
	cmd_vty_init(g_con_vty);
	g_con_vty->used = 1;
	g_con_vty->vtyId = CMD_VTY_CONSOLE_ID;
	g_con_vty->user.type = 0;
	g_con_vty->user.state = 1;
	g_con_vty->user.lastAccessTime = time(NULL);
	
	g_cfm_vty = (struct cmd_vty *)calloc(1, sizeof(struct cmd_vty));
	if(g_cfm_vty == NULL)
	{
		return CMD_ERR;
	}
	cmd_vty_init(g_cfm_vty);
	g_cfm_vty->used = 1;
	g_cfm_vty->vtyId = CMD_VTY_CFM_ID;
	g_cfm_vty->user.type = 0;
	g_cfm_vty->user.state = 1;
	g_cfm_vty->user.lastAccessTime = time(NULL);
	strcpy(g_cfm_vty->user.user_name, "CFM");
	
	for (int i = 0; i < CMD_VTY_MAXUSER_NUM; i++)
	{
		cmd_vty_init(&g_vty[i]);
		g_vty[i].vtyId = i;
		g_vty[i].user.type = 1;
	}

	return CMD_OK;
}

int cmd_main_entry(void *pEntry)
{
    extern ULONG PDT_IsCfgRecoverOver();
	
	for (;;)
	{
		/* 配置恢复还没结束，不进入命令行界面 */
		if (OS_NO == PDT_IsCfgRecoverOver())
		{
			Sleep(10);
			continue;
		}
		
		cmd_read(g_con_vty);
	}

	cmd_vty_deinit(g_con_vty);

	return 0;
}

