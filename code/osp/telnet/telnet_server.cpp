#include <iostream>
#include <windows.h>
#include <process.h>
#include <stdlib.h>
//#include <thread>
#include <string>
//#include <mutex>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <queue>

#include "osp\common\include\osp_common_def.h"
#include "osp\command\include\command_inc.h"
#include "product\include\pdt_common_inc.h"
#include "product\judge\include\judge_inc.h"


using namespace std;

#define TELNET_AUTHMODE_NONE     0
#define TELNET_AUTHMODE_PASSWORD 1
#define TELNET_AUTHMODE_AAA      2

typedef struct tagAAA_USER_S {
	int used;
	int level;
	int type;
	char user_name[32];
	char user_psw[32];
}AAA_USER_S;

SOCKET g_TelnetSSocket;
ULONG g_TelnetServerEnable = OS_NO;
ULONG g_TelnetAuthMode = 0;
char g_szTelnetUsername[32] = {0};
char g_szTelnetPassword[32] = {0};

int g_telnet_port = 23;
HANDLE hTelnetLisent = NULL;

#define AAA_USER_MAX_NUM 16 
AAA_USER_S g_stAAAUser[AAA_USER_MAX_NUM] = {0};

#define TELNET_Debug(x, args...) debugcenter_print(MID_TELNET, x, args)

/* 
SE    240(F0)     子选项结束
SB    250(FA)     子选项开始
IAC   255(FF)     选项协商的第一个字节
WILL  251(FB)     发送方激活选项(接收方同意激活选项)
DO    253(FD)     接收方同意（发送方想让接收方激活选项）
WONT  252(FC)     接收方不同意
DONT  254(FE)     接受方回应WONT
*/
#define TEL_SE   0xF0
#define TEL_SB   0xF0
#define TEL_IAC  0xFF
#define TEL_WILL 0xFB
#define TEL_DO   0xFD
#define TEL_WONT 0xFC
#define TEL_DONT 0xFE

/*
1(0x01)    回显(echo)
3(0x03)    抑制继续进行(传送一次一个字符方式可以选择这个选项)
24(0x18)   终端类型
31(0x1F)   窗口大小
32(0x20)   终端速率
33(0x21)   远程流量控制
34(0x22)   行方式
36(0x24)   环境变量
*/
#define TEL_ECHO  0x01

AAA_USER_S * AAA_LookupUser(char *uname)
{
	for (int i = 0; i < AAA_USER_MAX_NUM; i++)
	{
		if (1 == g_stAAAUser[i].used
			&& 0 == strcmp(uname, g_stAAAUser[i].user_name))
		{
			return &g_stAAAUser[i];
		}
	}

	return NULL;
}

AAA_USER_S * AAA_GetIdleUser(char *uname, char *psw)
{
	for (int i = 0; i < AAA_USER_MAX_NUM; i++)
	{
		if (0 == g_stAAAUser[i].used)
		{
			memset(&g_stAAAUser[i], 0, sizeof(AAA_USER_S));
			return &g_stAAAUser[i];
		}
	}

	return NULL;
}


ULONG AAA_AddUser(struct cmd_vty *vty, char *uname, char *psw)
{
	AAA_USER_S * user = NULL;

	user = AAA_LookupUser(uname);
	if (NULL != user)
	{	
		vty_printf(vty, "Error: The user %s is already exist.\r\n", uname);
		return OS_ERR;
	}

	user = AAA_GetIdleUser(uname, psw);
	if (NULL == user)
	{
		vty_printf(vty, "Error: Create user failed, because exceed the max number(%d).\r\n", AAA_USER_MAX_NUM);
		return OS_ERR;
	}

	user->used = OS_YES;
	strcpy(user->user_name, uname);
	strcpy(user->user_psw, psw);

	return OS_OK;
}

ULONG AAA_DelUser(struct cmd_vty *vty, char *uname)
{
	AAA_USER_S * user = NULL;

	user = AAA_LookupUser(uname);
	if (NULL == user)
	{	
		vty_printf(vty, "Error: The user %s is not exist.\r\n", uname);
		return OS_ERR;
	}

	memset(user, 0, sizeof(AAA_USER_S));

	return OS_OK;
}

ULONG AAA_UserAuth(char *uname, char *psw)
{
	AAA_USER_S * user = NULL;

	user = AAA_LookupUser(uname);
	if (NULL == user)
	{	
		return OS_ERR;
	}

	if (0 == strcmp(user->user_psw, psw))
	{
		return OS_OK;
	}

	return OS_ERR;
}

