#ifndef _KERNEL_H_
#define _KERNEL_H_

#ifdef _WIN32_
typedef SOCKET socket_t;
#else
typedef int socket_t;
#endif

#ifdef _WIN32_
typedef HANDLE thread_id_t;
#else
typedef pthread_t thread_id_t;
#endif

#ifdef _WIN32_
typedef HANDLE mutex_t;
#else
typedef pthread_mutex_t mutex_t;
#endif

#ifdef _WIN32_
typedef int socklen_t;
#else
typedef struct sockaddr SOCKADDR;
#endif

typedef struct thread_st {
	thread_id_t thread_id;
	void *arg;
	int (*thread_func)(void *);
	
}thread_t;

#ifdef _LINUX_
extern int stricmp(char *str1, char *str2);
extern int strnicmp(char *str1, char *str2, unsigned maxlen);
extern int closesocket(socket_t socket);
extern int Sleep(int ms);
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

