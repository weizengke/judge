#ifndef FTP_COMMON_H
#define FTP_COMMON_H

#ifdef _LINUX_
#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#else
#include <windows.h>
#include <process.h>
#include <iostream>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <time.h>
#include <queue>
#include <string>
#include <sstream>
#endif

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
	
	FTP_ERR_CODE_MAX
};

#define FTP_DEBUG_BUFSIZE (1024*6)
#define FTP_CMD_CODE_SIZE 5
#define FTP_CMD_ARGV_SIZE 512
#define FTP_CMD_MAXSIZE 1024
#define FTP_USERNAME_SIZE 32
#define FTP_PASSWORD_SIZE 32
#define FTP_FILEPATH_SIZE 256
#define FTP_DATABUF_SIZE 1024

extern VOID FTP_debug(const CHAR *format, ...);
extern ULONG FTP_RecvCommand(LONG lSock, CHAR* szCmd, CHAR* arg);
extern ULONG FTP_Send(LONG lSock, CHAR *szMessage, ULONG ulSize);
extern ULONG FTP_Recv(LONG lSock, CHAR *szMessage, ULONG ulSize);
extern ULONG FTP_Response(LONG lSock, ULONG ulCode);

/*  
1: create soket by port
2: return socket
*/
extern LONG FTP_SOCK_Create(ULONG ulPort);
	
/*  
1: connect by ip and port
2: return socket
*/
extern LONG FTP_SOCK_Connect(ULONG ulPort, CHAR* szIP);

/*
1: Accept client connect
2: return socket accepted
*/
extern LONG FTP_SOCK_Accept(LONG lSockListen);

#define FTP_debug(args...) debugcenter_print(MID_FTP, DEBUG_TYPE_INFO, args)

#endif







