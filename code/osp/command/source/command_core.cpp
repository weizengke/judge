/*
	Author: Jungle Wei
	Date  : 2013-10
	Description: A common command line module

	支持特性: TAB自动选择性补全, '?'联想 , CTRL+W 快速删除, 错误位置提示, 命令行带参数, 命令行视图, telnet的vty技术


				 如梦令·听

				枕头微冷微倾，
				两眼半张半醒。
				侧卧怎难寝，
				声轻太过安静？
				你听，
				你听，
				雨打风吹未停。
					
	存储结构: 命令行向量 - 命令行元素向量
	
	g_pstCmdVec - CMD_LINE_S 1 - CMD_ELMT_S 1
	                           - CMD_ELMT_S 2
	                           - CMD_ELMT_S <CR>
	            - CMD_LINE_S 2 - CMD_ELMT_S 1
	            			   - CMD_ELMT_S 2
	            			   - CMD_ELMT_S 3
	            			   - CMD_ELMT_S <CR>
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

CMD_VECTOR_S *g_pstCmdVec;    /* 存储全局命令行向量，每个命令行向量下挂一个命令行元素向量 */
VIEW_NODE_S *g_pstViewList;   /* 存储视图树，维护视图之间的关系 */
CMD_NTF_NODE_S *g_pstCMDRunNtfList = NULL;  /* 存储命令行处理注册钩子 */
CMD_ELMT_S g_pstElmt_CR = {CMD_ELEM_TYPE_END, CMD_ELEM_ID_CR, CMD_END, ""}; /* 命令行结束符 */

KEY_HANDLE_S key_resolver[] = {
	{ CMD_KEY_CODE_FILTER, 	cmd_resolve_filter },
	{ CMD_KEY_CODE_TAB, 	cmd_resolve_tab },
	{ CMD_KEY_CODE_ENTER, 	cmd_resolve_enter },
	{ CMD_KEY_CODE_QUEST, 	cmd_resolve_quest },
	{ CMD_KEY_CODE_UP, 		cmd_resolve_up },
	{ CMD_KEY_CODE_DOWN, 	cmd_resolve_down },
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

ULONG cmd_regcallback(ULONG ulMid, ULONG (*pfCallBackFunc)(VOID *pRcvMsg))
{
	CMD_NTF_NODE_S * pstNow = NULL;
	CMD_NTF_NODE_S * pstPre = NULL;
	CMD_NTF_NODE_S * pstEvtNtfNodeNew = NULL;

	pstEvtNtfNodeNew = (CMD_NTF_NODE_S *)malloc(sizeof(CMD_NTF_NODE_S));
	if (NULL == pstEvtNtfNodeNew)
	{
		return CMD_ERR;
	}

	pstEvtNtfNodeNew->ulMid = ulMid;
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
		return CMD_VTY_INVALID_ID;
	}

	return pstRunMsg->vtyId;
}

VOID *cmd_get_elem_by_index(VOID *pRunMsg, ULONG index)
{
	CMD_RUNMSG_S *pstRunMsg = (CMD_RUNMSG_S *)pRunMsg;

	if (NULL == pstRunMsg)
	{
		return NULL;
	}

	return (void*)(pstRunMsg->argv + index);
}


