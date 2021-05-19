#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>

#ifdef _LINUX_
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <assert.h>
#endif

#ifdef WIN32
#include <io.h>
#include <winsock2.h>
#endif

#include "kernel.h"

#include "osp_common_def.h"
#include "pdt_common_inc.h"
#include "icli.h"
#include "aaa/aaa.h"
#include "root.h"
#include "util/util.h"
#include "judge/include/judge_inc.h"
#include "telnet.h"

#if (OS_YES == OSP_MODULE_TELNETS)

using namespace std;

#define TELNET_AUTHMODE_NONE 0
#define TELNET_AUTHMODE_PASSWORD 1
#define TELNET_AUTHMODE_AAA 2

socket_t g_TelnetSSocket;
ULONG g_TelnetServerEnable = OS_NO;
ULONG g_TelnetAuthMode = 0;
char g_szTelnetUsername[32] = {0};
char g_szTelnetPassword[32] = {0};

int g_telnet_port = 23;
thread_id_t hTelnetLisent = NULL;

#define TELNET_Debug(x, format, ...) debugcenter_print(MID_TELNET, x, format, ##__VA_ARGS__)

void TELNET_KillAllVty()
{
	vty_offile_all();

	return;
}

void TELNET_AgingIdleVty()
{
	int i = 0;
	time_t t = time(NULL);
	long diff = 0;

	for (i == 0; i < CMD_VTY_MAXUSER_NUM; i++)
	{
		if (cmd_vty_is_used(i))
		{
			diff = util_getdiftime(t, vty_get_last_accesstime(i));

			int state = vty_get_state(i);
			/* kill the user is not online */
			if (diff > 60 * 2 && state == 0)
			{
				TELNET_Debug(DEBUG_TYPE_INFO, "TELNET_AgingIdleVty do 1.");
				vty_offline(i);
				continue;
			}

			/* force to kill the user */
			if (diff > 60 * 60 * 24 * 2 && state == 1)
			{
				TELNET_Debug(DEBUG_TYPE_INFO, "TELNET_AgingIdleVty do 2.");
				vty_offline(i);
				continue;
			}
		}
	}

	return;
}

int TELNET_TimerThread(void *pEntry)
{
	ULONG ulLoop = 0;

	for (;;)
	{
		/* 1s timer */
		if (0 == ulLoop % 1)
		{
		}

		/* 30s timer */
		if (0 == ulLoop % 30)
		{
		}

		/* 60s timer */
		if (0 == ulLoop % 60)
		{
			TELNET_AgingIdleVty();
		}

		ulLoop++;

		Sleep(1000);
	}

	return 0;
}

ULONG TELNET_InitSocket()
{
	int ret = 0;

#ifdef WIN32
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);

	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		return OS_ERR;
	}
#endif

	g_TelnetSSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_TelnetSSocket == INVALID_SOCKET)
	{
#ifdef WIN32
		WSACleanup();
#endif
		return OS_ERR;
	}

	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(g_telnet_port);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(g_TelnetSSocket, (const sockaddr *)&sin, sizeof(sin));
	if (SOCKET_ERROR == ret)
	{
		TELNET_Debug(DEBUG_TYPE_ERROR, "TELNET_InitSocket bind failed.");
		closesocket(g_TelnetSSocket);
		return OS_ERR;
	}

	ret = listen(g_TelnetSSocket, 20);
	if (SOCKET_ERROR == ret)
	{
		TELNET_Debug(DEBUG_TYPE_ERROR, "TELNET_InitSocket listen failed. ");
		closesocket(g_TelnetSSocket);

#ifdef WIN32
		WSACleanup();
#endif
		return OS_ERR;
	}

	return OS_OK;
}

