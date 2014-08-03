
#include "osp\command\include\command_inc.h"


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
void cmd_resolve_enter_ex(char* buf)
{
	struct para_desc *match[CMD_MAX_MATCH_SIZE];	// matched string
	int match_size = 0;

	int match_type = CMD_NO_MATCH;
	cmd_vector_t *v;

	int i = 0;
	int nomath_pos = -1;

	//printf("enter(%d %d %s)\r\n", vty->used_len, vty->buf_len, vty->buffer);

	strcpy(vty->buffer, buf);

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
	match_type = cmd_execute_command(v, vty, match, &match_size, &nomath_pos);
	// add executed command into history
	cmd_vty_add_history(vty);

	if (match_type == CMD_NO_MATCH)
	{
		cmd_output_missmatch(vty, nomath_pos);
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


/* ------------------ Interface Function ----------------- */
int cmd_resolve(char c)
{
	int key_type = CMD_KEY_CODE_NOTCARE;	// default is not special key
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
						::SendMessage(g_hWnd,WM_VSCROLL,MAKEWPARAM(SB_PAGEUP, 0),NULL);
						/*
						int nVertSP = GetScrollPos(g_hWnd, SB_VERT);
						SetScrollPos(g_hWnd, SB_VERT, nVertSP, 1);
						*/
						key_type = CMD_KEY_CODE_FILTER;
					}
					break;
				case CMD_KEY_PHDN:
					{
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

void cmd_resolve_filter(struct cmd_vty *vty)
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

	debug_print_ex(CMD_DEBUG_TYPE_INFO,"\r\ntab in:used_len=%d, pos=%d\r\n",vty->used_len, vty->cur_pos);


	if (g_InputMachine_prev == CMD_KEY_CODE_TAB)
	{
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

		if (g_InputMachine_prev != CMD_KEY_CODE_TAB)
		{
			strcpy(g_tabbingString, last_word);
		}

		cmd_vector_deinit(v, 1);
	}

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
				memset(g_tabString,0,sizeof(g_tabString));
				strcpy(g_tabString, match[0]->para);
				g_tabStringLenth = strlen(g_tabString);

				/* cmd_outstring("%s", CMD_ENTER); */
			}
			else
			{
				for (i = 0; i < match_size; i++)
				{
					if (0 == strcmp(g_tabString, match[i]->para))
					{
						break;
					}
				}

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


	debug_print_ex(CMD_DEBUG_TYPE_INFO,"\r\ntab out:used_len=%d, pos=%d\r\n",vty->used_len, vty->cur_pos);
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
	int nomath_pos = -1;

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
	match_type = cmd_execute_command(v, vty, match, &match_size, &nomath_pos);
	// add executed command into history
	cmd_vty_add_history(vty);

	if (match_type == CMD_NO_MATCH)
	{
		cmd_output_missmatch(vty, nomath_pos);
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
	int nomath_pos = -1;

	/*
	1: 取pos 之前的buf
	2: 需要覆盖当前光标后的buf
	*/
	/* BEGIN: Added by weizengke, 2013/11/17 bug for left and tab*/
	memset(&(vty->buffer[vty->cur_pos]), 0 ,strlen(vty->buffer) - vty->cur_pos);
	vty->used_len = strlen(vty->buffer);
	/* END:   Added by weizengke, 2013/11/17 */

	v = str2vec(vty->buffer);
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

	cmd_outstring("%s", CMD_ENTER);

	if (match_size) {
		for (i = 0; i < match_size; i++) {
			cmd_outstring(" %-25s%s\r\n", match[i]->para,match[i]->desc);
		}
		cmd_outprompt(vty->prompt);
		cmd_outstring("%s", vty->buffer);
	} else {

		cmd_output_missmatch(vty, nomath_pos);

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
	CMD_DBGASSERT(size >= 0, "cmd_resolve_delete");

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
	CMD_DBGASSERT(size >= 0, "cmd_resolve_backspace");

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

	debug_print_ex(CMD_DEBUG_TYPE_INFO,"\r\ninsert in:used_len=%d, pos=%d, buffer_len=%d\r\n",vty->used_len, vty->cur_pos, strlen(vty->buffer));

	CMD_DBGASSERT(size >= 0, "cmd_resolve_insert");

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

	debug_print_ex(CMD_DEBUG_TYPE_INFO,"\r\ninsert out:used_len=%d, pos=%d, buffer_len=%d\r\n",vty->used_len, vty->cur_pos, strlen(vty->buffer));

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

	cmd_delete_word_ctrl_W_ex(vty);

	cmd_outstring("%s", CMD_ENTER);
	cmd_outprompt(vty->prompt);
	cmd_outstring("%s", vty->buffer);

	/* BEGIN: Added by weizengke, 2014/3/23 support delete word form cur_pos*/
	for (i = 0; i < strlen(vty->buffer) - vty->cur_pos; i++)
	{
		cmd_back_one();
	}
	/* END:   Added by weizengke, 2014/3/23 */

}


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


