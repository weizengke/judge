
#include "osp\command\include\command_inc.h"

void vty_printf(struct cmd_vty *vty, const char *format, ...)
{	
	int ret = 0;
	char buffer[BUFSIZE] = {0};
	
	va_list args;
	va_start(args, format);

	/* 0:COM, 1:vty */
	if (0 == vty->user.type)
	{
		vprintf(format, args);
	}
	else
	{
		vsnprintf(buffer, BUFSIZE, format, args);
		(void)send(vty->user.socket, buffer, strlen(buffer),0);
	}
	
	va_end(args);

	return;
}

void vty_print2all(const char *format, ...)
{
	int ret = 0;
	char buffer[BUFSIZE] = {0};
	
	va_list args;
	va_start(args, format);

	/* com */
	vprintf(format, args);

	/* vty */
	vsnprintf(buffer, BUFSIZE, format, args);
	for (int i = 0; i < CMD_VTY_MAXUSER_NUM; i++)
	{
		if (g_vty[i].valid
			&& g_vty[i].user.state
			&& g_vty[i].user.terminal_debugging)
		{
			(void)send(g_vty[i].user.socket, buffer, strlen(buffer),0);
		}
	}
	
	va_end(args);
}


int cmd_getch()
{
	int c = 0;
	#ifdef _LINUX_
	struct termios org_opts, new_opts;
	int res = 0;
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
	char buff[1] = {0};

	/* 串口模式 */
	if (0 == vty->user.type)
	{
		buff[0] = cmd_getch();
	}
	else
	{	
		/* telnet模式 */
		ret = recv(vty->user.socket, (char*)buff, 1, 0);
		if (ret <= 0)
		{
			return -1;
		}
	}

	CMD_debug(DEBUG_TYPE_INFO, "vty_getch. (c=0x%x)", buff[0]);
	
	return buff[0];
}

void cmd_outstring(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}


/* io */
void cmd_delete_one(struct cmd_vty *vty)
{
	if (vty->user.type == 0)
	{
		vty_printf(vty, " \b");
	}
	else
	{
		vty_printf(vty, "  \b");
	}
	
}

void cmd_back_one(struct cmd_vty *vty)
{
	//vty_printf(vty, "\b");
	vty_printf(vty, "\b");
}

void cmd_put_one(struct cmd_vty *vty, char c)
{
	vty_printf(vty, "%c", c);
}

