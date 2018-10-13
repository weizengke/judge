
#include "product/judge/include/judge_inc.h"

#if (OS_YES == OSP_MODULE_JUDGE)

void judge_outstring(const char *format, ...)
{
	extern ULONG SYSMNG_IsCfgRecoverOver();

	if (OS_NO == SYSMNG_IsCfgRecoverOver())
	{
		return ;
	}
	
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

void MSG_OUPUT_DBG(const char *fmt, ...)
{
	va_list ap;
	char buffer[BUFSIZE];
	time_t  timep = time(NULL);
	int l;
	struct tm *p;

	if (JUDGE_DEBUG_OFF == g_oj_debug_switch)
	{
		return;
	}

    p = localtime(&timep);
    p->tm_year = p->tm_year + 1900;
    p->tm_mon = p->tm_mon + 1;

	printf("\r\n%04d-%02d-%02d %02d:%02d:%02d ",p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);

	va_start(ap, fmt);
	l = vsnprintf(buffer, BUFSIZE, fmt, ap);

	printf("\r\n%s", buffer);
	
	va_end(ap);

}




#endif
