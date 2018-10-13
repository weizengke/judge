#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>

#ifdef _LINUX_
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <termios.h>
#include <assert.h>
#endif

#ifdef _WIN32_
#include <conio.h>
#include <io.h>
#include <winsock2.h>

#endif

#include "kernel.h"

#include "osp/common/include/osp_common_def.h"
#include "product/include/pdt_common_inc.h"
#include "osp/event/include/event_pub.h"
#include "osp/command/include/icli.h"
#include "osp/aaa/aaa.h"
#include "product/judge/include/judge_inc.h"

#if (OS_YES == OSP_MODULE_TELNETC)

using namespace std;

#define TELNETC_DATABUF_SIZE 1024

#define TELNET_Debug(x, args...) debugcenter_print(MID_TELNET, x, args)


LONG TELNETC_Connect(ULONG ulPort, CHAR* szIP)
{
	LONG lSockfd;					
	struct sockaddr_in addr;

	if ((lSockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{ 
		TELNET_Debug(DEBUG_TYPE_ERROR, "TELNETC_Connect, socket() error.");
		return INVALID_SOCKET;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(ulPort);
	addr.sin_addr.s_addr = inet_addr(szIP);

	if(connect(lSockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0 )
	{
		TELNET_Debug(DEBUG_TYPE_ERROR, "TELNETC_Connect, connect() error.");
		return INVALID_SOCKET;
	}	 
	
	return lSockfd;
}


ULONG TELNETC_Recv(LONG lSock, CHAR *szMessage, ULONG ulSize)
{
	int iRecvByte = 0;
	
	memset(szMessage, 0, ulSize);
	
	iRecvByte = recv(lSock, szMessage, ulSize, 0);

	if (iRecvByte <= 0)
	{		
		return 0;
	}
	
	return iRecvByte;
}

ULONG TELNETC_Send(LONG lSock, CHAR *szMessage, ULONG ulSize)
{

	if (send(lSock, (char*)szMessage, ulSize, 0) < 0 )
	{
		return OS_ERR;
	}
	
	return OS_OK;
}

int TELNETC_RecvThread(void *pEntry)
{
	LONG lSock = *(LONG*)pEntry;
	size_t num_read;	
	CHAR szData[TELNETC_DATABUF_SIZE] = {0};
	
	while(TRUE)
	{
		num_read = TELNETC_Recv(lSock, szData, TELNETC_DATABUF_SIZE);
		if (0 == num_read)
		{			
			break;
		}

		vty_printf(CMD_VTY_CONSOLE_ID, "~%c", szData[0]);

		Sleep(1);
	}
	
	return OS_OK;
}

ULONG TELNETC_getch()
{
	ULONG c = 0;
	#ifdef _LINUX_
	struct termios org_opts, new_opts;
	ULONG res = 0;
	//-----  store old settings -----------
	res = tcgetattr(STDIN_FILENO, &org_opts);
	assert(res == 0);
	//---- set new terminal parms --------
	memcpy(&new_opts, &org_opts, sizeof(new_opts));
	new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
	tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
	c = getchar();
	//------  restore old settings ---------
	res = tcsetattr(STDIN_FILENO, TCSANOW, &org_opts);
	assert(res == 0);
	#else
	c = getch();
	#endif

	return c;
}

ULONG TELNETC_Read(LONG lSock)
{
	CHAR c = 0;
	CHAR szBuf[10] = {0};
	
	while(c = TELNETC_getch() > 0)
	{	
		
		if (c == 'q')
		{
			break;
		}	

		printf("%c", c);
		szBuf[0] = c;
		
		if (OS_OK != TELNETC_Send(lSock, (CHAR*)szBuf, 1))
		{
			printf("\r\n error send.");
		}
	}
}

ULONG TELNETC_Run(ULONG ulVtyID, ULONG ulPort, CHAR *szIP)
{
	LONG lSock = INVALID_SOCKET;
	
	/* connect socket */
	lSock = TELNETC_Connect(ulPort, szIP);
	if (INVALID_SOCKET == lSock)
	{
		return OS_ERR;
	}

	/* create recv thread */
	// thread_create(TELNETC_RecvThread, (VOID*)&lSock);
	
	TELNETC_Read(lSock);

	return OS_OK;
}

#endif
