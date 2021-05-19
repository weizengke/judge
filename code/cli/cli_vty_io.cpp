
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

#include "pdt_common_inc.h"
#include "ic/include/debug_center_inc.h"

VOID vty_printf(ULONG vtyId, CHAR *format, ...)
{
	ULONG ret = 0;
	char *buffer = (char*)malloc(BUFSIZE);
	if (buffer == NULL) {
		return;
	}
	memset(buffer, 0, BUFSIZE);

	CMD_VTY_S *vty = cmd_vty_getById(vtyId);
	if (NULL == vty) {
		free(buffer);
		return;
	}

	va_list args;
	va_start(args, format);

	if (CMD_VTY_CONSOLE_ID == vtyId) {
		vprintf(format, args);
	} else {
		vsnprintf(buffer, BUFSIZE, format, args);
		(VOID)cmd_socket_send(vty->user.socket, buffer, strlen(buffer), 0);
	}

	va_end(args);

	free(buffer);

	return;
}

VOID cmd_vty_printf(CMD_VTY_S *vty, CHAR *format, ...)
{
	ULONG ret = 0;
	char *buffer = (char*)malloc(BUFSIZE);
	if (buffer == NULL) {
		return;
	}
	memset(buffer, 0, BUFSIZE);

	va_list args;
	va_start(args, format);

	if (CMD_VTY_CONSOLE_ID == vty->vtyId) {
		vprintf(format, args);
	} else {
		vsnprintf(buffer, BUFSIZE, format, args);
		(VOID)cmd_socket_send(vty->user.socket, buffer, strlen(buffer), 0);
	}

	va_end(args);

	free(buffer);

	return;
}

VOID vty_print2all(CHAR *format, ...)
{
	ULONG ret = 0;
    CMD_VTY_S *vty = NULL;
	char *buffer = (char*)malloc(BUFSIZE);
	if (buffer == NULL) {
		return;
	}
	memset(buffer, 0, BUFSIZE);

	va_list args;
	va_start(args, format);

	/* com */
    vty = cmd_vty_getById(CMD_VTY_CONSOLE_ID);
	if (NULL != vty
		&& vty->used
		&& vty->user.state
		&& vty->user.terminal_debugging) {
		vprintf(format, args);
	}

	/* vty */
	vsnprintf(buffer, BUFSIZE, format, args);
	for (int i = 0; i < CMD_VTY_MAXUSER_NUM; i++) {
        vty = cmd_vty_getById(i);
		if (vty != NULL && vty->used && vty->user.state && vty->user.terminal_debugging) {
			(VOID)cmd_socket_send(vty->user.socket, buffer, strlen(buffer), 0);
		}
	}

	va_end(args);

	free(buffer);
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

UCHAR cmd_getch()
{
	UCHAR c = 0;
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

UCHAR vty_getchar(CMD_VTY_S *vty)
{
	int ret = 0;
	UCHAR buff = 0;

	if (vty->vtyId < CMD_VTY_MAXUSER_NUM) {
		/* vty terminal */
		ret = cmd_socket_recv(vty->user.socket, (char *)&buff, 1, 0);
		if (ret <= 0) {
			return 0;
		}
	} else {
		/* local terminal */
		buff = cmd_getch();
	}

	return buff;
}

UCHAR cmd_parse_vty(CMD_VTY_S *vty)
{
	UCHAR c = vty->c;
	ULONG key_type = CMD_KEY_CODE_INSERT;

	switch (c)
	{
	case 0x1b:
		c = vty_getchar(vty);
		if (0x5b == c)
		{
			c = vty_getchar(vty);
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
	case CMD_KEY_BACKSPACE_VTY:
		key_type = CMD_KEY_CODE_BACKSPACE;
		break;
	case CMD_KEY_SPACE:
		/* Linux 下空格后回车无法tab补全 '联想 待修 */
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
		else
		{
		}
		/* END:   Added by weizengke, 2014/4/6 */
		break;
	}

	return key_type;
}

UCHAR cmd_parse_console(CMD_VTY_S *vty)
{
	UCHAR c = vty->c;
	ULONG key_type = CMD_KEY_CODE_INSERT;

	switch (c)
	{
	case CMD_KEY_ARROW1:
		c = vty_getchar(vty);

#ifdef _LINUX_
		if (c == CMD_KEY_ARROW2)
		{
			c = vty_getchar(vty);
			if (c == CMD_KEY_ARROW3)
			{
				c = vty_getchar(vty);
			}
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
				cmd_page_up();
				key_type = CMD_KEY_CODE_FILTER;
			}
			break;
			case CMD_KEY_PHDN:
			{
				cmd_page_down();
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
#ifndef _LINUX_ /* windwos */
	case CMD_KEY_ARROW2:
		c = vty_getchar(vty);
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
	case CMD_KEY_BACKSPACE: /*  */
		key_type = CMD_KEY_CODE_BACKSPACE;
		break;
	case CMD_KEY_SPACE:
#if 0
	case CMD_KEY_CTRL_H:
#endif
		/* Linux 下空格后回车无法tab补全 '联想 待修 */
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

UCHAR cmd_parse(CMD_VTY_S *vty)
{
	if (CMD_VTY_CONSOLE_ID == vty->vtyId) {
		return cmd_parse_console(vty);
	} else {
		return cmd_parse_vty(vty);
	}
}

