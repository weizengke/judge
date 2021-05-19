#ifndef _KERNEL_H_
#define _KERNEL_H_

#ifdef WIN32
#define socket_t SOCKET
#else
#define socket_t int
#endif

#ifdef WIN32
#define thread_id_t HANDLE
#else
#define thread_id_t pthread_t
#endif

#ifdef WIN32
#define mutex_t HANDLE
#else
#define mutex_t pthread_mutex_t
#endif

#ifdef WIN32
typedef int socklen_t;
#else
#define SOCKADDR  struct sockaddr
#endif

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE  -1
#endif

#define THREAD_NAME_MAX_LEN 32

typedef struct thread_st {
	thread_id_t thread_id;
	char thread_name[THREAD_NAME_MAX_LEN];
	void *arg;
	int (*thread_func)(void *);
	
}thread_t;

#ifdef _LINUX_
extern int stricmp(char *str1, char *str2);
extern int strnicmp(char *str1, char *str2, unsigned maxlen);
extern int closesocket(socket_t socket);
extern int Sleep(int ms);
extern int CopyFile(const char* src, const char* des, int flag);
#endif

extern int geterror();
extern int thread_close(thread_id_t thread_id);
extern thread_id_t thread_create(int (*fn)(void *), void *arg);
extern unsigned long thread_get_self();

extern bool create_directory(char *path_name);
extern int file_access(const char *pathname, int mode);
extern bool get_current_directory(int buf_len, char* current_path);

extern mutex_t mutex_create(char *name);
extern int mutex_lock(mutex_t mutex);
extern int mutex_unlock(mutex_t mutex);

#endif

