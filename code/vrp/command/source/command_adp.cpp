/******************************************************************************

                  版权所有 (C), 2001-2011, 华为技术有限公司

 ******************************************************************************
  文 件 名   : command_adp.cpp
  版 本 号   : 初稿
  作    者   : weizengke
  生成日期   : 2013年11月17日
  最近修改   :
  功能描述   :  适配命令行调度
  函数列表   :
*
*       1.                cmd_resolve
*       2.                cmd_resolve_backspace
*       3.                cmd_resolve_delete
*       4.                cmd_resolve_del_lastword
*       5.                cmd_resolve_down
*       6.                cmd_resolve_enter
*       7.                cmd_resolve_insert
*       8.                cmd_resolve_left
*       9.                cmd_resolve_quest
*       10.                cmd_resolve_right
*       11.                cmd_resolve_tab
*       12.                cmd_resolve_up
*

  修改历史   :
  1.日    期   : 2013年11月17日
    作    者   : weizengke
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/


#include "..\include\command_inc.h"

/* ------------------ Interface Function ----------------- */
int cmd_resolve(char c)
{
	int key_type = CMD_KEY_CODE_NOTCARE;	// default is not special key

	//printf("c=%d,",c);
	switch (c) {
	case CMD_KEY_ARROW1:
		c = cmd_getch();

		//printf("cc=%d,",c);
		#ifdef _LINUX_
		if (c == CMD_KEY_ARROW2)
		{
			c = cmd_getch();
		#endif
			switch (c) {
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
			default:
				break;
			}
		#ifdef _LINUX_
		}
		#endif
		break;
#ifndef _LINUX_  /* windwos */
		case CMD_KEY_ARROW2:
			c = cmd_getch();
			switch (c) {
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

	case '\t':
		key_type = CMD_KEY_CODE_TAB;
		break;
	case '\r':
	case '\n':
		key_type = CMD_KEY_CODE_ENTER;
		break;
	case '?':
		/* BEGIN: Added by weizengke, 2013/10/4   PN:need print '?' */
		cmd_put_one('?');
		/* END:   Added by weizengke, 2013/10/4   PN:need print '?' */
		key_type = CMD_KEY_CODE_QUEST;
		break;
	default:
		break;
	}

	return key_type;
}
/* end key*/
/*
int cmd_resolve(char c)
{
	int key_type = CMD_KEY_CODE_NOTCARE;	// default is not special key

	switch (c) {
		case CMD_KEY_ARROW1:
			if (cmd_getch() == CMD_KEY_ARROW2) {
				c = cmd_getch();	// get real key
				switch (c) {
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
						break;
				}
			}
			break;
		case CMD_KEY_SPACE:
		case CMD_KEY_CTRL_H:
			key_type = CMD_KEY_CODE_BACKSPACE;
			break;
		case '\t':
			key_type = CMD_KEY_CODE_TAB;
			break;
		case '\r':
		case '\n':
			key_type = CMD_KEY_CODE_ENTER;
			break;
		case '?':
			key_type = CMD_KEY_CODE_QUEST;
			break;
		default:
			break;
	}

	return key_type;
}
*/

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
void cmd_resolve_tab(struct cmd_vty *vty)
{
	int i;
	cmd_vector_t *v;
	struct para_desc *match[CMD_MAX_MATCH_SIZE] = {0};	// matched string
	int match_size = 0;
	int match_type = CMD_NO_MATCH;
	int isNeedMatch = 1;   /* 非TAB场景(无空格)，不需要匹配 */
	char lcd_str[1024] = {0};	// if part match, then use this
	char *last_word = NULL;


	/*
	1: 取pos 之前的buf
	2: 需要覆盖当前光标后的buf
	*/
	/* BEGIN: Added by weizengke, 2013/11/17 bug for left and tab*/
	memset(&(vty->buffer[vty->cur_pos]), 0 ,strlen(vty->buffer) - vty->cur_pos);
	vty->used_len = strlen(vty->buffer);
	/* END:   Added by weizengke, 2013/11/17 */

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "TAB for completing command. (buf=%s)", vty->buffer);

	if (g_InputMachine_prev == CMD_KEY_CODE_TAB)
	{
		debug_print_ex(CMD_DEBUG_TYPE_FUNC, "TAB for completing command. continue tabMachine. (g_tabbingString=%s)", g_tabbingString);
		cmd_delete_word(vty);
		cmd_insert_word(vty, g_tabbingString);
	}
	else
	{
		memset(g_tabString,0,sizeof(g_tabString));
		g_tabStringLenth = 0;
	}

	v = str2vec(vty->buffer);
	if (v == NULL)
	{
		debug_print_ex(CMD_DEBUG_TYPE_INFO, "TAB for completing command. str2vec return is null. (buf=%s)", vty->buffer);
		/*
		v = cmd_vector_init(1);
		cmd_vector_insert(v, '\0');
		*/
		isNeedMatch = 0;
	}

	if (isspace((int)vty->buffer[vty->used_len - 1]))
	{
		debug_print_ex(CMD_DEBUG_TYPE_INFO, "TAB for completing command. the last one is space (buf=%s)", vty->buffer);
		isNeedMatch = 0;
	}

	if (1 == isNeedMatch && NULL != v)
	{
		match_type = cmd_match_command(v, vty, match, &match_size, lcd_str);

		last_word = (char*)cmd_vector_slot(v, cmd_vector_max(v) - 1);

		if (g_InputMachine_prev != CMD_KEY_CODE_TAB)
		{
			strcpy(g_tabbingString, last_word);
		}

		debug_print_ex(CMD_DEBUG_TYPE_INFO, "TAB for completing command. the last word is (last_word=%s, vector_size=%d)", last_word, cmd_vector_max(v) - 1);

		cmd_vector_deinit(v, 1);
	}

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "TAB for completing command. after cmd_match_command. (match_type=%d)", match_type);

	cmd_outstring("%s", CMD_ENTER);
	switch (match_type) {
		case CMD_NO_MATCH:
			cmd_outprompt(vty->prompt);
			cmd_outstring("%s", vty->buffer);
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
			cmd_outprompt(vty->prompt);
			cmd_outstring("%s", vty->buffer);

			/* BEGIN: Added by weizengke, 2013/10/27 PN: fix the bug of CMD_FULL_MATCH and then continue TAB*/
			memset(g_tabString,0,sizeof(g_tabString));
			memset(g_tabbingString,0,sizeof(g_tabbingString));
			g_tabStringLenth = 0;
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
			cmd_outprompt(vty->prompt);
			cmd_outstring("%s", vty->buffer);
			break;
		case CMD_LIST_MATCH:

			if (g_InputMachine_prev != CMD_KEY_CODE_TAB)
			{
				debug_print_ex(CMD_DEBUG_TYPE_FSM, "TAB for completing command. enter tabMachine. (g_tabString=%s)", g_tabString);
				memset(g_tabString,0,sizeof(g_tabString));
				strcpy(g_tabString, match[0]->para);
				g_tabStringLenth = strlen(g_tabString);

				/* cmd_outstring("%s", CMD_ENTER); */
			}
			else
			{
				debug_print_ex(CMD_DEBUG_TYPE_FSM, "TAB for completing command. continue tabMachine. (g_tabString=%s)", g_tabString);

				for (i = 0; i < match_size; i++)
				{
					if (0 == strcmp(g_tabString, match[i]->para))
					{
						break;
					}
				}

				debug_print_ex(CMD_DEBUG_TYPE_FSM, "TAB for completing command. continue tabMachine. (i=%d, match_size=%d)", i, match_size);

				if (i == match_size)
				{
					debug_print_ex(CMD_DEBUG_TYPE_ERROR, "TAB for completing command. bug of tab continue. (g_tabString=%s)", g_tabString);
				}

				i++;

				if (i == match_size)
				{
					i = 0;
				}

				memset(g_tabString,0,sizeof(g_tabString));
				strcpy(g_tabString, match[i]->para);
				g_tabStringLenth = strlen(g_tabString);

				debug_print_ex(CMD_DEBUG_TYPE_FSM, "TAB for completing command. continue tabMachine. (match[i]->para=%s, g_tabStringLenth=%d)", match[i]->para, g_tabStringLenth);

			}

			/*for (i = 0; i < match_size; i++) {
				if (ANOTHER_LINE(i))
					cmd_outstring("%s", CMD_ENTER);
				cmd_outstring("%-25s", match[i]->para);
			}
			*/

			cmd_delete_word(vty);
			cmd_insert_word(vty, g_tabString);

			cmd_outprompt(vty->prompt);
			cmd_outstring("%s", vty->buffer);
			break;
		default:
			break;
	}
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
void cmd_resolve_enter(struct cmd_vty *vty)
{
	struct para_desc *match[CMD_MAX_MATCH_SIZE];	// matched string
	int match_size = 0;

	int match_type = CMD_NO_MATCH;
	cmd_vector_t *v;

	int i = 0;

	//printf("enter(%d %d %s)\r\n", vty->used_len, vty->buf_len, vty->buffer);

	v = str2vec(vty->buffer);
	if (v == NULL) {
		cmd_outstring("%s", CMD_ENTER);
		cmd_outprompt(vty->prompt);
		return;
	}

	/* BEGIN: Added by weizengke, 2013/10/5   PN:for cmd end with <CR> */
	cmd_vector_insert_cr(v);
	/* END:   Added by weizengke, 2013/10/5   PN:None */

	cmd_outstring("%s", CMD_ENTER);

	// do command
	match_type = cmd_execute_command(v, vty, match, &match_size);
	// add executed command into history
	cmd_vty_add_history(vty);

	if (match_type == CMD_NO_MATCH)
	{
		cmd_outstring("Unknown Command");
		cmd_outstring("%s", CMD_ENTER);
	}
	else if (match_type == CMD_ERR_ARGU)
	{
		cmd_outstring("Too Many Arguments");
		cmd_outstring("%s", CMD_ENTER);
	}

	if (match_type == CMD_ERR_AMBIGOUS)
	{
		if (match_size)
		{
			cmd_outstring("Command '%s' anbigous follow:", vty->buffer);
			cmd_outstring("%s", CMD_ENTER);
			for (i = 0; i < match_size; i++)
			{
				if (ANOTHER_LINE(i))
					cmd_outstring("%s", CMD_ENTER);
				cmd_outstring(" %-25s", match[i]->para);
			}
			cmd_outstring("%s", CMD_ENTER);
			/* del 10-29
			cmd_outprompt(vty->prompt);
			cmd_outstring("%s", vty->buffer);
			*/
			vty->cur_pos = vty->used_len = 0;
			memset(vty->buffer, 0, vty->buf_len);
			cmd_outprompt(vty->prompt);
		}
	}
	else
	{
		// ready for another command
		vty->cur_pos = vty->used_len = 0;
		memset(vty->buffer, 0, vty->buf_len);
		cmd_outprompt(vty->prompt);
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
void cmd_resolve_quest(struct cmd_vty *vty)
{
	cmd_vector_t *v;
	struct para_desc *match[CMD_MAX_MATCH_SIZE];	// matched string
	int match_size = 0;
	int i = 0;

	/*
	1: 取pos 之前的buf
	2: 需要覆盖当前光标后的buf
	*/
	/* BEGIN: Added by weizengke, 2013/11/17 bug for left and tab*/
	memset(&(vty->buffer[vty->cur_pos]), 0 ,strlen(vty->buffer) - vty->cur_pos);
	vty->used_len = strlen(vty->buffer);
	/* END:   Added by weizengke, 2013/11/17 */


	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "'?' for associating command. (buf=%s)", vty->buffer);

	v = str2vec(vty->buffer);
	if (v == NULL)
	{
		debug_print_ex(CMD_DEBUG_TYPE_INFO, "'?' for associating command. after str2vec, v is null (buf=%s)", vty->buffer);

		v = cmd_vector_init(1);
		cmd_vector_insert_cr(v);
	}
	else if (isspace((int)vty->buffer[vty->used_len - 1]))
	{
		debug_print_ex(CMD_DEBUG_TYPE_INFO, "'?' for associating command. the last one is space (buf=%s)", vty->buffer);
		cmd_vector_insert_cr(v);
	}

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "In cmd_resolve_quest, after str2vec, get %d vector.", v->size);

	for (i = 0; i < cmd_vector_max(v); i++)
	{
		debug_print_ex(CMD_DEBUG_TYPE_INFO, "In cmd_resolve_quest, str=%s.", (char*)cmd_vector_slot(v, i));
	}

	cmd_complete_command(v, vty, match, &match_size);

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "In cmd_resolve_quest, after cmd_complete_command, get %d matched.", match_size);

	cmd_outstring("%s", CMD_ENTER);
	if (match_size) {
		for (i = 0; i < match_size; i++) {
			cmd_outstring(" %-25s%s\r\n", match[i]->para,match[i]->desc);
		}
		cmd_outprompt(vty->prompt);
		cmd_outstring("%s", vty->buffer);
	} else {
		cmd_outstring("Unknow Command%s", CMD_ENTER);
		cmd_outprompt(vty->prompt);
		cmd_outstring("%s", vty->buffer);
	}

	cmd_vector_deinit(v, 0);
}