ULONG AAA_BuildRun(CHAR **ppBuildrun)
{
	CHAR *pBuildrun = NULL;

	*ppBuildrun = (CHAR*)malloc(BDN_MAX_BUILDRUN_SIZE);
	if (NULL == *ppBuildrun)
	{
		return OS_ERR;
	}
	memset(*ppBuildrun, 0, BDN_MAX_BUILDRUN_SIZE);
	
	pBuildrun = *ppBuildrun;

	pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"aaa");

	for (int i = 0; i < AAA_USER_MAX_NUM; i++)
	{
		if (g_stAAAUser[i].used)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN_INDENT_1"local-user %s password %s",
				g_stAAAUser[i].user_name, g_stAAAUser[i].user_psw);
		}
	}

	return OS_OK;
}

void TELNET_AgingIdleVty()
{
	int i = 0;
	time_t t = time(NULL);
	long diff = 0;
	
	for (i == 0; i < CMD_VTY_MAXUSER_NUM; i++)
	{
		if (g_vty[i].valid == 1)
		{
			diff = getdiftime(t, g_vty[i].user.lastAccessTime);  

			/* 两分钟未认证通过，终止vty */
			if (diff > 60*2
				&& g_vty[i].user.state == 0)
			{
				TELNET_Debug(DEBUG_TYPE_INFO, "TELNET_AgingIdleVty do 1.");
				cmd_vty_offline(&g_vty[i]);
				continue;
			}

			/* 48小时无操作，终止vty */
			if (diff > 60*60*24*2
				&& g_vty[i].user.state == 1)
			{
				TELNET_Debug(DEBUG_TYPE_INFO, "TELNET_AgingIdleVty do 2.");
				cmd_vty_offline(&g_vty[i]);
				continue;
			}
		}
	}

	return;
}

