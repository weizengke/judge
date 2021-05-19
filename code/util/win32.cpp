#ifdef WIN32

#include <iostream>
#include <windows.h>
#include <process.h>
#include <stdlib.h>
#include <string>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <queue>
#include <io.h>

#include "util/utlist.h"

#include "kernel.h"

#ifndef M_DES
#define M_DES(x,y) y
#endif

extern void write_log(int level, const char *fmt, ...);

mutex_t mutex_create(char *name)
{
	return CreateSemaphore(NULL, 1, 1, name);
}

int mutex_lock(mutex_t mutex)
{
	if (WaitForSingleObject(mutex, INFINITE) != 0)
	{
		return 1;
	}

	return 0;
}

int mutex_unlock(mutex_t mutex)
{
	if (ReleaseSemaphore(mutex, 1, NULL) != 0)
	{
		return 1;
	}

	return 0;
}


#if M_DES("thread", 1)
#define start_thread (HANDLE)_beginthreadex
typedef unsigned int (__stdcall *w32_thread_func)(void*);

int geterror()
{
	return (int)GetLastError();
}

void w32_error(const char *func)
{
    LPVOID lpMsgBuf;
    DWORD err = GetLastError();
    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		      FORMAT_MESSAGE_FROM_SYSTEM |
		      FORMAT_MESSAGE_IGNORE_INSERTS,
		      NULL,
		      err,
		      MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
		      (LPTSTR) & lpMsgBuf, 0, NULL) == 0)
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		      FORMAT_MESSAGE_FROM_SYSTEM |
		      FORMAT_MESSAGE_IGNORE_INSERTS,
		      NULL,
		      err,
		      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		      (LPTSTR) & lpMsgBuf, 0, NULL);
			  
    printf("\r\n%s: %s", func, (char*)lpMsgBuf);
	write_log(2, "%s: %s", func, (char*)lpMsgBuf);
}

HANDLE w32_create_thread(DWORD stack_size, w32_thread_func func, void *val)
{
	HANDLE hd = start_thread(0, stack_size, func, val, CREATE_SUSPENDED, 0);

    return hd;
}

int w32_close_handle(HANDLE handle)
{
    if (CloseHandle(handle) == 0) {
		w32_error("w32_close_handle");
		return -1;
	}

	return 0;
}

void w32_resume_thread(HANDLE handle)
{
    if (ResumeThread(handle) == (DWORD)-1) {
		w32_error("w32_resume_thread");
	}
}

typedef struct tag_thread_info_s 
{
	thread_id_t thread_id;
	char thread_name[THREAD_NAME_MAX_LEN];
	struct tag_thread_info_s *prev;
	struct tag_thread_info_s *next;
} thread_info_s;

thread_info_s *g_thread_list = NULL;

int thread_iner_comp(void *a, void *b) {
	thread_info_s *key1 = (thread_info_s*)a;
	thread_info_s *key2 = (thread_info_s*)b;

    return (int)(key1->thread_id) - (int)(key2->thread_id);
}

int thread_iner_add_to_list(thread_t *thread)
{
	if (NULL == thread) {
		return 0;
	}

	thread_info_s *thd = NULL;
	thread_info_s key = {0};
	key.thread_id = thread->thread_id;
	DL_SEARCH(g_thread_list, thd, &key, thread_iner_comp);
	if (thd != NULL) {
		printf("A thread update. (thread_id:%d, thread_name:%s -> %s)",
				thd->thread_id, thd->thread_name, thread->thread_name);		
		strcpy(thd->thread_name, thread->thread_name);
		return 1;
	}

	thread_info_s *thd_new = (thread_info_s*)malloc(sizeof(thread_info_s));
	memset(thd_new, 0, sizeof(thread_info_s));
	thd_new->thread_id = thread->thread_id;
	strcpy(thd_new->thread_name, thread->thread_name);
	
	DL_APPEND(g_thread_list, thd_new);

	thread_info_s *neibor;
	int count = 0;
	DL_COUNT(g_thread_list, neibor, count);

	printf("A thread add. (thread_id:%d, thread_name:%s, count=%d)",
				thd_new->thread_id, thd_new->thread_name, count);

	return 1;
}

VOID thread_iner_del_from_list(thread_id_t thread_id)
{
	thread_info_s *thd, *tmp;
	DL_FOREACH_SAFE(g_thread_list, thd, tmp) {
		if (thd->thread_id == thread_id) {
			printf("A thread del. (thread_id:%d, thread_name:%s)",
					thd->thread_id, thd->thread_name);
			DL_DELETE(g_thread_list, thd);
			free(thd);
			return ;
		}
	}

	return ;
}

int thread_close(thread_id_t thread_id)
{
	return w32_close_handle(thread_id);
}

unsigned int __stdcall thread_start_func(void *th_ptr)
{
    thread_t *th = (thread_t *)th_ptr;
    thread_id_t thread_id = th->thread_id;

	if (NULL != th->thread_func){
		if (th->thread_func(th->arg)){
			w32_error("thread_start_func");
		}
	}
	
    (void)w32_close_handle(thread_id);

	free(th_ptr);
	th_ptr = NULL;
	
    return 0;
}

thread_id_t thread_create(int (*fn)(void *), void *arg)
{
	thread_t *th = NULL;

	th = (thread_t *)malloc(sizeof(thread_t));
	if (NULL == th){
		w32_error("thread_create, no memory.");
		return 0;
	}
	memset(th, 0, sizeof(thread_t));
	
	th->thread_func = fn;
	th->arg = arg;

	/* th�ڴ���thread_start_func���ͷ� */
    th->thread_id = w32_create_thread(0, thread_start_func, th);	
    if ((th->thread_id) == 0) {
		w32_error("thread_create");
		return 0;
    }

    w32_resume_thread(th->thread_id);

	return th->thread_id;
}

unsigned long thread_get_self()
{
	return (unsigned long)GetCurrentThreadId();
}


#endif

#if M_DES("thread", 1)
//extern BOOL CreateDirectory(LPCTSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);

bool win32_create_directory(char *path)
{
	if (NULL == path){
		return false;
	}
	
	if (TRUE != CreateDirectory(path, NULL)){
		w32_error("win32_create_directory");
		return false;
	}

	return true;
}

#endif


int file_access(const char *pathname, int mode)
{
	return _access(pathname, mode);
}

bool create_directory(char *path_name)
{
	return win32_create_directory(path_name);
}

bool get_current_directory(int buf_len, char* current_path)
{
	if (0 != GetCurrentDirectory(buf_len, current_path))
	{	
		return true;
	}

	return false;
}


#endif /* WIN32 */