/* bug of up twice with last key is not up, the hpos not restart */
void cmd_resolve_up(struct cmd_vty *vty)
{
	int try_idx = vty->hpos == 0 ? (HISTORY_MAX_SIZE - 1) : vty->hpos - 1;

	// if no history
	if (vty->history[try_idx] == NULL)
		return;
	vty->hpos = try_idx;

	// print history command
	cmd_clear_line(vty);
	strcpy(vty->buffer, vty->history[vty->hpos]);
	vty->cur_pos = vty->used_len = strlen(vty->history[vty->hpos]);
	cmd_outstring("%s", vty->buffer);
}

void cmd_resolve_down(struct cmd_vty *vty)
{
	int try_idx = vty->hpos ==( HISTORY_MAX_SIZE - 1) ? 0 : vty->hpos + 1;

	// if no history
	if (vty->history[try_idx] == NULL)
		return;
	vty->hpos = try_idx;

	// print history command
	cmd_clear_line(vty);
	strcpy(vty->buffer, vty->history[vty->hpos]);
	vty->cur_pos = vty->used_len = strlen(vty->history[vty->hpos]);
	cmd_outstring("%s", vty->buffer);
}

// handle in read buffer, including left, right, delete, insert
void cmd_resolve_left(struct cmd_vty *vty)
{
	// already at leftmost, cannot move more
	if (vty->cur_pos <= 0)
		return;
	// move left one step
	vty->cur_pos--;
	cmd_back_one();
}

