#include <iostream>
#include <sstream>  
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>

#ifdef _LINUX_
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <termios.h>
#include <assert.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#endif

#ifdef WIN32
#include <io.h>
#include <winsock2.h>

#endif

#include "kernel.h"
#include "util.h"

using namespace std;

char* util_strdup(const char *s)
{
	size_t len =strlen (s) + 1;
	void *str = malloc (len);

	if (str == NULL){
		return NULL;
	}
		
	return (char *)memcpy (str, s, len);
}

int util_stricmp(const char *dst, const char *src)
{
	int ch1, ch2;

	do{
		if (((ch1 = (unsigned char)(*(dst++))) >= 'A') &&(ch1 <= 'Z')){
			ch1 += 0x20;
		}
		
		if (((ch2 = (unsigned char)(*(src++))) >= 'A') &&(ch2 <= 'Z')){
			ch2 += 0x20;
		}
	} while (ch1 && (ch1 == ch2));

	return(ch1 - ch2);
}


int util_strnicmp(const char *dst, const char *src, int count)
{
	int ch1, ch2;

	do{
		if (((ch1 = (unsigned char)(*(dst++))) >= 'A') &&(ch1 <= 'Z')){
			ch1 += 0x20;
		}
		
		if (((ch2 = (unsigned char)(*(src++))) >= 'A') &&(ch2 <= 'Z')){
			ch2 += 0x20;
		}
	} while (--count && ch1 && (ch1 == ch2));

	return (ch1 - ch2);
}


/* ascii?10???????? */
char util_ascii_to_byte(char b)
{	
	char ret = 0;
	
	if(b >= '0' && b <= '9')
		ret = b - '0';
	else if(b >= 'A' && b <= 'F')
		ret = b - 'A' + 10;
	else if(b >= 'a' && b <= 'f')
		ret = b - 'a' + 10;
	else
		ret = 0;
	
	return ret;
}

/* ?????????????????? */
long util_strtol(char *str, int base)
{	
	long ret = 0;	
	int count = strlen(str);
	
	while(count-- > 0)
	{
		ret *= base;
		ret += util_ascii_to_byte(*str++);
	}
	
	return ret;
}

int util_freset(const char *filename)
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

int util_fread(const char *filename, char * buffer, int buf_size)
{
	if (NULL == filename || NULL == buffer) {
		return 0;
	}
	FILE * fp=fopen(filename,"r");
	char tmp[4096] = {0};
	int size_tmp = 0;
	char *buf = NULL;

	if (fp == NULL) {
		return 0;
	}

	buf = buffer;
	while(fgets(tmp, 4095 ,fp)) {
		if (strlen(buffer) + strlen(tmp) >= (buf_size - 5)) {
			sprintf(&buffer[buf_size - 5],"...");
			break;
		}

		buf += sprintf(buf,"%s",tmp);
	}

	while (strlen(buffer) > 0 && buffer[strlen(buffer) - 1] == '\n') {
		buffer[strlen(buffer) - 1] = '\0';
	}

	fclose(fp);
	return 0;
}

int util_fwrite(const char *filename, const char *fmt, ...)
{
	va_list ap;
	int l;
	if (NULL == filename)
	{
		return 0;
	}

	FILE *fp = fopen(filename, "a+");
	char buffer[UTIL_BUFSIZE + 1] = {0};
	if (fp == NULL)
	{
		return 0;
	}

	va_start(ap, fmt);
	l = vsnprintf(buffer, UTIL_BUFSIZE, fmt, ap);
	fprintf(fp, "%s", buffer);
	va_end(ap);

	fclose(fp);

	return 0;
}

int util_remove(char * filename)
{
	return remove(filename);
}

int util_string_to_time(const string &string_time,time_t &time_data)
{
    char *pBeginPos = (char*) string_time.c_str();
    char *pPos = strstr(pBeginPos,"-");
	
    if(pPos == NULL)
    {
        return -1;
    }
	
    int iYear = atoi(pBeginPos);
    int iMonth = atoi(pPos + 1);
	
    pPos = strstr(pPos + 1,"-");
    if(pPos == NULL)
    {
        return -1;
    }
	
    int iDay = atoi(pPos + 1);
    int iHour=0;
    int iMin=0;
    int iSec=0;
	
    pPos = strstr(pPos + 1," ");
    //?????????�???????????
    if(pPos != NULL)
    {
        iHour=atoi(pPos + 1);
        pPos = strstr(pPos + 1,":");
        if(pPos != NULL)
        {
            iMin=atoi(pPos + 1);
            pPos = strstr(pPos + 1,":");
            if(pPos != NULL)
            {
                iSec=atoi(pPos + 1);
            }
        }
    }

    struct tm sourcedate;
    memset((void*)&sourcedate,0,sizeof(sourcedate));
    sourcedate.tm_sec = iSec;
    sourcedate.tm_min = iMin;
    sourcedate.tm_hour = iHour;
    sourcedate.tm_mday = iDay;
    sourcedate.tm_mon = iMonth - 1;
    sourcedate.tm_year = iYear - 1900;
	
    time_data = mktime(&sourcedate);
	
    return 0;
}