unsigned _stdcall TELNET_TimerThread(void *pEntry)
{
	ULONG ulLoop = 0;

	for (;;)
	{		
		/* 1s */
		if (0 == ulLoop % 1)
		{
		
		}
		
		/* 30s*/
		if (0 == ulLoop % 30)
		{

		}

		/* 60s*/
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
	int ret=0;
	WSADATA wsaData;
    WORD sockVersion = MAKEWORD(2, 2);

	if(WSAStartup(sockVersion, &wsaData) != 0)
	{
		return OS_ERR;
	}
	
	g_TelnetSSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(g_TelnetSSocket == INVALID_SOCKET)
	{
		return OS_ERR;
	}

	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(g_telnet_port);
	sin.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	ret = bind(g_TelnetSSocket,(LPSOCKADDR)&sin,sizeof(sin));
	if(SOCKET_ERROR == ret)
	{
		TELNET_Debug(DEBUG_TYPE_ERROR, "TELNET_InitSocket bind failed.");
		closesocket(g_TelnetSSocket);
		return OS_ERR;
	}
	
	printf("TELNET server socket bind port %u ok...\r\n", g_telnet_port);
	
	ret=listen(g_TelnetSSocket, 20);
	if (SOCKET_ERROR == ret)
	{
		TELNET_Debug(DEBUG_TYPE_ERROR, "TELNET_InitSocket listen failed. ");
		closesocket(g_TelnetSSocket);
		WSACleanup();
		return OS_ERR;
	}

	printf("TELNET server socket listen port %u ok...\r\n", g_telnet_port);

	return OS_OK;
}

void TELNET_RecvBuff(struct cmd_vty *vty, char *pbuff, int isPsw)
{
	int ret = 0;
	SOCKET sClient = 0;
	char buff[1] = {0};
	char buff32[32] = {0};
	int index = 0;

	sClient = vty->user.socket;
		
	while(OS_YES == g_TelnetServerEnable)
	{
		ret = recv(sClient,(char*)buff,sizeof(buff), 0);
		if(ret <= 0)
		{
			break;
		}

		//printf("\r\nc=0x%x", buff[0]);
		if (0xd == buff[0])
		{
			break;
		}

		if (0xa == buff[0])
		{
			continue;
		}

		/* 退格 */
		if (0x8 == buff[0])
		{
			if (0 == index)
			{
				buff32[index] = '\0';
				
				if (OS_NO == isPsw)
				{
					send(sClient,(const char*)"\b", sizeof("\b"),0);
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
				send(sClient,(const char*)buff, sizeof(buff),0);
			}
			
			continue;
		}
	}

	memcpy(pbuff, buff32, sizeof(buff32));

	return ;
}

ULONG TELNET_UserAuth(char *uname, char *psw)
{
	if (0 == strlen(g_szTelnetUsername)
		|| 0 == strlen(g_szTelnetPassword))
	{
		return OS_ERR;
	}

	if (0 == strcmp(uname, g_szTelnetUsername)
		&& 0 == strcmp(psw, g_szTelnetPassword))
	{
		return OS_OK;
	}

	return OS_ERR;
}

ULONG TELNET_DoUserAccess(struct cmd_vty *vty)
{
	int ret = 0;
	SOCKET sClient = 0;
	char buff_username[32] = {0};
	char buff_password[32] = {0};

	sClient = vty->user.socket;

	if (TELNET_AUTHMODE_NONE == g_TelnetAuthMode)
	{
		memcpy(vty->user.user_name, buff_username, sizeof(buff_username));
		return OS_OK;
	}
	
	send(sClient,(const char*)"\r\nUsername:", sizeof("\r\nUsername:"),0);
	TELNET_RecvBuff(vty, buff_username, OS_NO);
	send(sClient,(const char*)"\r\nPassword:", sizeof("\r\nPassword:"),0);
	TELNET_RecvBuff(vty, buff_password, OS_YES);

	//printf("\r\nTELNET_DoUserAccess. (name=%s, psw=%s)", buff_username, buff_password);
	
	write_log(JUDGE_INFO,"TELNET_DoUserAccess. (name=%s, psw=%s)", buff_username, buff_password);
	
	//if (OS_OK == SQL_UserLogin(buff_username, buff_password))
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
		if (OS_OK == AAA_UserAuth(buff_username, buff_password))
		{
			memcpy(vty->user.user_name, buff_username, sizeof(buff_username));
			return OS_OK;
		}

		return OS_ERR;
	}

	return OS_ERR;
}

void TELNET_Send_Will(struct cmd_vty *vty, char option)
{
	char buff[3] = {0};
	char buf_option[3] = {0};
	SOCKET sClient = 0;

	sClient = vty->user.socket;
	
	buf_option[0] = TEL_IAC;
	buf_option[1] = TEL_WILL;
	buf_option[2] = option;
	(void)send(sClient,(const char*)buf_option, 3, 0);
	(void)recv(sClient,(char*)buff, 3, 0);

	TELNET_Debug(DEBUG_TYPE_INFO, "TELNET_Send_Will. Ack(0x%x, 0x%x, 0x%x).", buff[0], buff[1], buff[2]);
}

void TELNET_Send_Do(struct cmd_vty *vty, char option)
{
	char buff[3] = {0};
	char buf_option[3] = {0};
	SOCKET sClient = 0;

	sClient = vty->user.socket;

	buf_option[0] = TEL_IAC;
	buf_option[1] = TEL_DO;
	buf_option[2] = option;
	(void)send(sClient,(const char*)buf_option, 3, 0);
	(void)recv(sClient,(char*)buff, 3, 0);
	
	TELNET_Debug(DEBUG_TYPE_INFO, "TELNET_Send_Do. Ack(0x%x, 0x%x, 0x%x).", buff[0], buff[1], buff[2]);
}

unsigned _stdcall TELNET_SessionThread(void *pEntry)
{
	SOCKET sClient = 0;
	char buf_option[4] = {0};
	char buff_hello[1024] = "\r\n=========================>>\r\nWeilcome to judger-kernel.\r\n=========================>>\r\n";
	struct cmd_vty *vty = (struct cmd_vty*)pEntry;

	if (NULL == vty)
	{
		return 0;
	}

	sClient = vty->user.socket;

	/* 服务端激活echo选项 */
	TELNET_Send_Will(vty, TEL_ECHO); 

	vty->user.lastAccessTime = time(NULL);
	
	/* 用户接入 */
	if (OS_OK != TELNET_DoUserAccess(vty))
	{
		cmd_vty_offline(vty);
		return 0;
	}

	/* 用户接入ok */
	vty->user.state = 1;
	vty->user.lastAccessTime = time(NULL);
	send(sClient,(const char*)buff_hello, sizeof(buff_hello),0);
	cmd_outprompt(vty);

	/* 循环读取用户输入 */
	cmd_read(vty);

	/* 用户下线 */
	cmd_vty_offline(vty);

	return 0;
}


unsigned _stdcall TELNET_ListenThread(void *pEntry)
{
	char buff[1024] = {0};
	sockaddr_in remoteAddr;
	SOCKET sClient;
	int nAddrLen = sizeof(remoteAddr);
	struct cmd_vty *vty = NULL;
	
	while(OS_YES == g_TelnetServerEnable)
	{
		sClient = accept(g_TelnetSSocket, (SOCKADDR*)&remoteAddr, &nAddrLen);
		if(sClient == INVALID_SOCKET)
		{
			continue;
		}

		//printf("\r\nAccept new connect. (ip:%s, port:%u, socket=%u)\r\n", inet_ntoa(remoteAddr.sin_addr), htons(remoteAddr.sin_port), sClient);

		vty = cmd_get_idlevty();
		if (NULL == vty)
		{
			sprintf(buff, "\r\nError: Rejected the connection because the judger exeed the max vty number(%u).",CMD_VTY_MAXUSER_NUM);
			send(sClient, (const char*)buff,sizeof(buff), 0);
			
			closesocket(sClient);
			continue;
		}

		vty->user.socket = sClient;
		
		_beginthreadex(NULL, 0, TELNET_SessionThread, vty, NULL, NULL);
		
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
	
	/* 启动监听线程 */
	hTelnetLisent = (HANDLE)_beginthreadex(NULL, 0, TELNET_ListenThread, NULL, NULL, NULL);

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

	CloseHandle(hTelnetLisent);
	hTelnetLisent = NULL;
	
	g_TelnetServerEnable = OS_NO;
	
	closesocket(g_TelnetSSocket);
	g_TelnetSSocket = INVALID_SOCKET;

	/* 踢所有用户下线 */
	for (int i = 0; i < CMD_VTY_MAXUSER_NUM; i++)
	{
		cmd_vty_offline(&g_vty[i]);
	}
	
	TELNET_Debug(DEBUG_TYPE_INFO, "TELNET_ServerDisable ok.");
	
	return OS_OK;
}

ULONG TELNET_BuildRun(CHAR **ppBuildrun)
{
	CHAR *pBuildrun = NULL;

	*ppBuildrun = (CHAR*)malloc(BDN_MAX_BUILDRUN_SIZE);
	if (NULL == *ppBuildrun)
	{
		return OS_ERR;
	}
	memset(*ppBuildrun, 0, BDN_MAX_BUILDRUN_SIZE);
	
	pBuildrun = *ppBuildrun;

	if (OS_YES == g_TelnetServerEnable)
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"telnet server enable");
	}
	else
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"undo telnet server enable");
	}
	
	if (TELNET_AUTHMODE_NONE == g_TelnetAuthMode)
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"telnet authentication-mode none");
	}
	else if (TELNET_AUTHMODE_PASSWORD == g_TelnetAuthMode)
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"telnet authentication-mode password");

		if (strlen(g_szTelnetUsername) > 0 
			&& strlen(g_szTelnetPassword) > 0)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"telnet username %s password %s", g_szTelnetUsername, g_szTelnetPassword);
		}
	}
	else if (TELNET_AUTHMODE_AAA == g_TelnetAuthMode)
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"telnet authentication-mode aaa");
	}

	return OS_OK;
}


int TELNET_Init()
{

	return OS_OK;
}

unsigned _stdcall  TELNET_TaskEntry(void *pEntry)

{
	ULONG ulRet = OS_OK;
	
	(void)BDN_RegistBuildRun(MID_TELNET, BDN_PRIORITY_HIGH + 100, TELNET_BuildRun);
	(void)BDN_RegistBuildRun(MID_TELNET, BDN_PRIORITY_HIGH + 99, AAA_BuildRun);
	
	/* 启动定时器线程 */
	_beginthreadex(NULL, 0, TELNET_TimerThread, NULL, NULL, NULL);
	
	/* 循环读取消息队列 */
	for(;;)
	{
		/* 放权 */
		Sleep(1000);
	}
	
}

APP_INFO_S g_telnetAppInfo =
{
	NULL,
	"TELNET",
	TELNET_Init,
	TELNET_TaskEntry
};

void TELNET_RegAppInfo()
{
	RegistAppInfo(&g_telnetAppInfo);

}