void TELNET_RecvBuff(CMD_VTY_S *vty, char *pbuff, int isPsw)
{
	int ret = 0;
	socket_t sClient = 0;
	char buff[1] = {0};
	char buff32[32] = {0};
	int index = 0;

	sClient = vty->user.socket;

	while (OS_YES == g_TelnetServerEnable)
	{
		ret = recv(sClient, (char *)buff, sizeof(buff), 0);
		if (ret <= 0)
		{
			break;
		}

		if (0xd == buff[0])
		{
			break;
		}

		if (0xa == buff[0])
		{
			continue;
		}

		/* backspace */
		if (0x8 == buff[0])
		{
			if (0 == index)
			{
				buff32[index] = '\0';

				if (OS_NO == isPsw)
				{
					send(sClient, (const char *)"\b", sizeof("\b"), 0);
				}

				continue;
			}

			buff32[index--] = '\0';
		}
		else
		{
			if (index >= 32)
			{
				break;
			}

			buff32[index++] = buff[0];

			if (OS_NO == isPsw)
			{
				send(sClient, (const char *)buff, sizeof(buff), 0);
			}

			continue;
		}
	}

	memcpy(pbuff, buff32, sizeof(buff32));

	return;
}

ULONG TELNET_UserAuth(char *uname, char *psw)
{
	if (0 == strlen(g_szTelnetUsername) || 0 == strlen(g_szTelnetPassword))
	{
		return OS_ERR;
	}

	if (0 == strcmp(uname, g_szTelnetUsername) && 0 == strcmp(psw, g_szTelnetPassword))
	{
		return OS_OK;
	}

	return OS_ERR;
}

ULONG TELNET_DoUserAccess(CMD_VTY_S *vty)
{
	int ret = 0;
	socket_t sClient = 0;
	char buff_username[32] = {0};
	char buff_password[32] = {0};

	sClient = vty->user.socket;

	if (TELNET_AUTHMODE_NONE == g_TelnetAuthMode)
	{
		memcpy(vty->user.user_name, buff_username, sizeof(buff_username));
		return OS_OK;
	}

	send(sClient, (const char *)"\r\nUsername:", sizeof("\r\nUsername:"), 0);
	TELNET_RecvBuff(vty, buff_username, OS_NO);
	send(sClient, (const char *)"\r\nPassword:", sizeof("\r\nPassword:"), 0);
	TELNET_RecvBuff(vty, buff_password, OS_YES);

	//write_log(JUDGE_INFO,"Telnet user access. (name=%s, psw=%s)", buff_username, buff_password);

	if (TELNET_AUTHMODE_PASSWORD == g_TelnetAuthMode)
	{
		if (OS_OK == TELNET_UserAuth(buff_username, buff_password))
		{
			memcpy(vty->user.user_name, buff_username, sizeof(buff_username));
			return OS_OK;
		}

		return OS_ERR;
	}

	if (TELNET_AUTHMODE_AAA == g_TelnetAuthMode)
	{
		if (OS_OK == AAA_UserAuth(buff_username, buff_password, AAA_SERVICE_TYPE_TELNET))
		{
			memcpy(vty->user.user_name, buff_username, sizeof(buff_username));
			return OS_OK;
		}

		return OS_ERR;
	}

	return OS_ERR;
}

void TELNET_Send_Will(CMD_VTY_S *vty, char option)
{
	char buff[3] = {0};
	char buf_option[3] = {0};
	socket_t sClient = 0;

	sClient = vty->user.socket;

	buf_option[0] = TEL_IAC;
	buf_option[1] = TEL_WILL;
	buf_option[2] = option;
	(void)send(sClient, (const char *)buf_option, 3, 0);
	TELNET_Debug(DEBUG_TYPE_INFO, "TELNET_Send_Will. Send(%x, %x, %x).", buf_option[0], buf_option[1], buf_option[2]);

	(void)recv(sClient, (char *)buff, 3, 0);
	TELNET_Debug(DEBUG_TYPE_INFO, "TELNET_Send_Will. Recv(%x, %x, %x).", buff[0], buff[1], buff[2]);
}

