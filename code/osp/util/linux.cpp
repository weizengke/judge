
#ifdef _LINUX_

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <termios.h>
#include <assert.h>
#include <errno.h>

#include "kernel.h"

int stricmp(char *str1, char *str2)
{
	return strcasecmp(str1, str2);
}

int strnicmp(char *str1, char *str2, unsigned maxlen)
{
	return strncasecmp(str1, str2, maxlen);
}

int closesocket(socket_t socket)
{
	close(socket);
}

int Sleep(int ms)
{
	usleep(ms*1000);
}

int file_access(const char *pathname, int mode)
{
	return access(pathname, mode);
}

int geterror()
{
	return errno;
}

void* thread_start_func(void *th_ptr)
{
    thread_t *th = (thread_t *)th_ptr;
    thread_id_t thread_id = th->thread_id;

	if (NULL != th->thread_func){
		if (th->thread_func(th->arg)){
		}
	}
	
	free(th_ptr);
	th_ptr = NULL;
	
    return 0;
}

int thread_close(thread_id_t thread_id)
{
	return pthread_cancel(thread_id);
}


thread_id_t thread_create(int (*fn)(void *), void *arg)
{
	int err = 0;
	thread_t *th = NULL;
	pthread_attr_t *const attrp = NULL;
	
	th = (thread_t *)malloc(sizeof(thread_t));
	if (NULL == th){
		return 0;
	}
	memset(th, 0, sizeof(thread_t));
	
	th->thread_func = fn;
	th->arg = arg;

	/* th内存在thread_start_func内释放 */
    err = pthread_create(&th->thread_id, attrp, thread_start_func, th);
    if (!err) {
		return 0;
    }

	return th->thread_id;
}

unsigned long thread_get_self()
{
	return (unsigned long)pthread_self();;
}

bool create_directory(char *path_name)
{
	if (-1 == mkdir(path_name, 0755))
	{
		printf("\r\n mkdir failed. (path_name=%s)", path_name);
		return false;
	}
	
	return true;
}

#endif

