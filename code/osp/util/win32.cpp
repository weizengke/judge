#ifdef _WIN32_

#include <iostream>
#include <windows.h>
#include <process.h>
#include <stdlib.h>
#include <string>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <queue>

#include <conio.h>
#include <io.h>
#include <winsock2.h>

#include "kernel.h"

#ifndef M_DES
#define M_DES(x,y) y
#endif

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
//int pthread_create(pthread_t *tidp, const pthread_attr_t *attr, (void*)(*start_rtn)(void*), void *arg);
#define start_thread (HANDLE)_beginthreadex

typedef unsigned (__stdcall *w32_thread_func)(void*);

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
}

HANDLE w32_create_thread(DWORD stack_size, w32_thread_func func, void *val)
{
    return start_thread(0, stack_size, func, val, CREATE_SUSPENDED, 0);
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

int thread_close(thread_id_t thread_id)
{
	return w32_close_handle(thread_id);
}

unsigned _stdcall thread_start_func(void *th_ptr)
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

	/* th内存在thread_start_func内释放 */
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
extern BOOL CreateDirectory(LPCTSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);

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


#endif /* _WIN32_ */
