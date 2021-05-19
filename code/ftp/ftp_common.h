#ifndef FTP_COMMON_H
#define FTP_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>

#include <iostream>
#include <string>
#include <sstream>

#ifdef _LINUX_

#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#else
#include <io.h>
#include <time.h>
#include <windows.h>
#endif

#include "kernel.h"

#ifndef ULONG
#undef ULONG
#define ULONG unsigned long
#endif

#ifndef LONG
#undef LONG
#define LONG long
#endif

#ifdef UCHAR
#undef UCHAR
#define UCHAR unsigned char
#endif

#ifndef CHAR
#undef CHAR
#define CHAR char
#endif

#ifndef VOID
#undef VOID
#define VOID void
#endif

/* constants */
#define DEBUG				1
#define MAXSIZE 			512

#ifndef INVALID_SOCKET
#define INVALID_SOCKET	(ULONG)(~0)
#endif

/* error code */
enum FTP_ERR_CODE_EM {
	FTP_OK = 0,
	FTP_ERR,
	FTP_ERR_CONNECT_FAILED,
	FTP_ERR_SOCKET,
	FTP_CWD_INVALID_PATH,
	FTP_CWD_BAD_DOT,
	FTP_CWD_NO_PERMISSION,

	FTP_ERR_CODE_MAX
};

#define FTP_DEBUG_BUFSIZE (1024*6)
#define FTP_CMD_CODE_SIZE 512
#define FTP_CMD_ARGV_SIZE 512
#define FTP_CMD_MAXSIZE 1024
#define FTP_USERNAME_SIZE 32
#define FTP_PASSWORD_SIZE 32
#define FTP_FILEPATH_SIZE 256
#define FTP_DATABUF_SIZE 1024

#define FTP_CLOSESOCKET(s)			     \
{										 \
	if (s != INVALID_SOCKET) {           \
		closesocket(s);     			 \
		s = INVALID_SOCKET;				 \
	} 									 \
}

extern VOID FTP_debug(const CHAR *format, ...);
extern ULONG FTP_RecvCommand(socket_t sock, CHAR* szCmd, CHAR* arg);
extern ULONG FTP_Send(socket_t sock, CHAR *szMessage, ULONG ulSize);
extern ULONG FTP_Recv(socket_t sock, CHAR *szMessage, ULONG ulSize);
extern ULONG FTP_Response(socket_t sock, ULONG ulCode);

extern void FTP_GetLocalIpBySocket(int sock, int *ip);
extern void FTP_GetPeerIpBySocket(int sock, int *ip);
/*  
1: create soket by port
2: return socket
*/
extern socket_t FTP_SOCK_Create(ULONG Ip, ULONG ulPort);
	
/*  
1: connect by ip and port
2: return socket
*/
extern socket_t FTP_SOCK_Connect(ULONG ulPort, CHAR* szIP);

/*
1: Accept client connect
2: return socket accepted
*/
extern socket_t FTP_SOCK_Accept(socket_t sockListen);

#define FTP_debug(format, ...) debugcenter_print(MID_FTP, DEBUG_TYPE_INFO, format, ##__VA_ARGS__)

#endif