ULONG cmd_get_elem_num(VOID *pRunMsg)
{
	CMD_RUNMSG_S *pstRunMsg = (CMD_RUNMSG_S *)pRunMsg;

	if (NULL == pstRunMsg)
	{
		return 0;
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

	return ((0x00FF0000 & pElem->ulElmtId) >> 16);

}

ULONG cmd_get_elemid(VOID *pElemMsg)
{
	CMD_RUNMSG_ELEM_S *pElem = (CMD_RUNMSG_ELEM_S *)pElemMsg;

	if (NULL == pElem)
	{
		return CMD_ELEMID_NULL;
	}

	return pElem->ulElmtId;
}

CHAR* cmd_get_elem_param(VOID *pElemMsg)
{
	CMD_RUNMSG_ELEM_S *pElem = (CMD_RUNMSG_ELEM_S *)pElemMsg;

	if (NULL == pElem)
	{
		return NULL;
	}

	return pElem->aszElmtArray;
}

ULONG cmd_get_ulong_param(VOID *pElemMsg)
{
	CMD_RUNMSG_ELEM_S *pElem = (CMD_RUNMSG_ELEM_S *)pElemMsg;

	if (NULL == pElem)
	{
		return 0xFFFFFFFF;
	}

	return atol(pElem->aszElmtArray);
}

VOID cmd_copy_string_param(VOID *pElemMsg, CHAR *param)
{
	CMD_RUNMSG_ELEM_S *pElem = (CMD_RUNMSG_ELEM_S *)pElemMsg;

	if (NULL == pElem)
	{
		return ;
	}

	strcpy(param, pElem->aszElmtArray);

	return ;
}

ULONG cmd_get_ip_ulong_param(VOID *pElemMsg)
{
	CMD_RUNMSG_ELEM_S *pElem = (CMD_RUNMSG_ELEM_S *)pElemMsg;

	if (NULL == pElem)
	{
		return 0xFFFFFFFF;
	}

	if (0 == cmd_string_is_ip(pElem->aszElmtArray))
	{
		return 0xFFFFFFFF;
	}
			
	return cmd_ip_string_to_ulong(pElem->aszElmtArray);
}


ULONG cmd_run_notify(CMD_LINE_S * pstCmdLine, CMD_VTY_S *vty, CHAR **argv, ULONG argc)
{
	ULONG index  = 0;
	ULONG isize = 0;
	ULONG ret = CMD_OK;
	ULONG argcNum = 0;
	CMD_RUNMSG_S *pstRunMsg = NULL;
	CMD_RUNMSG_ELEM_S *pstRunMsgElem = NULL;
	CMD_NTF_NODE_S * pstHead =  g_pstCMDRunNtfList;

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
		CMD_ELMT_S * cmd_elem = (CMD_ELMT_S *)cmd_vector_get(pstCmdLine->pstElmtVec, i);

		CMD_debug(DEBUG_TYPE_FUNC,
			"cmd_run_notify: 0x%x, %d, %s, %s, %s, %d",
			cmd_elem->ulElmtId, cmd_elem->eElmtType, cmd_elem->pszElmtName, cmd_elem->pszElmtHelp, argv[i], argc);

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
	CMD_VTY_S *vty = NULL;

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

VOID cmd_vty_printf(CMD_VTY_S *vty, CHAR *format, ...)
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

VOID cmd_delete_one(CMD_VTY_S *vty)
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

VOID cmd_back_one(CMD_VTY_S *vty)
{
	cmd_vty_printf(vty, "\b");
}

VOID cmd_put_one(CMD_VTY_S *vty, CHAR c)
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

int vty_getch(CMD_VTY_S *vty)
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

/* vty */
CMD_VTY_S *cmd_vty_getById(ULONG vtyId)
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

VOID cmd_vty_init(CMD_VTY_S *vty)
{
	vty->vtyId = vty->vtyId;
	vty->used = 0;
	vty->view_id = VIEW_USER;
	vty->ulBufMaxLen = CMD_BUFFER_SIZE;
	vty->ulUsedLen = vty->ulCurrentPos = 0;
	vty->ulhpos = vty->ulhNum = 0;

	vty->ucKeyTypePre = CMD_KEY_CODE_NOTCARE;
	vty->ucKeyTypeNow = CMD_KEY_CODE_NOTCARE;
	memset(vty->tabbingString, 0, CMD_MAX_CMD_ELEM_SIZE);
	memset(vty->tabString, 0, CMD_MAX_CMD_ELEM_SIZE);

	vty->user.socket = INVALID_SOCKET;
	vty->user.state = 0;
	vty->user.terminal_debugging = 0;
	memset(vty->user.user_name, 0, sizeof(vty->user.user_name));
	//vty->user.lastAccessTime = time(NULL);

	return ;
}

VOID cmd_vty_add_history(CMD_VTY_S *vty)
{
	ULONG idx =  vty->ulhNum ? vty->ulhNum - 1 : HISTORY_MAX_SIZE - 1;

	/* if same as previous command, then ignore */
	if (vty->pszHistory[idx] &&
		!strcmp(vty->szBuffer, vty->pszHistory[idx]))
	{
		vty->ulhpos = vty->ulhNum;
		return;
	}

	/* insert into history tail */
	if (vty->pszHistory[vty->ulhNum])
	{
		free(vty->pszHistory[vty->ulhNum]);
	}

	vty->pszHistory[vty->ulhNum] = strdup(vty->szBuffer);
	vty->ulhNum = (vty->ulhNum + 1) == HISTORY_MAX_SIZE ? 0 : vty->ulhNum + 1;
	vty->ulhpos = vty->ulhNum;

}

CMD_VECTOR_S *cmd_vector_init()
{
	CMD_VECTOR_S *v = NULL;

	v = (CMD_VECTOR_S *)malloc(sizeof(CMD_VECTOR_S));
	if (v == NULL)
	{
		return NULL;
	}
	memset(v, 0, sizeof(CMD_VECTOR_S));

	v->ulSize = 0;
	v->ppData = NULL;
	
	return v;
}

/* freeall标识是否需要释放data数组内指针指向的内存 */
VOID cmd_vector_deinit(CMD_VECTOR_S *v, ULONG freeall)
{
	if (v == NULL)
	{
		return;
	}

	if (v->ppData)
	{
		if (freeall)
		{
			ULONG i;
			for (i = 0; i < cmd_vector_size(v); i++)
			{
				if (cmd_vector_get(v, i))
				{
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
	if (NULL == new_v)
	{
		CMD_DBGASSERT(0, "cmd_vector_copy");
		return NULL;
	}
	memset(new_v, 0, sizeof(CMD_VECTOR_S));

	new_v->ulSize = v->ulSize;

	size = sizeof(void *) * (v->ulSize);
	new_v->ppData = (void**)malloc(size);
	if (NULL == new_v->ppData)
	{
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
	
	v->ppData = (void**)realloc(v->ppData, size);
	if (!v->ppData)
	{
		CMD_debug(CMD_DEBUG_ERROR, "cmd_vector_insert, no enough memory for data.");
		CMD_DBGASSERT(0,"cmd_vector_insert, no enough memory for data.");
		return ;
	}
	
	memset(&v->ppData[v->ulSize], 0, sizeof(void *) * 1);
	
	v->ppData[v->ulSize] = val;
	v->ulSize += 1;

	return;
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

	if (string == NULL)
	{
		return NULL;
	}

	cur = string;

	/* 跳过前缀为空格的字符 */
	while (*cur == ' ' && *cur != '\0')
	{
		cur++;
	}

	if (*cur == '\0')
	{
		return NULL;
	}

	vec = cmd_vector_init();
	if(NULL == vec)
	{
		return NULL;
	}

	while (1)
	{
		start = cur;
		while (!(*cur == ' ' || *cur == '\r' || *cur == '\n') &&
				*cur != '\0')
		{
			cur++;
		}

		str_len = cur - start;
		token = (CHAR *)malloc(sizeof(CHAR) * (str_len + 1));
		if (NULL == token)
		{
			CMD_debug(CMD_DEBUG_ERROR, "In cmd_str2vec, There is no memory for param token.");
			return NULL;
		}

		memcpy(token, start, str_len);
		*(token + str_len) = '\0';
		cmd_vector_insert(vec, (void *)token);

		while((*cur == ' ' || *cur == '\n' || *cur == '\r') &&
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
	CMD_VECTOR_S *vec = NULL;
	CMD_ELMT_S *pstCmdElem = NULL;

	if(NULL == pVec
		|| NULL == pCmdLine)
	{
		return NULL;
	}

	vec = cmd_vector_init();
	if(NULL == vec)
	{
		return NULL;
	}

	for (ULONG i = 0; i < n; i++)
	{
		if (0 == pCmdLine[i])
		{
			CMD_DBGASSERT(0, "cmd cmd2vec, cmdline index cannot be 0.");
			return NULL;
		}

		if (pCmdLine[i] - 1 >= cmd_vector_size(pVec))
		{
			CMD_DBGASSERT(0, 
				"cmd cmd2vec, cmdline index(%u) cannot more than %u.",
				pCmdLine[i], cmd_vector_size(pVec));
			return NULL;
		}

		pstCmdElem = (CMD_ELMT_S *)cmd_vector_get(pVec, pCmdLine[i] - 1);

		CMD_ELMT_S * pstCmdElmtTmp = NULL;
		pstCmdElmtTmp = (CMD_ELMT_S *)malloc(sizeof(CMD_ELMT_S));
		if (NULL == pstCmdElmtTmp)
		{
			return NULL;
		}

		pstCmdElmtTmp->ulElmtId = pstCmdElem->ulElmtId;
		pstCmdElmtTmp->eElmtType = pstCmdElem->eElmtType;

		pstCmdElmtTmp->pszElmtName = (CHAR *)malloc(strlen(pstCmdElem->pszElmtName) + 1);
		if (NULL == pstCmdElmtTmp->pszElmtName)
		{
			free(pstCmdElmtTmp);
			return NULL;
		}
		memset(pstCmdElmtTmp->pszElmtName, 0, strlen(pstCmdElem->pszElmtName) + 1);
		strcpy(pstCmdElmtTmp->pszElmtName, pstCmdElem->pszElmtName);

		pstCmdElmtTmp->pszElmtHelp = (CHAR *)malloc(strlen(pstCmdElem->pszElmtHelp) + 1);
		if (NULL == pstCmdElmtTmp->pszElmtHelp)
		{
			free(pstCmdElmtTmp->pszElmtName);
			free(pstCmdElmtTmp);
			return NULL;
		}
		memset(pstCmdElmtTmp->pszElmtHelp, 0, strlen(pstCmdElem->pszElmtHelp) + 1);
		strcpy(pstCmdElmtTmp->pszElmtHelp, pstCmdElem->pszElmtHelp);

		cmd_vector_insert(vec, (void *)pstCmdElmtTmp);
	}

	cmd_vector_insert(vec, (void *)&g_pstElmt_CR);

	return vec;
}

/* end vector */


/* cmd */
static ULONG cmd_match_unique_string(CMD_ELMT_S **pstCmdElem, CHAR *str, ULONG size)
{
	ULONG i;

	for (i = 0; i < size; i++)
	{
		if (pstCmdElem[i]->pszElmtName != NULL && strcmp(pstCmdElem[i]->pszElmtName, str) == 0)
		{
			return 0;
		}
	}

	return 1;
}

static ULONG cmd_match_unique_elmtid(CMD_ELMT_S **pstCmdElem, ULONG ulElmtId, ULONG size)
{
	ULONG i;

	for (i = 0; i < size; i++)
	{
		CMD_debug(CMD_DEBUG_FUNC, "cmd_match_unique_elmtid. (ulElmtId=%u, %u)",
									pstCmdElem[i]->ulElmtId,
									ulElmtId);
		if (pstCmdElem[i]->ulElmtId == ulElmtId)
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

		while((*cur == ' ' || *cur == '\n' || *cur == '\r') &&
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

/*****************************************************************************
*   Prototype    : cmd_output_missmatch
*   Description  : Output miss match command position
*   Input        : CMD_VTY_S *vty
*                  int ulNoMatchPos
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
VOID cmd_output_missmatch(CMD_VTY_S *vty, ULONG ulNoMatchPos)
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

	(VOID)cmd_get_nth_elem_pos(vty->szBuffer, ulNoMatchPos, &n);
	pos_arrow += n;

	for (i=0;i<pos_arrow;i++)
	{
		cmd_vty_printf(vty, " ");
	}

	cmd_vty_printf(vty, "^\r\nError: Unrecognized command at '^' position.\r\n");

}

/* 检查字符串是不是只包含数字 */
ULONG cmd_string_isdigit(CHAR *string)
{
	ULONG i = 0;
	if (string == NULL)
	{
		return CMD_ERR;
	}

	for (i = 0; i < (int)strlen(string); i++)
	{
		if (!isdigit(*(string + i)))
		{
			CMD_debug(CMD_DEBUG_ERROR, "cmd_string_isdigit return error. (c=%c)", *(string + i));
			return CMD_ERR;
		}
	}

	CMD_debug(CMD_DEBUG_INFO, "cmd_string_isdigit return ok.");
	return CMD_OK;
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

	if (NULL == pstElmt->pszElmtName)
	{
		return CMD_ERR;
	}

	pleft  = strchr(pstElmt->pszElmtName, '<');
	pline  = strchr(pstElmt->pszElmtName, '-');
	pright = strchr(pstElmt->pszElmtName, '>');

	if (pleft == NULL || pline == NULL || pright == NULL)
	{
		return CMD_ERR;
	}

	sscanf(pstElmt->pszElmtName,"%[A-Z]<%u-%u>", type_string, &a, &b);
	if (0 != strcmp(type_string, CMD_INTEGER))
	{
		return CMD_ERR;
	}

	CMD_debug(CMD_DEBUG_INFO, "%s<%u-%u>",type_string, a, b);

	/* icmd must only has digit */
	if (CMD_OK == cmd_string_isdigit(icmd))
	{
		/* ULONG is not larger than 4294967295 */
		if (strlen(icmd) > 10
			|| (strlen(icmd) == 10 && strcmp(icmd,"4294967295") > 0))
		{
			return CMD_ERR;
		}

		/* check range is valid */
		icmd_i = atol(icmd);
		if (icmd_i >= a && icmd_i <=b)
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
	
	if (NULL == pstElmt->pszElmtName)
	{
		return CMD_ERR;
	}

	pleft  = strchr(pstElmt->pszElmtName, '<');
	pline  = strchr(pstElmt->pszElmtName, '-');
	pright = strchr(pstElmt->pszElmtName, '>');

	if (pleft == NULL || pline == NULL || pright == NULL)
	{
		return CMD_ERR;
	}

	sscanf(pstElmt->pszElmtName,"%[A-Z]<%u-%u>", type_string, &a, &b);
	if (0 != strcmp(type_string, CMD_STRING))
	{
		return CMD_ERR;
	}

	CMD_debug(CMD_DEBUG_INFO, "%s<%u-%u>",type_string, a, b);

	/* 检查整形参数是否在范围内 */
    icmd_len = (ULONG)strlen(icmd);
	if (icmd_len >= a && icmd_len <= b)
	{
		return CMD_OK;
	}

	return CMD_ERR;
}

ULONG cmd_ip_string_to_ulong(CHAR *ip)  
{  
	ULONG re = 0;
	UCHAR tmp = 0;

	while (1) 
	{
		if (*ip != '\0' && *ip != '.')
		{
			tmp = tmp * 10 + *ip - '0';
		} 
		else
		{
			re = (re << 8) + tmp;
			if (*ip == '\0')
			{
				break;
			}
			
			tmp = 0;
		}
		
		ip++;
	}
      
    return re; 
}   
  
VOID cmd_ip_ulong_to_string(ULONG ip, CHAR *buf)  
{  
    sprintf(buf, "%u.%u.%u.%u",  
        (UCHAR)*((CHAR *)&ip + 3),  
        (UCHAR)*((CHAR *)&ip + 2),  
        (UCHAR)*((CHAR *)&ip + 1),  
        (UCHAR)*((CHAR *)&ip + 0));  

	return;
}  

ULONG cmd_string_is_ip(CHAR *str)
{
	ULONG ulLen = 0;
	ULONG aulArr[4] = {0};
	ULONG ulLoop = 0;
	ULONG ulIndex = 0;
	ULONG ulValue = 0;
	ULONG ulIsDotOrEnd = 0;
	
	if (NULL == str)
	{
		return 0;
	}
	
	ulLen = strlen(str);
	if(ulLen < 7 || ulLen > 15) return 0;   

	for (ulLoop = 0; ulLoop < ulLen; ulLoop++)
	{
		if ('.' == str[ulLoop])
		{
			/* 连续的'.' */
			if (1 == ulIsDotOrEnd)
			{
				return 0;
			}
			
			ulIsDotOrEnd = 1;
			aulArr[ulIndex++] = ulValue;
			ulValue = 0;
		}
		else
		{
			ulIsDotOrEnd = 0;
			
			/* not digit */
			if (!isdigit(str[ulLoop]))
			{
				return 0;
			}
			
			ulValue = ulValue * 10 + (str[ulLoop] - '0');

			if (ulValue > 255)
			{
				return 0;
			}
		}				
	}

	if (0 == ulIsDotOrEnd)
	{
		aulArr[ulIndex++] = ulValue;
		ulValue = 0;
	}
	
	if (4 != ulIndex)
	{
		return 0;
	}

	return 1;
}

ULONG cmd_match_command_ip(CHAR *icmd, CMD_ELMT_S *pstElmt)
{

	if (icmd == NULL || pstElmt == NULL)
	{
		return CMD_ERR;
	}
	
	if (NULL == pstElmt->pszElmtName)
	{
		return CMD_ERR;
	}

	if (0 != strcmp(pstElmt->pszElmtName, CMD_IP))
	{
		return CMD_ERR;
	}
	
	if (1 == cmd_string_is_ip(icmd))
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

	CMD_debug(CMD_DEBUG_INFO, "cmd_match_command_param. icmd=%s, pszElmtName=%s",
				icmd, pstElmt->pszElmtName);
	
	switch(pstElmt->eElmtType)
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

	for (i = 0; i < cmd_vector_size(v); i++)
	{
		if ((pstCmdLine = (CMD_LINE_S*)cmd_vector_get(v, i)) != NULL)
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
				if(strnicmp(cmd, pstCmdElem->pszElmtName, strlen(cmd)) != 0)
				{
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
		CMD_ELMT_S **ppstMatchCmdElem, ULONG *pulMatchNum)
{
	ULONG i = 0;	
	ULONG isize = 0;
	ULONG size = 0;
	CMD_VECTOR_S *cmd_vec_copy = cmd_vector_copy(g_pstCmdVec);

	if (NULL == icmd_vec
		|| NULL == cmd_vec_copy)
	{
		return CMD_NO_MATCH;
	}

	isize = cmd_vector_size(icmd_vec) - 1;

	/* scan all command vectors, set null while not match. */
	for (i = 0; i < isize; i++)
	{
		if (CMD_OK != cmd_filter_command(vty, (CHAR*)cmd_vector_get(icmd_vec, i), cmd_vec_copy, i))
		{
			*pulMatchNum = 0;
			return CMD_NO_MATCH;
		}
	}
	
	for(i = 0; i < cmd_vector_size(cmd_vec_copy); i++)
	{
		CMD_LINE_S *pstCmdLine = NULL;
		pstCmdLine = (CMD_LINE_S *)cmd_vector_get(cmd_vec_copy, i);

		if(pstCmdLine != NULL)
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
			CHAR *str = (CHAR*)cmd_vector_get(icmd_vec, isize);

			if (str == NULL || strnicmp(str, pstCmdElem->pszElmtName, strlen(str)) == 0)
			{
				if (cmd_match_unique_string(ppstMatchCmdElem, pstCmdElem->pszElmtName, size))
					ppstMatchCmdElem[size++] = pstCmdElem;
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

/*****************************************************************************
 Prototype    : cmd_complete_command
 Description  : complete command　for ? complete
 Input        : CMD_VECTOR_S *icmd_vec  input cmd vector
                CMD_VTY_S *vty     the input vty

 Output       : char **doc              the return doc
 Return Value :
 Calls        :
 Called By    :

  History        :
  1.Date         : 2013/10/4
    Author       : weizengke
    Modification : Created function

*****************************************************************************/
ULONG cmd_complete_command(CMD_VECTOR_S *icmd_vec, CMD_VTY_S *vty,
									 CMD_ELMT_S **ppstCmdElem, ULONG *pulMatchNum, ULONG *pulNoMatchPos)
{


	ULONG i = 0;
	CMD_VECTOR_S *cmd_vec_copy = cmd_vector_copy(g_pstCmdVec);
	ULONG ulMatchNum = 0;
	CHAR *str = NULL;
	CMD_ELMT_S *pstCmdElem = NULL;
	CMD_LINE_S *pstCmdLine = NULL;

	if (icmd_vec == NULL || vty == NULL || ppstCmdElem == NULL || pulMatchNum == NULL)
	{
		return CMD_ERR;
	}

	*pulNoMatchPos = CMD_NULL_DWORD;

	/* scan all command vectors, set null while not match. */
	/* BEGIN: Modified by weizengke, 2013/10/4   PN:循环过滤每一个向量 */
	for (i = 0; i < cmd_vector_size(icmd_vec); i++)
	{
		if (CMD_OK != cmd_filter_command(vty, (CHAR*)cmd_vector_get(icmd_vec, i), cmd_vec_copy, i))
		{
			/* BEGIN: Added by weizengke, 2013/11/19 这里可以优化，不命中可以不需要再匹配了 */
			/* 保存在第几个命令字无法匹配 */
			*pulNoMatchPos = (*pulNoMatchPos == CMD_NULL_DWORD)?(i):(*pulNoMatchPos);

			CMD_debug(CMD_DEBUG_ERROR, "cmd_complete_command, cannot match at pos %u", *pulNoMatchPos);

			cmd_vector_deinit(cmd_vec_copy, 0);
			
			return CMD_OK;
		}
	}
	/* END:   Modified by weizengke, 2013/10/4   PN:None */

	for(i = 0; i < cmd_vector_size(cmd_vec_copy); i++)
	{
		pstCmdLine = (CMD_LINE_S *)cmd_vector_get(cmd_vec_copy, i);

		if(pstCmdLine  != NULL)
		{
			if (cmd_vector_size(icmd_vec) - 1 >= cmd_vector_size(pstCmdLine->pstElmtVec))
			{
				cmd_vector_get(cmd_vec_copy, i) = NULL;
				continue;
			}

			//CMD_debug(CMD_DEBUG_FUNC, "cmd_complete_command, view_id=%d.", pstCmdLine->ulViewId);

			if (pstCmdLine->ulViewId != VIEW_GLOBAL
				&& pstCmdLine->ulViewId != vty->view_id)
			{
				continue;
			}

			str = (CHAR*)cmd_vector_get(icmd_vec, cmd_vector_size(icmd_vec) - 1);
			pstCmdElem = (CMD_ELMT_S *)cmd_vector_get(pstCmdLine->pstElmtVec, cmd_vector_size(icmd_vec) - 1);
			if (pstCmdElem == NULL)
			{
				continue;
			}

			CMD_debug(CMD_DEBUG_FUNC, "cmd_complete_command, eElmtType=%u", pstCmdElem->eElmtType);
			
			/* BEGIN: Added by weizengke, 2013/11/19 */
			/* match STRING , INTEGER 
			   bug: ip complete
			*/
			/* match param */
			if (CMD_YES == cmd_elem_is_para_type(pstCmdElem->eElmtType))
			{
				/* 无条件补全 */
				if (0 == strnicmp(str, CMD_END, strlen(str)))
				{
					CMD_debug(CMD_DEBUG_FUNC, "cmd_complete_command, input cr, eElmtType=%u, pszElmtName=%s, str=%s",
												pstCmdElem->eElmtType,
												pstCmdElem->pszElmtName,
												str);

					ppstCmdElem[ulMatchNum] = pstCmdElem;	
					ulMatchNum++;

				}
				/* 无条件补全 */
				else if (0 == strnicmp(pstCmdElem->pszElmtName, CMD_END, strlen(pstCmdElem->pszElmtName)))
				{
					CMD_debug(CMD_DEBUG_FUNC, "cmd_complete_command, cr, eElmtType=%u, pszElmtName=%s, str=%s",
												pstCmdElem->eElmtType,
												pstCmdElem->pszElmtName,
												str);

					ppstCmdElem[ulMatchNum] = pstCmdElem;	
					ulMatchNum++;
				}
				/* 有条件补全 */
				else if (CMD_OK == cmd_match_command_param(str, pstCmdElem))
				{					
					CMD_debug(CMD_DEBUG_FUNC, "cmd_complete_command, param, eElmtType=%u, pszElmtName=%s",
												pstCmdElem->eElmtType,
												pstCmdElem->pszElmtName);
					
					if (cmd_match_unique_elmtid(ppstCmdElem, pstCmdElem->ulElmtId, ulMatchNum))
					{
						ppstCmdElem[ulMatchNum] = pstCmdElem;
						ulMatchNum++;
					}
				}
				else
				{
					CMD_debug(CMD_DEBUG_FUNC, "cmd_complete_command, other, eElmtType=%u, pszElmtName=%s",
												pstCmdElem->eElmtType,
												pstCmdElem->pszElmtName);
				}
			}
			/* END:   Added by weizengke, 2013/11/19 */
			else
			{
				CMD_debug(CMD_DEBUG_FUNC, "cmd_complete_command, key, eElmtType=%u, pszElmtName=%s",
											pstCmdElem->eElmtType,
											pstCmdElem->pszElmtName);
					
				/* match key */
				if (0 == strnicmp(str, CMD_END, strlen(str)) /* 无任何输入时输入?*/
					|| strnicmp(str, pstCmdElem->pszElmtName, strlen(str)) == 0)
				{				
					/* get only one if more than one  */
					if (cmd_match_unique_string(ppstCmdElem, pstCmdElem->pszElmtName, ulMatchNum))
					{
						ppstCmdElem[ulMatchNum] = pstCmdElem;
						ulMatchNum++;
					}
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
				CMD_ELMT_S *pstCmdElem_ = NULL;
				if (0 == strnicmp(ppstCmdElem[i]->pszElmtName, CMD_END, strlen(ppstCmdElem[i]->pszElmtName))
					|| ( 1 == stricmp(ppstCmdElem[i]->pszElmtName, ppstCmdElem[j]->pszElmtName)
					&& 0 != strnicmp(ppstCmdElem[j]->pszElmtName, CMD_END, strlen(ppstCmdElem[j]->pszElmtName)))
					)
				{
					pstCmdElem_ = ppstCmdElem[i];
					ppstCmdElem[i] =  ppstCmdElem[j];
					ppstCmdElem[j] = pstCmdElem_;
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
	CMD_VECTOR_S *cmd_vec_copy = cmd_vector_copy(g_pstCmdVec);
	CMD_LINE_S *pstMatchCmdLine = NULL;
	ULONG ulMatchNum = 0;

	*pulNoMatchPos = CMD_NULL_DWORD;

	CMD_debug(CMD_DEBUG_INFO, "cmd_execute_command. (cmd=%s, view_id=%u)", vty->szBuffer, vty->view_id);

	for (i = 0; i < cmd_vector_size(icmd_vec); i++)
	{
		if (CMD_OK != cmd_filter_command(vty, (CHAR*)cmd_vector_get(icmd_vec, i), cmd_vec_copy, i))
		{
			/* BEGIN: Added by weizengke, 2013/11/19 不命中可以不需要再匹配了 */
			/* 保存在第几个命令字无法匹配 */
			*pulNoMatchPos = (*pulNoMatchPos == CMD_NULL_DWORD)?(i):(*pulNoMatchPos);

			CMD_debug(CMD_DEBUG_ERROR, "cmd_execute_command. not match at pos %u.", *pulNoMatchPos);

			cmd_vector_deinit(cmd_vec_copy, 0);
			
			return CMD_NO_MATCH;
		}
	}

	if (*pulNoMatchPos == CMD_NULL_DWORD)
	{
		/* BEGIN: Added by weizengke, 2014/3/9 修复命令行不完全时，错误位置提示不准确的问题 */
		*pulNoMatchPos = cmd_vector_size(icmd_vec) - 1;

		for(i = 0; i < cmd_vector_size(cmd_vec_copy); i++)
		{
			char *str;
			CMD_ELMT_S *pstCmdElem;
			CMD_LINE_S *pstCmdLine = NULL;

			pstCmdLine = (CMD_LINE_S *)cmd_vector_get(cmd_vec_copy, i);
			if(pstCmdLine != NULL)
			{
				if (pstCmdLine->ulViewId != VIEW_GLOBAL
				&& pstCmdLine->ulViewId != vty->view_id)
				{
					continue;
				}

				str = (CHAR*)cmd_vector_get(icmd_vec, cmd_vector_size(icmd_vec) - 1);
				pstCmdElem = (CMD_ELMT_S *)cmd_vector_get(pstCmdLine->pstElmtVec, cmd_vector_size(icmd_vec) - 1);

				/* modified for command without argv */
				if (cmd_vector_size(icmd_vec) == cmd_vector_size(pstCmdLine->pstElmtVec))
				{
					/* BEGIN: Added by weizengke, 2013/10/5   PN:for support STRING<a-b> & INTEGER<a-b> */
					if (CMD_OK == cmd_match_command_param(str, pstCmdElem) ||
						str != NULL && strnicmp(str, pstCmdElem->pszElmtName, strlen(str)) == 0)
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

	for (i = 0, argc = 0; i < pstMatchCmdLine->ulElmtNum; i ++)
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
			argv[argc++] = (CHAR*)cmd_vector_get(icmd_vec, i);
		}
		else
		{
			argv[argc++] = pstCmdElem->pszElmtName;
		}
	}
	/* END:   Added by weizengke, 2013/10/5   PN:None */

	/* execute command */
	(VOID)cmd_run_notify(pstMatchCmdLine, vty, argv, argc);

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

	if (g_pstCmdVec == NULL)
	{
		CMD_debug(CMD_DEBUG_ERROR, "Command Vector Not Exist");
		return;
	}

	CMD_LINE_S *pstCmdLine = NULL;

	pstCmdLine = (CMD_LINE_S *)malloc(sizeof(CMD_LINE_S));
	if (NULL == pstCmdLine)
	{
		return;
	}
	memset(pstCmdLine, 0, sizeof(CMD_LINE_S));

	pstCmdLine->ulMid = mid;
	pstCmdLine->ulViewId = cmd_view;
	pstCmdLine->pstElmtVec = cmd_cmd2vec(pVec, iIndex, iLoop);
    if (NULL == pstCmdLine->pstElmtVec)
    {
		free(pstCmdLine);
        return;
    }
	pstCmdLine->ulElmtNum = cmd_vector_size(pstCmdLine->pstElmtVec);

	cmd_vector_insert(g_pstCmdVec, pstCmdLine);

	return;
}

ULONG cmd_regelement_new(ULONG cmd_elem_id,
								CMD_ELEM_TYPE_E cmd_elem_type,
								CHAR *cmd_name,
								CHAR *cmd_help,
								CMD_VECTOR_S * pVec)
{
	CMD_ELMT_S * pstCmdElem = NULL;

	if (CMD_MAX_CMD_ELEM_SIZE <= strlen(cmd_name))
	{
		CMD_DBGASSERT(0,"cmd_regelement_new");
		return CMD_ERR;
	}

	pstCmdElem = (CMD_ELMT_S *)malloc(sizeof(CMD_ELMT_S));
	if (NULL == pstCmdElem)
	{
		return CMD_ERR;
	}

	pstCmdElem->ulElmtId = cmd_elem_id;
	pstCmdElem->eElmtType = cmd_elem_type;

	pstCmdElem->pszElmtName = (char *)malloc(strlen(cmd_name) + 1);
	if (NULL == pstCmdElem->pszElmtName)
	{
		free(pstCmdElem);
		return CMD_ERR;
	}
	memset(pstCmdElem->pszElmtName, 0, strlen(cmd_name) + 1);
	strcpy(pstCmdElem->pszElmtName, cmd_name);

	pstCmdElem->pszElmtHelp = (char *)malloc(strlen(cmd_help) + 1);
	if (NULL == pstCmdElem->pszElmtHelp)
	{
		free(pstCmdElem->pszElmtName);
		free(pstCmdElem);
		return CMD_ERR;
	}
	memset(pstCmdElem->pszElmtHelp, 0, strlen(cmd_help) + 1);
	strcpy(pstCmdElem->pszElmtHelp, cmd_help);

	cmd_vector_insert(pVec, pstCmdElem);

	return CMD_OK;
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

/* Note: Added by weizengke, 2013/10/04 delete the last word from input buffer*/
VOID cmd_delete_word_ctrl_W(CMD_VTY_S *vty)
{
	/* 删除最后一个elem */

	int pos = strlen(vty->szBuffer);

	if (pos == 0)
	{
		return;
	}

	/* ignore suffix-space */
	while (vty->szBuffer[pos - 1] == ' ')
	{
		pos--;
	}

	/* del the last word */
	while (pos > 0 && vty->szBuffer[pos - 1] != ' ')
	{
		pos--;
	}

	/* BEGIN: Modified by weizengke, 2014/3/23, for https://github.com/weizengke/jungle/issues/1 */
	//vty->buffer[pos] = '\0';
	memset(&vty->szBuffer[pos], 0, sizeof(vty->szBuffer) - pos);
	/* END:   Modified by weizengke, 2014/3/23 */

	vty->ulCurrentPos = strlen(vty->szBuffer);
	vty->ulUsedLen = strlen(vty->szBuffer);
}

/* Note: Added by weizengke, 2014/3/23 delete the last word at current pos from input buffer*/
VOID cmd_delete_word_ctrl_W_ex(CMD_VTY_S *vty)
{
	/* 删除光标所在当前或之前elem */
	int start_pos = 0;
	int end_pos  = 0;
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
		/* 往回找第一个非空字符 */
		while (pos  >= 0 && vty->szBuffer[pos] == ' ')
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
			while (pos	>= 0 && vty->szBuffer[pos] != ' ')
			{
				pos--;
			}

			start_pos = pos + 1;

		}

	}
	else
	{
		/* 分别往左右找空格 */
		while (vty->szBuffer[pos + 1] != ' ')
		{
			/* BEGIN: Added by weizengke, 2014/4/5 当光标位于命令行最后一个元素中间，再执行CTRL+W，出现异常显示 https://github.com/weizengke/jungle/issues/2 */
			if (vty->szBuffer[pos + 1] == '\0') break;
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
	memset(&vty->szBuffer[start_pos + len_last], 0, sizeof(vty->szBuffer)-(start_pos + len_last));


	vty->ulCurrentPos -= (vty->ulCurrentPos - start_pos);
	vty->ulUsedLen -= (end_pos - start_pos);

	CMD_debug(CMD_DEBUG_INFO, "ctrl+w end: buffer=%s, start_pos=%d end_pos=%d len_last=%d cur_pos=%d used_len=%d",
		vty->szBuffer,start_pos,end_pos,len_last,vty->ulCurrentPos, vty->ulUsedLen);

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
		CMD_debug(CMD_DEBUG_ERROR, 
			"cmd_pub_run. input(%u) command length larger then %u",
			strlen(szCmdBuf), CMD_BUFFER_SIZE);
		return 1;
	}

	memcpy(vty->szBuffer, szCmdBuf, CMD_BUFFER_SIZE);
	vty->ulBufMaxLen = CMD_BUFFER_SIZE;

	iRet = cmd_run(vty);

	vty->ulCurrentPos = vty->ulUsedLen = 0;
	memset(vty->szBuffer, 0, vty->ulBufMaxLen);

	return iRet;
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
	if (v == NULL) {
		return 1;
	}

	/* BEGIN: Added by weizengke, 2013/10/5   PN:for cmd end with <CR> */
	cmd_vector_insert_cr(v);
	/* END:   Added by weizengke, 2013/10/5   PN:None */

	view_id_pre = vty->view_id;
	
	// do command
	ulMatchType = cmd_execute_command(v, vty, pstCmdElem, &ulMatchNum, &ulNoMatchPos);
	// add executed command into history

	CMD_debug(CMD_DEBUG_INFO, "cmd_run. (cmd=%s, match_type=%u)", vty->szBuffer, ulMatchType);

	view_id_ = vty->view_id;

	/* 回退上一级视图执行 */
	while (ulMatchType != CMD_FULL_MATCH)
	{
		ULONG ulMatchNum_ = 0;
		ULONG ulNoMatchPos_ = CMD_NULL_DWORD;
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
		view_id_pre = vty->view_id;
		CMD_debug(CMD_DEBUG_FUNC, "Return back to view '%s'.", cmd_view_getViewName(vty->view_id));

		ulMatchType = cmd_execute_command(v, vty, pstCmdElem, &ulMatchNum_, &ulNoMatchPos_);
	}

	CMD_debug(CMD_DEBUG_FUNC, "Execute command '%s' on view '%s'. (result:%u, username=%s, vtyId=%u)",
		vty->szBuffer, cmd_view_getViewName(view_id_pre), ulMatchType, vty->user.user_name, vty->vtyId);

	write_log(0, "Execute command '%s' on view '%s'. (result:%u, username=%s, vtyId=%u)",
		vty->szBuffer, cmd_view_getViewName(view_id_pre), ulMatchType, vty->user.user_name, vty->vtyId);

	if (CMD_FULL_MATCH != ulMatchType)
	{
		return 1;
	}

	return 0;
}

/* Note: Added by weizengke, 2013/10 clear current line by cur-pos */
VOID cmd_clear_line(CMD_VTY_S *vty)
{
	ULONG size = vty->ulUsedLen - vty->ulCurrentPos;

	CMD_DBGASSERT((vty->ulUsedLen - vty->ulCurrentPos >= 0), "cmd_clear_line");

	while (size--)
	{
		cmd_put_one(vty, ' ');
	}

	while (vty->ulUsedLen)
	{
		vty->ulUsedLen--;
		cmd_back_one(vty);
		cmd_put_one(vty, ' ');
		cmd_back_one(vty);
	}

	memset(vty->szBuffer, 0, HISTORY_MAX_SIZE);
}

CMD_VTY_S * cmd_get_idlevty()
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
	CMD_VTY_S *vty = NULL;

	CMD_debug(CMD_DEBUG_FUNC, "vty_offline. (vtyId=%u)", vtyId);

	vty = cmd_vty_getById(vtyId);
	if (NULL == vty)
	{
		return;
	}

	if (INVALID_SOCKET != vty->user.socket)
	{
		closesocket(vty->user.socket);
		vty->user.socket = INVALID_SOCKET;
	}	

	for (i = 0; i < HISTORY_MAX_SIZE; i++)
	{
		if (vty->pszHistory[i] != NULL)
		{
			free(vty->pszHistory[i]);
			vty->pszHistory[i] = NULL;
		}
	}

	cmd_vty_init(vty);
}

VOID vty_offline_by_username(CHAR *pszName)
{
	ULONG i = 0;

	if (NULL == pszName)
	{
		return;
	}
	
	for (int i = 0; i < CMD_VTY_MAXUSER_NUM; i++)
	{
		if (g_vty[i].used
			&& g_vty[i].user.state)
		{
			if (0 == strcmp(pszName, g_vty[i].user.user_name))
			{
				vty_offline(g_vty[i].vtyId);
			}
		}
	}
}

ULONG vty_set_socket(ULONG vtyId, ULONG socket)
{
	CMD_VTY_S *vty = NULL;

	vty = cmd_vty_getById(vtyId);
	if (NULL == vty)
	{
		return CMD_ERR;
	}

	vty->user.socket = socket;

	return CMD_OK;
}

VOID cmd_outprompt(CMD_VTY_S *vty)
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

	VIEW_NODE_S * view = NULL;
	view = cmd_view_lookup(g_pstViewList, vty->view_id);
	if (view == NULL)
	{
		cmd_vty_printf(vty, "%s>", cmd_get_sysname());
		return ;
	}

	cmd_vty_printf(vty, "%s-%s]", cmd_get_sysname(), view->szViewAisName);

}

ULONG cmd_view_getaislenth(CMD_VTY_S *vty)
{
	VIEW_NODE_S * view = NULL;

	view = cmd_view_lookup(g_pstViewList, vty->view_id);
	if (view == NULL)
	{
		return 0;
	}

	return strlen(view->szViewAisName);
}

CHAR* cmd_view_getAis(ULONG view_id)
{
	VIEW_NODE_S * view = NULL;

	view = cmd_view_lookup(g_pstViewList, view_id);
	if (view == NULL)
	{
		return NULL;
	}

	return view->szViewAisName;
}

CHAR* cmd_view_getViewName(ULONG view_id)
{
	VIEW_NODE_S * view = NULL;

	view = cmd_view_lookup(g_pstViewList, view_id);
	if (view == NULL)
	{
		return NULL;
	}

	return view->szViewName;
}

ULONG vty_view_getParentViewId(ULONG view_id)
{
	VIEW_NODE_S * view = NULL;
	VIEW_NODE_S * prev_view = NULL;

	view = cmd_view_lookup(g_pstViewList, view_id);
	if (view == NULL)
	{
		return VIEW_NULL;
	}

	prev_view = view->pParent;
	if (prev_view == NULL)
	{
		return VIEW_NULL;
	}

	CMD_debug(CMD_DEBUG_FUNC, "vty_view_getParentViewId. (prev_view=%s, view_id=%u)", prev_view->szViewName, prev_view->ulViewId);

	return prev_view->ulViewId;
}

VIEW_NODE_S * cmd_view_lookup(VIEW_NODE_S *view, ULONG view_id)
{
	VIEW_NODE_S * view_son = NULL;
	VIEW_NODE_S * view_ = NULL;

	if (NULL == view)
	{
		CMD_debug(CMD_DEBUG_ERROR, "cmd_view_lookup NULL. (view_id=%d)", view_id);
		return view;
	}

	//CMD_debug(CMD_DEBUG_FUNC, "cmd_view_lookup. (view(%d, view_name=%s, sons=%d), view_id=%d)", view->view_id, view->view_name, view->view_son_num, view_id);

	if (view->ulViewId == view_id)
	{
		//CMD_debug(CMD_DEBUG_ERROR, "cmd_view_lookup found. (view_id=%d, view_name=%s, sons=%d)", view->view_id, view->view_name, view->view_son_num);
		return view;
	}

	for(int i = 0; i < view->ulViewSonNum; i++)
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
	VIEW_NODE_S * view = NULL;
	CMD_VTY_S *vty = NULL;

	vty = cmd_vty_getById(vtyId);
	if (NULL == vty)
	{
		return;
	}

	view = cmd_view_lookup(g_pstViewList, view_id);
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
	VIEW_NODE_S * view = NULL;
	VIEW_NODE_S * prev_view = NULL;
	CMD_VTY_S *vty = NULL;

	vty = cmd_vty_getById(vtyId);
	if (NULL == vty)
	{
		return;
	}

	view = cmd_view_lookup(g_pstViewList, vty->view_id);
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

	vty_view_set(vtyId, prev_view->ulViewId);

}

ULONG vty_get_current_viewid(ULONG vtyId)
{
	CMD_VTY_S *vty = NULL;

	vty = cmd_vty_getById(vtyId);
	if (NULL == vty)
	{
		return VIEW_NULL;
	}

	return vty->view_id;

}

ULONG cmd_view_regist(CHAR *view_name, CHAR *view_ais, ULONG view_id, ULONG parent_view_id)
{
	VIEW_NODE_S * view = NULL;
	VIEW_NODE_S * pSons = NULL;
	VIEW_NODE_S * pParent = NULL;

	if (VIEW_NULL != parent_view_id)
	{
		pParent = cmd_view_lookup(g_pstViewList, parent_view_id);
		if (NULL == pParent)
		{
			CMD_DBGASSERT(0, "cmd_view_regist, no parent view %u.", parent_view_id);
			return CMD_ERR;
		}
	}

	view = (VIEW_NODE_S *)malloc(sizeof(VIEW_NODE_S));
	if (NULL == view)
	{
		CMD_DBGASSERT(0, "cmd_view_regist, malloc failed.");
		return CMD_ERR;
	}
	memset(view, 0, sizeof(VIEW_NODE_S));
	view->ulViewId = view_id;
	strcpy(view->szViewName, view_name);
	strcpy(view->szViewAisName, view_ais);

	view->ppSons = (VIEW_NODE_S **)malloc(CMD_VIEW_SONS_NUM * sizeof(VIEW_NODE_S));
	if (NULL == view->ppSons)
	{
		CMD_DBGASSERT(0, "cmd_view_regist sons malloc failed");
		free(view);
		return CMD_ERR;
	}
	memset(view->ppSons, 0, CMD_VIEW_SONS_NUM * sizeof(VIEW_NODE_S));

	view->ulViewSonNum = 0;
	view->pParent = pParent;

	if (NULL != pParent)
	{
		if (pParent->ulViewSonNum >= CMD_VIEW_SONS_NUM)
		{
			CMD_DBGASSERT(0, "cmd_view_regist sons num more than 100");
			free(view);
			free(view->ppSons);
			return CMD_ERR;
		}

		pParent->ppSons[pParent->ulViewSonNum++] = view;
	}
	else
	{
		g_pstViewList = view;
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

ULONG cmd_resolve_vty(CMD_VTY_S *vty)
{
	CHAR c = vty->c;
	ULONG key_type = CMD_KEY_CODE_NOTCARE;

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

ULONG cmd_resolve(CMD_VTY_S *vty)
{
	CHAR c = vty->c;
	ULONG key_type = CMD_KEY_CODE_NOTCARE;

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

VOID cmd_resolve_filter(CMD_VTY_S *vty)
{
	return;
}

/*****************************************************************************
 函 数 名  : cmd_resolve_tab
 功能描述  : 适配TAB键，命令补全
 输入参数  : CMD_VTY_S *vty
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年11月17日
    作    者   : weizengke
    修改内容   : 新生成函数

*****************************************************************************/
VOID cmd_resolve_tab(CMD_VTY_S *vty)
{
	ULONG i = 0;
	CMD_VECTOR_S *v = NULL;
	CMD_ELMT_S *pstCmdElem[CMD_MAX_MATCH_SIZE] = {0};
	ULONG ulMatchNum = 0;
	ULONG ulMatchType = CMD_NO_MATCH;
	ULONG isNeedMatch = 1;   /* 非TAB场景(无空格)，不需要匹配 */
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
	memset(&(vty->szBuffer[vty->ulCurrentPos]), 0 ,strlen(vty->szBuffer) - vty->ulCurrentPos);
	vty->ulUsedLen = strlen(vty->szBuffer);
	/* END:   Added by weizengke, 2013/11/17 */

	CMD_debug(CMD_DEBUG_FUNC,"tab in:ulUsedLen=%d, ulCurrentPos=%d\r\n",vty->ulUsedLen, vty->ulCurrentPos);

	if (vty->ucKeyTypePre == CMD_KEY_CODE_TAB)
	{
		cmd_delete_word(vty);
		cmd_insert_word(vty, vty->tabbingString);
	}
	else
	{
		memset(vty->tabString,0,sizeof(vty->tabString));
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

	if (1 == isNeedMatch && NULL != v)
	{
		ulMatchType = cmd_match_command(v, vty, pstCmdElem, &ulMatchNum);

		last_word = (CHAR*)cmd_vector_get(v, cmd_vector_size(v) - 1);

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
			if (NULL != pstCmdElem[0])
			{
				cmd_insert_word(vty, pstCmdElem[0]->pszElmtName);
			}
			/* BEGIN: Added by weizengke, 2013/10/14 for full match then next input*/
			cmd_insert_word(vty, " ");
			/* END:   Added by weizengke, 2013/10/14 */
			cmd_outprompt(vty);
			cmd_vty_printf(vty, "%s", vty->szBuffer);

			/* BEGIN: Added by weizengke, 2013/10/27 PN: fix the bug of CMD_FULL_MATCH and then continue TAB*/
			memset(vty->tabString,0,sizeof(vty->tabString));
			memset(vty->tabbingString,0,sizeof(vty->tabbingString));
			/* END:   Added by weizengke, 2013/10/27 */

			break;
		case CMD_LIST_MATCH:
			/* first TAB result */
			if (vty->ucKeyTypePre != CMD_KEY_CODE_TAB)
			{
				memset(vty->tabString,0,sizeof(vty->tabString));
				strcpy(vty->tabString, pstCmdElem[0]->pszElmtName);
			}
			else
			{
				for (i = 0; i < ulMatchNum; i++)
				{
					if (0 == strcmp(vty->tabString, pstCmdElem[i]->pszElmtName))
					{
						break;
					}
				}

				i++;

				if (i == ulMatchNum)
				{
					i = 0;
				}

				memset(vty->tabString,0,sizeof(vty->tabString));
				strcpy(vty->tabString, pstCmdElem[i]->pszElmtName);
			}

			cmd_delete_word(vty);
			cmd_insert_word(vty, vty->tabString);

			cmd_outprompt(vty);
			cmd_vty_printf(vty, "%s", vty->szBuffer);
			break;
		default:
			break;
	}

	CMD_debug(CMD_DEBUG_FUNC,"tab out:ulUsedLen=%d, ulCurrentPos=%d",vty->ulUsedLen, vty->ulCurrentPos);

	return;
}

/*****************************************************************************
 函 数 名  : cmd_resolve_enter
 功能描述  : 适配回车执行命令行
 输入参数  : CMD_VTY_S *vty
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
VOID cmd_resolve_enter(CMD_VTY_S *vty)
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

		view_id__ = vty_view_getParentViewId(vty->view_id);

		if (VIEW_NULL == view_id__
			|| VIEW_USER == view_id__
			|| VIEW_GLOBAL == view_id__)
		{
			vty->view_id = view_id_;
			break;
		}

		vty->view_id = view_id__;
		view_id_pre = vty->view_id;
		CMD_debug(CMD_DEBUG_FUNC, "Return back to view '%s'.", cmd_view_getViewName(vty->view_id));

		ulMatchType = cmd_execute_command(v, vty, pstCmdElem, &ulMatchNum_, &ulNoMatchPos_);
	}

	CMD_debug(CMD_DEBUG_FUNC, "Execute command '%s' on view '%s'. (result:%u, username=%s, vtyId=%u)",
		vty->szBuffer, cmd_view_getViewName(view_id_pre), ulMatchType, vty->user.user_name, vty->vtyId);

	write_log(0, "Execute command '%s' on view '%s'. (result:%u, username=%s, vtyId=%u)",
		vty->szBuffer, cmd_view_getViewName(view_id_pre), ulMatchType, vty->user.user_name, vty->vtyId);

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
				
				cmd_vty_printf(vty, " %-25s", pstCmdElem[i]->pszElmtName);
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

/*****************************************************************************
 函 数 名  : cmd_resolve_quest
 功能描述  : 适配?字符，联想命令
 输入参数  : CMD_VTY_S *vty
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
VOID cmd_resolve_quest(CMD_VTY_S *vty)
{
	CMD_VECTOR_S *v = NULL;
	CMD_ELMT_S *pstCmdElem[CMD_MAX_MATCH_SIZE] = {0};
	ULONG ulMatchNum = 0;
	ULONG i = 0;
	ULONG ulNoMatchPos = CMD_NULL_DWORD;

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
	memset(&(vty->szBuffer[vty->ulCurrentPos]), 0 ,strlen(vty->szBuffer) - vty->ulCurrentPos);
	vty->ulUsedLen = strlen(vty->szBuffer);
	/* END:   Added by weizengke, 2013/11/17 */

	v = cmd_str2vec(vty->szBuffer);
	if (v == NULL)
	{
		v = cmd_vector_init();
		if(NULL == v)
		{
			return ;
		}
			
		cmd_vector_insert_cr(v);
	}
	else if (' ' == vty->szBuffer[vty->ulUsedLen - 1])
	{
		cmd_vector_insert_cr(v);
	}

	cmd_complete_command(v, vty, pstCmdElem, &ulMatchNum, &ulNoMatchPos);

	cmd_vty_printf(vty, "%s", CMD_ENTER);
	if (ulMatchNum > 0) 
	{
	    cmd_vty_printf(vty,"%s commands:\r\n",cmd_view_getViewName(vty->view_id));
		for (i = 0; i < ulMatchNum; i++)
		{
			cmd_vty_printf(vty, " %-25s%s\r\n", pstCmdElem[i]->pszElmtName,pstCmdElem[i]->pszElmtHelp);
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

	
	CMD_debug(CMD_DEBUG_FUNC, "cmd_resolve_quest. (ulMatchNum=%u, ulNoMatchPos=%u)", ulMatchNum, ulNoMatchPos);

	return;
}


/* bug of up twice with last key is not up, the hpos not restart */
VOID cmd_resolve_up(CMD_VTY_S *vty)
{
	ULONG idx = 0;

	idx = vty->ulhpos == 0 ? (HISTORY_MAX_SIZE - 1) : vty->ulhpos - 1;

	if (vty->pszHistory[idx] == NULL)
	{
		return;
	}
	
	vty->ulhpos = idx;

	cmd_clear_line(vty);
	strcpy(vty->szBuffer, vty->pszHistory[vty->ulhpos]);
	vty->ulCurrentPos = vty->ulUsedLen = strlen(vty->pszHistory[vty->ulhpos]);
	cmd_vty_printf(vty, "%s", vty->szBuffer);

	return;
}

VOID cmd_resolve_down(CMD_VTY_S *vty)
{
	ULONG idx = 0;

	idx = vty->ulhpos ==( HISTORY_MAX_SIZE - 1) ? 0 : vty->ulhpos + 1;

	if (vty->pszHistory[idx] == NULL)
	{
		return;
	}
	
	vty->ulhpos = idx;

	cmd_clear_line(vty);
	strcpy(vty->szBuffer, vty->pszHistory[vty->ulhpos]);
	vty->ulCurrentPos = vty->ulUsedLen = strlen(vty->pszHistory[vty->ulhpos]);
	cmd_vty_printf(vty, "%s", vty->szBuffer);

	return;
}

VOID cmd_resolve_left(CMD_VTY_S *vty)
{
	if (vty->ulCurrentPos <= 0)
	{
		return;
	}

	vty->ulCurrentPos--;
	cmd_back_one(vty);

	return;
}

VOID cmd_resolve_right(CMD_VTY_S *vty)
{
	if (vty->ulCurrentPos >= vty->ulUsedLen)
	{
		return;
	}

	cmd_put_one(vty, vty->szBuffer[vty->ulCurrentPos]);
	vty->ulCurrentPos++;

	return;
}

/*****************************************************************************
 函 数 名  : cmd_resolve_delete
 功能描述  : 适配Delete键，向前删除字符
 输入参数  : CMD_VTY_S *vty
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年11月17日
    作    者   : weizengke
    修改内容   : 新生成函数

*****************************************************************************/
VOID cmd_resolve_delete(CMD_VTY_S *vty)
{
	ULONG i = 0;
	ULONG size = 0;

	if (vty->ulCurrentPos >= vty->ulUsedLen)
	{
		return;
	}

	cmd_delete_one(vty);

	size = vty->ulUsedLen - vty->ulCurrentPos;

	CMD_DBGASSERT((vty->ulUsedLen >= vty->ulCurrentPos),
		"cmd_resolve_delete. (ulUsedLen=%u, ulCurrentPos=%u)",
		vty->ulUsedLen, vty->ulCurrentPos);

	memcpy(&vty->szBuffer[vty->ulCurrentPos], &vty->szBuffer[vty->ulCurrentPos + 1], size);

	vty->szBuffer[vty->ulUsedLen - 1] = '\0';

	for (i = 0; i < size; i ++)
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

VOID cmd_resolve_backspace(CMD_VTY_S *vty)
{
	ULONG i = 0;
	ULONG size = 0;

	if (vty->ulCurrentPos <= 0)
	{
		return;
	}

	size = vty->ulUsedLen - vty->ulCurrentPos;

	CMD_DBGASSERT((vty->ulUsedLen >= vty->ulCurrentPos),
		"cmd_resolve_backspace. (ulUsedLen=%u, ulCurrentPos=%u)",
		vty->ulUsedLen, vty->ulCurrentPos);

	vty->ulCurrentPos--;
	vty->ulUsedLen--;
	cmd_back_one(vty);

	memcpy(&vty->szBuffer[vty->ulCurrentPos], &vty->szBuffer[vty->ulCurrentPos + 1], size);
	vty->szBuffer[vty->ulUsedLen] = '\0';
	
	for (i = 0; i < size; i ++)
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

VOID cmd_resolve_insert(CMD_VTY_S *vty)
{
	ULONG i = 0;
	ULONG size = 0;

	if (vty->ulUsedLen >= vty->ulBufMaxLen)
	{
		CMD_debug(CMD_DEBUG_FUNC, "cmd_resolve_insert, used=%u, vtyId=%uulUsedLen(%u)>=ulBufMaxLen(%u, ulCurrentPos=%u)",
			vty->used, vty->vtyId, vty->ulUsedLen, vty->ulBufMaxLen, vty->ulCurrentPos);

		return;
	}

	if (vty->ulUsedLen < vty->ulCurrentPos)
	{
		CMD_DBGASSERT(0, "cmd_resolve_insert. (ulUsedLen=%u, ulCurrentPos=%u)", vty->ulUsedLen, vty->ulCurrentPos);
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

/*****************************************************************************
 函 数 名  : cmd_resolve_del_lastword
 功能描述  : 适配CTRL_W键，快速删除最后一个命令字
 输入参数  : CMD_VTY_S *vty
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年11月17日
    作    者   : weizengke
    修改内容   : 新生成函数

*****************************************************************************/
VOID cmd_resolve_del_lastword(CMD_VTY_S *vty)
{
	ULONG i = 0;
	ULONG size = 0;

	CMD_debug(CMD_DEBUG_FUNC, "cmd_resolve_del_lastword, ulCurrentPos=%d",vty->ulCurrentPos);

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

VOID cmd_read(CMD_VTY_S *vty)
{
	UCHAR ucKeyType = 0;

	if (NULL == vty)
	{
		return ;
	}

	while((vty->c = vty_getch(vty)) > 0)
	{
		/* check vty is online */
		if (0 == vty->used)
		{
			break;
		}

		/* 更新最后活动时间 */
		vty->user.lastAccessTime = time(NULL);

		if (CMD_VTY_CONSOLE_ID == vty->vtyId)
		{
			ucKeyType = cmd_resolve(vty);
		}
		else
		{
			ucKeyType = cmd_resolve_vty(vty);
		}

#if 0
		CMD_debug(CMD_DEBUG_INFO, "cmd_read. "
			"(vtyId=%d, used=%u, ulCurrentPos=%u, ulUsedLen=%u, c=%d, szBuffer=%s)", 
			vty->vtyId, vty->used, vty->ulCurrentPos, vty->ulUsedLen, vty->c, vty->szBuffer);
#endif

		vty->ucKeyTypeNow = ucKeyType;

		if (ucKeyType <= CMD_KEY_CODE_NONE 
			|| ucKeyType > CMD_KEY_CODE_NOTCARE)
		{
			CMD_debug(CMD_DEBUG_ERROR, "Unknow key type, c=%c, ucKeyType=%u\n", vty->c, ucKeyType);
			continue;
		}

		key_resolver[ucKeyType].pKeyCallbackfunc(vty);
		
		vty->ucKeyTypePre = vty->ucKeyTypeNow;
		
		/* not key type TAB, clear tabString */
		if (vty->ucKeyTypeNow != CMD_KEY_CODE_TAB)
		{
			memset(vty->tabString,0,sizeof(vty->tabString));
			memset(vty->tabbingString,0,sizeof(vty->tabbingString));
		}

		/* for debug */
		if (vty->ulBufMaxLen != CMD_BUFFER_SIZE
			|| vty->ulUsedLen > CMD_BUFFER_SIZE
			|| vty->ulCurrentPos > CMD_BUFFER_SIZE)
		{
			CMD_DBGASSERT(0, "cmd_read, szBuffer=%s, ulUsedLen=%u, ulCurrentPos=%u, ulBufMaxLen=%u",
				vty->szBuffer, vty->ulUsedLen, vty->ulCurrentPos,  vty->ulBufMaxLen);
		}

	}

	return ;

}

VOID vty_go(ULONG vtyId)
{
	CMD_VTY_S *vty = NULL;

	vty = cmd_vty_getById(vtyId);
	if (NULL == vty)
	{
		return;
	}

	cmd_read(vty);

	return ;
}

int cmd_init()
{
	g_pstCmdVec = cmd_vector_init();
	if(NULL == g_pstCmdVec)
	{
		return CMD_ERR;
	}

	/* 视图初始化 */
	cmd_view_init();

	g_con_vty = (CMD_VTY_S *)malloc(sizeof(CMD_VTY_S));
	if(g_con_vty == NULL)
	{
		return CMD_ERR;
	}
	memset(g_con_vty, 0, sizeof(CMD_VTY_S));
	
	cmd_vty_init(g_con_vty);
	g_con_vty->used = 1;
	g_con_vty->vtyId = CMD_VTY_CONSOLE_ID;
	g_con_vty->user.type = 0;
	g_con_vty->user.state = 1;
	g_con_vty->user.lastAccessTime = time(NULL);
	strcpy(g_con_vty->user.user_name, "CON");
	
	g_cfm_vty = (CMD_VTY_S *)malloc(sizeof(CMD_VTY_S));
	if(g_cfm_vty == NULL)
	{
		return CMD_ERR;
	}
	memset(g_cfm_vty, 0, sizeof(CMD_VTY_S));
	
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

	return 0;
}

