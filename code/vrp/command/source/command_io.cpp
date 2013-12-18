
#include "vrp\command\include\command_inc.h"


void debug_print_ex(CMD_DEBUG_TYPE_EM type, const char *format, ...)
{

	if (g_debug_switch == DEBUG_DISABLE)
	{
		return;
	}

	if (!CMD_DEBUG_TYPE_ISVALID(type))
	{
		return;
	}

	if (!CMD_DEBUGMASK_GET(type))
	{
		return;
	}


	time_t  timep = time(NULL);
	struct tm *p;

    p = localtime(&timep);
    p->tm_year = p->tm_year + 1900;
    p->tm_mon = p->tm_mon + 1;

	printf("<%04d-%02d-%02d %02d:%02d:%02d>",p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	printf("\r\n");

}


void debug_print(const char *format, ...)
{
	if (g_debug_switch == DEBUG_DISABLE)
	{
		return;
	}

	time_t  timep = time(NULL);
	struct tm *p;

    p = localtime(&timep);
    p->tm_year = p->tm_year + 1900;
    p->tm_mon = p->tm_mon + 1;

	printf("<%04d-%02d-%02d %02d:%02d:%02d>",p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	printf("\r\n");

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

void cmd_outprompt(char *prompt)
{
	//cmd_outstring("<%s-%s>", g_sysname, prompt);
	cmd_outstring("<%s>", g_sysname);
}


void cmd_debug(int level, const char *fname, const char *fmt, ...)
{
	va_list ap;
	char logbuf[1024];

	va_start(ap, fmt);
	vsprintf(logbuf, fmt, ap);
	va_end(ap);

	if (level <= CMD_LOG_LEVEL) {
		time_t now;
		struct tm *tm_now;

		if ((now = time(NULL)) < 0)
			exit(1);
		tm_now = localtime(&now);

		fprintf(stderr, "%02d:%02d:%02d: %s:%s",
			tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec,
			fname, logbuf);
	}
}

void cmd_outstring(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}


/* io */
void cmd_back_one()
{
	printf("\b");
}

void cmd_put_one(char c)
{
	printf("%c", c);
}

