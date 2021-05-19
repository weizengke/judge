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

#ifdef WIN32
#include <conio.h>
#include <io.h>
#include <winsock2.h>
#endif

#include "kernel.h"

#include "osp_common_def.h"
#include "pdt_common_inc.h"
#include "event/include/event_pub.h"
#include "icli.h"
#include "aaa/aaa.h"
#include "judge/include/judge_inc.h"
#include "telnet.h"

using namespace std;

#if (OS_YES == OSP_MODULE_TELNETC)

#define TELNETC_DATABUF_SIZE 4096
#define TELNET_Debug(x, format, ...) debugcenter_print(MID_TELNET, x, format, ##__VA_ARGS__)

typedef struct telnetc_thread_st {
	ULONG vtyId;
	LONG  sock;
}TELNETC_THREAD_S;

LONG TELNETC_Connect(ULONG ulPort, CHAR* szIP)
{
	LONG sock;					
	struct sockaddr_in addr;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
		TELNET_Debug(DEBUG_TYPE_ERROR, "TELNETC_Connect, socket() error.");
		return INVALID_SOCKET;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(ulPort);
	addr.sin_addr.s_addr = inet_addr(szIP);

	if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0 ) {
		TELNET_Debug(DEBUG_TYPE_ERROR, "TELNETC_Connect, connect() error.");
		return INVALID_SOCKET;
	}
	
	TELNET_Debug(DEBUG_TYPE_INFO, "TELNETC_Connect. (sock=%d, szIP=%s:%d)", sock, szIP, ulPort);
	return sock;
}


int TELNETC_Recv(LONG sock, CHAR *szMessage, ULONG ulSize)
{
	int len = 0;
	
	memset(szMessage, 0, ulSize);
	len = recv(sock, szMessage, ulSize, 0);
	if (len <= 0) {		
		return 0;
	}
	
	return len;
}

ULONG TELNETC_Send(LONG sock, CHAR *szMessage, ULONG ulSize)
{
	if (send(sock, (char*)szMessage, ulSize, 0) < 0 ) {
		TELNET_Debug(DEBUG_TYPE_ERROR, "TELNETC_Send failed. (sock=%d)", sock);
		return OS_ERR;
	}
	
	return OS_OK;
}

void TELNETC_Send_Do(LONG sock, char option)
{
	char buff[3] = {0};
	char buf_option[3] = {0};

	buf_option[0] = TEL_IAC;
	buf_option[1] = TEL_DO;
	buf_option[2] = option;
	(void)send(sock, (const char *)buf_option, 3, 0);
	TELNET_Debug(DEBUG_TYPE_INFO, "TELNETC_Send_Do. Send(%x, %x, %x).", buf_option[0], buf_option[1], buf_option[2]);

	(void)recv(sock, (char *)buff, 3, 0);
	TELNET_Debug(DEBUG_TYPE_INFO, "TELNETC_Send_Do. Recv(%x, %x, %x).", buff[0], buff[1], buff[2]);
}
int TELNETC_RecvThread(void *pEntry)
{
	TELNETC_THREAD_S *para = (TELNETC_THREAD_S*)pEntry;
	ULONG vtyId = 0;
	LONG sock = 0;
	int len;	
	UCHAR buff[TELNETC_DATABUF_SIZE] = {0};
	
	vtyId = para->vtyId;
	sock = para->sock;

	while (1) {
		len = TELNETC_Recv(sock, (CHAR*)buff, TELNETC_DATABUF_SIZE);
		if (len <= 0) {			
			break;
		}

		TELNET_Debug(DEBUG_TYPE_INFO, "TELNETC: 0x%x,0x%x,0x%x; len=%d", buff[0], buff[1], buff[2], len);

		if (buff[0] == TEL_IAC && len >= 3) {
			TELNET_Debug(DEBUG_TYPE_INFO, "TELNETC: IAC 0x%x 0x%x", buff[1], buff[2]);

			/* TODO: process option */
			if (buff[1] == TEL_WILL) {
				TELNETC_Send_Do(sock, buff[2]);
			}
		} else if (buff[0] == TEL_WILL && len == 2) {
			TELNET_Debug(DEBUG_TYPE_INFO, "TELNETC: IAC 0x%x 0x%x", buff[0], buff[1]);

			/* TODO: process option */
			if (buff[0] == TEL_WILL) {
				TELNETC_Send_Do(sock, buff[1]);
			}
		} else {
			/* output to terminal */
			vty_printf(vtyId, "%s", buff);
		}
		
		memset(buff, 0, sizeof(buff));
	}
	
	return OS_OK;
}

UCHAR TELNETC_getch()
{
	UCHAR c = 0;
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

VOID TELNETC_Read(CMD_VTY_S* vty)
{
	UCHAR c = 0;
	UCHAR buff[1] = {0};
	int len;
	extern UCHAR vty_getchar(CMD_VTY_S *vty);

	/* single char mode */
	while((c = vty_getchar(vty)) > 0) {
		buff[0] = c;
		if (OS_OK != TELNETC_Send(vty->user.socket, (CHAR*)buff, 1)) {
			TELNET_Debug(DEBUG_TYPE_ERROR, "TELNETC_Read, telnetc send failed.");
			break;
		}
	}

	return ;
}

ULONG TELNETC_Run(ULONG vtyId, ULONG ulPort, CHAR *szIP)
{
	TELNETC_THREAD_S para = {0};
	LONG sock = INVALID_SOCKET;

#if 0	
	if (CMD_VTY_CONSOLE_ID != vtyId) {
		vty_printf(vtyId, "Error: Only console user support telnet client.\r\n");
		return OS_ERR;
	}
#endif

	/* connect socket */
	sock = TELNETC_Connect(ulPort, szIP);
	if (INVALID_SOCKET == sock) {
		return OS_ERR;
	}

	/* create recv thread */
	para.vtyId = vtyId;
	para.sock = sock;
	thread_create(TELNETC_RecvThread, (VOID*)&para);
	
	/* loop input */
	CMD_VTY_S vty = {0};
	vty.vtyId = vtyId;
	vty.user.socket = sock;
	TELNETC_Read(&vty);

	return OS_OK;
}

#endif