int util_time_to_string(string &time_string,const time_t &time_data)
{
    char chTmp[25];
    memset(chTmp,0,sizeof(chTmp));

    struct tm *p;
    p = localtime(&time_data);

    p->tm_year = p->tm_year + 1900;

    p->tm_mon = p->tm_mon + 1;

	sprintf(chTmp,"%04d-%02d-%02d %02d:%02d:%02d",p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);

    time_string = chTmp;
    return 0;
}

long util_getdiftime(time_t maxt,time_t mint)  //??????? ????????
{
	return (long)difftime(maxt,mint);
}

#ifdef WIN32
int util_get_directory_info(const string &string_dir, string &string_result)
{
	_finddata_t    fileInfo;
	string         strSearch = string_dir + "\\*.*";
	long           handle  = 0;
	char chStrHead[256] = {0};
	
	if (string_dir.empty() || file_access(string_dir.c_str(), 0) != 0) {
		return 1;
	}

	string_result.clear();

	handle =_findfirst(strSearch.c_str(), &fileInfo);
	if (-1 ==handle) {
		return 1;
	}

	sprintf(chStrHead, " %-4s %14s  %-10s %-8s  %-10s", "Attr", "Size(Byte)", "Date", "Time", "FileName");
	string_result = string_result + chStrHead;
		
	while (0 ==_findnext(handle, &fileInfo)) {
		string str;
		char chStrTmp[256];
		char chTmp[25];
		memset(chTmp,0,sizeof(chTmp));	   
		struct tm *p;
		p = localtime(&fileInfo.time_write);	   
		p->tm_year = p->tm_year + 1900;	   
		p->tm_mon = p->tm_mon + 1;	   
		sprintf(chTmp,"%04d-%02d-%02d %02d:%02d:%02d",p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);

		if ((fileInfo.attrib&_A_SUBDIR) == _A_SUBDIR) {
			sprintf(chStrTmp, "\r\n %-4s %14s  %-20s %-10s",
				"drw-", "-", chTmp, fileInfo.name);

			str = chStrTmp;
		  	string_result = string_result + str;
		}

		if ((fileInfo.attrib&_A_ARCH) == _A_ARCH) {
			sprintf(chStrTmp, "\r\n %-4s %14u  %-20s %-10s",
					"-rw-", fileInfo.size, chTmp, fileInfo.name);

			str = chStrTmp;
		  	string_result = string_result + str;
		}
	}

	string_result += "\r\n";
	
    _findclose(handle);

	return 0;
}
#endif