void TELNET_Send_Do(CMD_VTY_S *vty, char option)
{
	UCHAR buff[3] = {0};
	UCHAR buf_option[3] = {0};
	socket_t sClient = 0;

	sClient = vty->user.socket;

	buf_option[0] = TEL_IAC;
	buf_option[1] = TEL_DO;
	buf_option[2] = option;
	(void)send(sClient, (const char *)buf_option, 3, 0);
	TELNET_Debug(DEBUG_TYPE_INFO, "TELNET_Send_Do. Send(%x, %x, %x).", buf_option[0], buf_option[1], buf_option[2]);
	(void)recv(sClient, (char *)buff, 3, 0);
	TELNET_Debug(DEBUG_TYPE_INFO, "TELNET_Send_Do. Recv(%x, %x, %x).", buff[0], buff[1], buff[2]);
}

int TELNET_SessionThread(void *pEntry)
{
	socket_t sClient = 0;
	UCHAR buf_option[4] = {0};
	UCHAR buff_hello[1024] = "\r\n=========================>>\r\nWelcome to judger-kernel.\r\n=========================>>\r\n";
	CMD_VTY_S *vty = (CMD_VTY_S *)pEntry;
	extern VOID cmd_outprompt(CMD_VTY_S * vty);

	if (NULL == vty)
	{
		return 0;
	}

	sClient = vty->user.socket;

	/* will echo */
	TELNET_Send_Will(vty, TEL_ECHO);

	vty->user.lastAccessTime = time(NULL);

	/* user do access check */
	if (OS_OK != TELNET_DoUserAccess(vty))
	{
		vty_offline(vty->vtyId);
		return 0;
	}

	TELNET_Debug(DEBUG_TYPE_INFO, "Telnet user '%s' access successfuly.", vty->user.user_name);
	write_log(JUDGE_INFO, "Telnet user '%s' access successfuly.", vty->user.user_name);

	/* vty access ok */
	vty->user.state = 1;
	vty->user.lastAccessTime = time(NULL);
	send(sClient, (const char *)buff_hello, sizeof(buff_hello), 0);

	cmd_outprompt(vty);

	/* vty loop read */
	vty_go(vty->vtyId);

	/* vty offline */
	vty_offline(vty->vtyId);

	return 0;
}

int TELNET_ListenThread(void *pEntry)
{
	char buff[1024] = {0};
	sockaddr_in remoteAddr;
	socket_t sClient;
	socklen_t nAddrLen = sizeof(remoteAddr);
	CMD_VTY_S *vty = NULL;

	while (OS_YES == g_TelnetServerEnable)
	{
		sClient = accept(g_TelnetSSocket, (SOCKADDR *)&remoteAddr, &nAddrLen);
		if (sClient == INVALID_SOCKET)
		{
			continue;
		}

		TELNET_Debug(DEBUG_TYPE_INFO, "\r\nAccept new connect. (ip:%s, port:%u, socket=%u)\r\n", inet_ntoa(remoteAddr.sin_addr), htons(remoteAddr.sin_port), sClient);

		vty = cmd_get_idle_vty();
		if (NULL == vty)
		{
			sprintf(buff, "\r\nError: Rejected the connection because the judger exeed the max vty number(%u).", CMD_VTY_MAXUSER_NUM);
			send(sClient, (const char *)buff, sizeof(buff), 0);

			closesocket(sClient);
			continue;
		}

		vty->user.socket = sClient;
		thread_create(TELNET_SessionThread, vty);

		Sleep(1);
	}

	return 0;
}

ULONG TELNET_ServerEnable()
{
	ULONG ulRet = OS_OK;

	if (OS_YES == g_TelnetServerEnable)
	{
		return OS_OK;
	}

	ulRet = TELNET_InitSocket();
	if (OS_OK != ulRet)
	{
		return OS_ERR;
	}

	g_TelnetServerEnable = OS_YES;

	hTelnetLisent = thread_create(TELNET_ListenThread, NULL);

	TELNET_Debug(DEBUG_TYPE_INFO, "TELNET_ServerEnable ok.");

	return OS_OK;
}

