#include <windows.h>
#include <iostream>
#include <conio.h>
#include <stdlib.h>
#include <io.h>
#include <time.h>
#include <queue>
#include <string>
#include <sstream>

#include "product\judge\include\judge_inc.h"

extern char g_sysname[];

void write_log(int level, const char *fmt, ...) {
	va_list ap;
	char buffer[BUFSIZE];
	time_t  timep = time(NULL);
	int l;
	struct tm *p;
    p = localtime(&timep);
    p->tm_year = p->tm_year + 1900;
    p->tm_mon = p->tm_mon + 1;
	sprintf(buffer,"log/%04d-%02d-%02d.log", p->tm_year, p->tm_mon, p->tm_mday);

	FILE *fp = fopen(buffer, "a+");
	if (fp == NULL) {
		fprintf(stderr, "open logfile error!\n");
		return;
	}

	fprintf(fp, "%s:%04d-%02d-%02d %02d:%02d:%02d ",LEVEL_NAME[level],p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);

	va_start(ap, fmt);
	l = vsnprintf(buffer, BUFSIZE, fmt, ap);
	fprintf(fp, "%s\r\n", buffer);
	if (g_oj_debug_switch == JUDGE_DEBUG_ON)
	{
		/* BEGIN: Added by weizengke, 2013/11/15 for vrp */
		//pdt_debug_print("%s", buffer);
		Judge_Debug(DEBUG_TYPE_INFO, "%s", buffer);
		/* END:   Added by weizengke, 2013/11/15 */
	}
	va_end(ap);
	fclose(fp);
}


void judge_outstring(const char *format, ...)
{
	extern int g_pdt_recovering;

	if (OS_YES == g_pdt_recovering)
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


int read_buffer(const char *filename, char * buffer, int buf_size)
{
	if (NULL == filename || NULL == buffer)
	{
		return 0;
	}
	FILE * fp=fopen(filename,"r");
	char tmp[4096] = {0};
	int size_tmp = 0;
	char *buf = NULL;

	if (fp == NULL)
	{
		return 0;
	}

	buf = buffer;
	while(fgets(tmp, 4096 ,fp))
	{

		buf += sprintf(buf,"%s",tmp);
		if (strlen(buffer) >= buf_size - 5)
		{
			sprintf(&buffer[buf_size - 5],"...");
			break;
		}
	}

	while (strlen(buffer) > 0 && buffer[strlen(buffer) - 1] == '\n')
	{
		buffer[strlen(buffer) - 1] = '\0';
	}

	fclose(fp);
	return 0;
}

int reset_file(const char *filename)
{
	if (NULL == filename)
	{
		return 0;
	}
	FILE *fp = fopen(filename, "w");

	if (fp == NULL)
	{
		return 0;
	}

	fprintf(fp,"");

	fclose(fp);

	return 0;
}

int write_buffer(const char *filename, const char *fmt, ...)
{
	va_list ap;
	int l;
	if (NULL == filename)
	{
		return 0;
	}

	FILE *fp = fopen(filename, "a+");
	char buffer[BUFSIZE] = {0};
	if (fp == NULL)
	{
		return 0;
	}

	va_start(ap, fmt);
	l = vsnprintf(buffer, BUFSIZE, fmt, ap);
	fprintf(fp, "%s", buffer);
	va_end(ap);

	fclose(fp);

	return 0;
}