#ifdef _LINUX_
int util_get_directory_info(const string &string_dir, string &string_result)
{
	DIR *dir;
	struct dirent *ptr;
	char base[1000];
	char chStrHead[256] = {0};
	
	string_result.clear();

	sprintf(chStrHead, " %-4s %14s  %-10s %-8s  %-10s", "Attr", "Size(Byte)", "Date", "Time", "FileName");
	string_result = string_result + chStrHead;
	
	if ((dir = opendir(string_dir.c_str())) == NULL) {
		return 1;
	}
 
	while ((ptr=readdir(dir)) != NULL) {
		string str;
		char chStrTmp[256];
		char chTmp[25];
		memset(chTmp,0,sizeof(chTmp));
		struct stat st;

		/*
		if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)
		{
			continue;
		}
		
		else */if(ptr->d_type == 8)	 /* file */
		{
			int ret = stat(ptr->d_name,&st);
			if(ret == -1) {
				continue;
			}

			int size = (int)st.st_size;
			
			struct tm *p;
			p = localtime(&st.st_mtime);	   
							p->tm_year = p->tm_year + 1900;    
							p->tm_mon = p->tm_mon + 1;	   
							sprintf(chTmp,"%04d-%02d-%02d %02d:%02d:%02d",
							p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
			
			sprintf(chStrTmp, "\r\n %-4s %14u  %-20s %-10s",
					"-rw-", st.st_size, chTmp, ptr->d_name);

			str = chStrTmp;
		  	string_result = string_result + str;

		} else if(ptr->d_type == 4)	{	 ///dir 
			int ret = stat(ptr->d_name,&st);
			if(ret == -1) {
				continue;
			}

			struct tm *p;
			p = localtime(&st.st_mtime);	   
							p->tm_year = p->tm_year + 1900;    
							p->tm_mon = p->tm_mon + 1;	   
							sprintf(chTmp,"%04d-%02d-%02d %02d:%02d:%02d",
							p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
							
			sprintf(chStrTmp, "\r\n %-4s %14s  %-20s %-10s",
				"drw-", "-", chTmp, ptr->d_name);
		
			str = chStrTmp;
			string_result = string_result + str;
			
		}
	}

	string_result = string_result + "\r\n";
	
	closedir(dir);

	return 0;
}
#endif

int util_ini_get_string(char *title, char *key, char *buff_default,
							char *buff_return, int buff_size, char *filename) 
{ 
	FILE *fp; 
	char szLine[1024] = {0};
	static char tmpstr[1024] = {0};
	int rtnval;
	int i = 0; 
	int flag = 0; 
	char *tmp;
 
	if((fp = fopen(filename, "r")) == NULL) 
	{ 
		memcpy(buff_return, buff_default, strlen(buff_default));
		return strlen(buff_default); 
	}
	
	while(!feof(fp)) 
	{ 
		rtnval = fgetc(fp); 
		if(rtnval == EOF) 
		{ 
			break; 
		} 
		else 
		{ 
			szLine[i++] = rtnval; 
		} 
		if(rtnval == '\n') 
		{ 
#ifndef WIN32
			i--;
#endif	
			szLine[--i] = '\0';
			i = 0; 
			tmp = strchr(szLine, '='); 
 
			if(( tmp != NULL )&&(flag == 1)) 
			{ 
				if(strstr(szLine,key)!=NULL) 
				{ 
					if ('#' == szLine[0])
					{
					}
					else if ( '/' == szLine[0] && '/' == szLine[1] )
					{
						
					}
					else
					{
						strcpy(tmpstr,tmp+1); 
						fclose(fp);

						memcpy(buff_return, tmpstr, strlen(tmpstr));
						return strlen(tmpstr); 
					}
				} 
			}
			else 
			{ 
				strcpy(tmpstr,"["); 
				strcat(tmpstr,title); 
				strcat(tmpstr,"]");
				if( strncmp(tmpstr,szLine,strlen(tmpstr)) == 0 ) 
				{
					flag = 1; 
				}
			}
		}
	}
	
	fclose(fp); 

	memcpy(buff_return, buff_default, strlen(buff_default));
	
	return strlen(buff_default); 
}
 
int util_ini_get_int(char *title,char *key, int value_default, char *filename)
{
	char szBuf[1024] = {0};
	
	int v = util_ini_get_string(title, key, "", szBuf, sizeof(szBuf), filename);
	
	if (0 == v)
	{
		return value_default;
	}

	return atoi(szBuf);
}

char* util_rindex(const char *s, int c)
{
	char *r = NULL;
	char *s_ = (char *)s;

	if (s == NULL){
		return NULL;
	}

	while (*s_ != '\0'){
		if (*s_ == c){
			r = s_;
		}
		
		s_++;
	}

	return r;
}

void util_split_whole_name(const char *whole_name, char *fname, char *ext)
{
	char *p_ext;

	p_ext = util_rindex(whole_name, '.');
	if (NULL != p_ext) {
		strcpy(ext, p_ext);
		snprintf(fname, p_ext - whole_name + 1, "%s", whole_name);
	} else {
		ext[0] = '\0';
		strcpy(fname, whole_name);
	}
}

void util_splitpath(const char *path, char *drive, char *dir, char *fname, char *ext)
{
	char *p_whole_name;

	drive[0] = '\0';
	if (NULL == path) {
		dir[0] = '\0';
		fname[0] = '\0';
		ext[0] = '\0';
		return;
	}

	if ('/' == path[strlen(path)]) {
		strcpy(dir, path);
		fname[0] = '\0';
		ext[0] = '\0';
		return;
	}

	p_whole_name = util_rindex(path, '/');
	if (NULL != p_whole_name) {
		p_whole_name++;
		util_split_whole_name(p_whole_name, fname, ext);

		snprintf(dir, p_whole_name - path, "%s", path);
	} else {
		util_split_whole_name(path, fname, ext);
		dir[0] = '\0';
	}
}