ULONG TELNET_ServerDisable()
{
	ULONG ulRet = OS_OK;

	if (OS_NO == g_TelnetServerEnable)
	{
		return OS_ERR;
	}

	thread_close(hTelnetLisent);
	hTelnetLisent = NULL;

	g_TelnetServerEnable = OS_NO;

	closesocket(g_TelnetSSocket);
	g_TelnetSSocket = INVALID_SOCKET;

	/* kill all user */
	vty_offile_all();

	TELNET_Debug(DEBUG_TYPE_INFO, "TELNET_ServerDisable ok.");

	return OS_OK;
}

ULONG TELNET_BuildRun(CHAR **ppBuildrun, ULONG ulIncludeDefault)
{
	CHAR *pBuildrun = NULL;

	*ppBuildrun = (CHAR *)malloc(BDN_MAX_BUILDRUN_SIZE);
	if (NULL == *ppBuildrun)
	{
		return OS_ERR;
	}
	memset(*ppBuildrun, 0, BDN_MAX_BUILDRUN_SIZE);

	pBuildrun = *ppBuildrun;

	extern int g_telnet_port;
	if (23 != g_telnet_port || OS_YES == ulIncludeDefault) {
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"telnet server port %u", g_telnet_port);
	}

	if (OS_YES == g_TelnetServerEnable)
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN "telnet server enable");
	}
	else
	{
		if (VOS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN "undo telnet server enable");
		}
	}

	if (TELNET_AUTHMODE_NONE == g_TelnetAuthMode)
	{
		if (VOS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN "telnet authentication-mode none");
		}
	}
	else if (TELNET_AUTHMODE_PASSWORD == g_TelnetAuthMode)
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN "telnet authentication-mode password");

		if (strlen(g_szTelnetUsername) > 0 && strlen(g_szTelnetPassword) > 0)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN "telnet username %s password %s", g_szTelnetUsername, g_szTelnetPassword);
		}
	}
	else if (TELNET_AUTHMODE_AAA == g_TelnetAuthMode)
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN "telnet authentication-mode aaa");
	}

	return OS_OK;
}

ULONG TELNET_EVT_UserKill(ULONG keyId, ULONG cmdId, VOID *pData, VOID **ppInfo)
{
	if (TELNET_AUTHMODE_AAA != g_TelnetAuthMode)
	{
		return OS_OK;
	}

	/* AAA user del, then kill vty offline */
	vty_offline_by_username((CHAR *)pData);

	return OS_OK;
}

int TELNET_Init()
{
	/* reg aaa event */
	(VOID) AAA_EvtRegistFunc("TELNET", AAA_EVT_USER_DEL, 0, TELNET_EVT_UserKill);
	(VOID) AAA_EvtRegistFunc("TELNET", AAA_EVT_USER_PSW_CHANGE, 0, TELNET_EVT_UserKill);
	(VOID) AAA_EvtRegistFunc("TELNET", AAA_EVT_USER_SERVICE_TELNET_DEL, 0, TELNET_EVT_UserKill);

	return OS_OK;
}

int TELNET_TaskEntry(void *pEntry)
{
	ULONG ulRet = OS_OK;

	{
		extern ULONG TELNET_RegCmd();
		(VOID) TELNET_RegCmd();
	}

	(void)cli_bdn_regist(MID_TELNET, VIEW_SYSTEM, BDN_PRIORITY_HIGH + 100, TELNET_BuildRun);

	(VOID)thread_create(TELNET_TimerThread, NULL);

	for (;;)
	{
		Sleep(1000);
	}
}

APP_INFO_S g_telnetAppInfo =
	{
		MID_TELNET,
		"Telnet",
		TELNET_Init,
		TELNET_TaskEntry
	};

void TELNET_RegAppInfo()
{
	APP_RegistInfo(&g_telnetAppInfo);
}

#endif