void cmd_resolve_right(struct cmd_vty *vty)
{
	// already at rightmost, cannot move more
	if (vty->cur_pos >= vty->used_len)
		return;
	// move right one step
	cmd_put_one(vty->buffer[vty->cur_pos]);
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
void cmd_resolve_delete(struct cmd_vty *vty)
{
	int i, size;

	// no more to delete
	if (vty->cur_pos >= vty->used_len)
		return;

	/* del the  current char*/
	printf(" \b");

	size = vty->used_len - vty->cur_pos;
	CMD_DBGASSERT(size >= 0);

	memcpy(&vty->buffer[vty->cur_pos], &vty->buffer[vty->cur_pos + 1], size);
	vty->buffer[vty->used_len - 1] = '\0';

	/* output the right chars */
	for (i = 0; i < size; i ++)
		cmd_put_one(vty->buffer[vty->cur_pos + i]);

	vty->used_len--;

	/* back the cur_pos */
	for (i = 0; i < size; i++)
		cmd_back_one();


}

void cmd_resolve_backspace(struct cmd_vty *vty)
{
	int i, size;

	// no more to delete
	if (vty->cur_pos <= 0)
		return;
	size = vty->used_len - vty->cur_pos;
	CMD_DBGASSERT(size >= 0);

	// delete char
	vty->cur_pos--;
	vty->used_len--;
	cmd_back_one();

	// print left chars
	memcpy(&vty->buffer[vty->cur_pos], &vty->buffer[vty->cur_pos + 1], size);
	vty->buffer[vty->used_len] = '\0';
	for (i = 0; i < size; i ++)
		cmd_put_one(vty->buffer[vty->cur_pos + i]);
	cmd_put_one(' ');
	for (i = 0; i < size + 1; i++)
		cmd_back_one();

}

void cmd_resolve_insert(struct cmd_vty *vty)
{
	int i, size;

	// no more to insert
	if (vty->used_len >= vty->buf_len)
		return;
	size = vty->used_len - vty->cur_pos;
	CMD_DBGASSERT(size >= 0);

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
		cmd_put_one(vty->buffer[vty->cur_pos + i]);
	for (i = 0; i < size; i++)
		cmd_back_one();

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
void cmd_resolve_del_lastword(struct cmd_vty *vty)
{
	int i, size;

	// no more to delete
	if (vty->cur_pos <= 0)
		return;

	cmd_delete_word_ctrl_W(vty);

	cmd_outstring("%s", CMD_ENTER);
	cmd_outprompt(vty->prompt);
	cmd_outstring("%s", vty->buffer);

}


key_handler_t key_resolver[] = {
	// resolve a whole line
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


