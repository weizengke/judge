/*
	邻居发现模块: 用于分布式系统中的各个client，以便server灵活调度可用资源判题。
	
	相关命令行:
	ndp server enable --- 使能ndp服务端，属于整个系统的大脑
		ndp server bind port XXX --- 服务端的socket端口绑定(默认 6000)
	ndp client enable --- 使能ndp 客户端，用于分布式部署时client端向服务端发起链接
		ndp server ip x.x.x.x port xxx --- client端用于配置服务端的ip和端口号，以便client发起链接(默认 127.0.0.1: 6000)

	display ndp neighbor --- server端查看上线的客户端
	
*/
#include <iostream>
#include <sstream>
#include <list>
#include <queue>
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

#include "osp/command/include/icli.h"

#include "product/include/pdt_common_inc.h"
#include "product/main/root.h"
#include "osp/common/include/osp_common_def.h"
#include "osp/event/include/event_pub.h"
#include "osp/debug/include/debug_center.h"

#if (OS_YES == OSP_MODULE_NDP)

using namespace std;


#define NDP_CLIENT_MAX_SPEC 512 /* 支持client 512个 */
#define NDP_IP_LEN 16

ULONG g_ulNdpServerEnable = OS_NO;
ULONG g_ulNdpClientEnable = OS_NO;

char g_szNdpServerIP[NDP_IP_LEN]="127.0.0.1";
ULONG g_ulNdpServerPort = 6000;
SOCKET g_ndpClientSocket = INVALID_SOCKET;
SOCKET g_ndpServerSocket = INVALID_SOCKET;

char g_szNdpPeerIP[NDP_IP_LEN]="127.0.0.1"; /* for client */
ULONG g_ulNdpPeerPort = 6000; /* for client*/

#define NDP_AGE_TIME 120


HANDLE hServerThread = NULL;
HANDLE hClientThread = NULL;


enum NDP_MSG_TYPE_EM {
	NDP_MSG_TYPE_NONE,
		
	NDP_MSG_TYPE_ONLINE,
	NDP_MSG_TYPE_OFFLINE,
	NDP_MSG_TYPE_ECHO,
	
	NDP_MSG_TYPE_MAX
};


typedef struct tag_NDP_PACKET_ST
{
	ULONG ulMsgType;
	char szSysName[24];

}NDP_PACKET_ST;

typedef struct tag_NDP_NEIGHBORS_ST
{
	char szSysName[24];
	ULONG ulIp;
	ULONG ulPort;
	ULONG ulAgeTime;
	
}NDP_NEIGHBORS_ST;

typedef list<NDP_NEIGHBORS_ST *> NDP_NEIGHBORS_LIST;

NDP_NEIGHBORS_LIST g_NdpNeighborsList;

#define NDP_Debug(x, args...) debugcenter_print(MID_NDP, x, args)


char* GetIpFromULong(ULONG uIp)   
{   
    in_addr addr;   
  
    memcpy(&addr, &uIp, sizeof(uIp));   
  
    return inet_ntoa(addr);   
}   

ULONG NDP_NeighborAdd(NDP_NEIGHBORS_ST * pstNeighbor)
{
	if (NULL == pstNeighbor)
	{
		return OS_ERR;
	}

	for(NDP_NEIGHBORS_LIST::iterator UserIterator = g_NdpNeighborsList.begin();
			UserIterator != g_NdpNeighborsList.end();
			++UserIterator)
	{
		if(pstNeighbor->ulIp == (*UserIterator)->ulIp
			&& pstNeighbor->ulPort == (*UserIterator)->ulPort)
		{
			memcpy((*UserIterator)->szSysName, pstNeighbor->szSysName, sizeof((*UserIterator)->szSysName));
			(*UserIterator)->ulAgeTime = NDP_AGE_TIME;
			return OS_OK;
		}
	}	
			
	NDP_NEIGHBORS_ST *pstNeighborTmp = new NDP_NEIGHBORS_ST();

	memcpy(pstNeighborTmp, pstNeighbor, sizeof(NDP_NEIGHBORS_ST));
	pstNeighborTmp->ulAgeTime =  NDP_AGE_TIME;
	
	g_NdpNeighborsList.push_back(pstNeighborTmp);

	NDP_Debug(DEBUG_TYPE_MSG, "A NDP neighbor online. (sysname:%s, ip:%s, port:%u)",
					pstNeighborTmp->szSysName,
					GetIpFromULong(htonl(pstNeighborTmp->ulIp)),
					pstNeighborTmp->ulPort);
	
	
	return OS_OK;
}

ULONG NDP_NeighborAll()
{
	g_NdpNeighborsList.clear();
			
	return OS_OK;
}


ULONG NDP_NeighborDel(NDP_NEIGHBORS_ST * pstNeighbor)
{
	if (NULL == pstNeighbor)
	{
		return OS_ERR;
	}

	for(NDP_NEIGHBORS_LIST::iterator UserIterator = g_NdpNeighborsList.begin();
			UserIterator != g_NdpNeighborsList.end();)
	{
		if(pstNeighbor->ulIp == (*UserIterator)->ulIp
			&& pstNeighbor->ulPort == (*UserIterator)->ulPort)
		{
			g_NdpNeighborsList.erase(UserIterator);	
			return OS_OK;
		}
	}

	return OS_OK;
}

void NDP_NeighborsAge()
{
	for(NDP_NEIGHBORS_LIST::iterator UserIterator = g_NdpNeighborsList.begin();
			UserIterator != g_NdpNeighborsList.end();)
	{
		if((*UserIterator)->ulAgeTime <= 1)
		{
			UserIterator = g_NdpNeighborsList.erase(UserIterator);	
		}
		else
		{
			(*UserIterator)->ulAgeTime--;
			++UserIterator;
		}
	}
}

void NDP_ShowAllNeighbors(ULONG vtyId)
{
	vty_printf(vtyId, "Neighbors (%u):\r\n", g_NdpNeighborsList.size());
	vty_printf(vtyId, " Sysname        IP               Port     Age(s)\r\n");
	vty_printf(vtyId, " -----------------------------------------------\r\n");
		
	for(NDP_NEIGHBORS_LIST::iterator UserIterator = g_NdpNeighborsList.begin();
			UserIterator != g_NdpNeighborsList.end();
			++UserIterator)
	{
		vty_printf(vtyId, " %-13s  %-16s %-8u %-4u\r\n", (*UserIterator)->szSysName, GetIpFromULong(htonl((*UserIterator)->ulIp)), (*UserIterator)->ulPort, (*UserIterator)->ulAgeTime);
	}
}

SOCKET NDP_GetServerSocket()
{
	return g_ndpServerSocket;
}

SOCKET NDP_GetClientSocket()
{
	return g_ndpClientSocket;
}

ULONG NDP_InitWinSock()
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		NDP_Debug(DEBUG_TYPE_ERROR, "Windows sockets 2.2 startup error.");
		return OS_ERR;
	}

	return OS_OK;
}


SOCKET NDP_mksock(int type)
{
	SOCKET sock = socket(AF_INET, type, IPPROTO_IP);
	if (sock < 0)
	{
        NDP_Debug(DEBUG_TYPE_ERROR, "create socket error.");
		return INVALID_SOCKET;
	}
	
	return sock;
}


int NDP_ServerListenThread(void *pEntry)
{
	sockaddr_in remoteAddr;
	int nAddrLen = sizeof(remoteAddr);
	NDP_PACKET_ST stNdpPacket = {0};

	NDP_Debug(DEBUG_TYPE_FUNC, "NDP server listen thread run.");
	
	while(g_ulNdpServerEnable)
	{
		int ret = recvfrom(NDP_GetServerSocket(), (char *)&stNdpPacket, sizeof(NDP_PACKET_ST), 0, (sockaddr *)&remoteAddr, &nAddrLen);
		if(ret <= 0)
		{
			continue;
		}
		else
		{
			NDP_Debug(DEBUG_TYPE_FUNC, "Server Recv msg from %s(%u): %u.", inet_ntoa(remoteAddr.sin_addr), htons(remoteAddr.sin_port), stNdpPacket.ulMsgType);

			NDP_NEIGHBORS_ST stNeighbor = {0};
			stNeighbor.ulIp = ntohl(remoteAddr.sin_addr.S_un.S_addr);
			stNeighbor.ulPort =  ntohs(remoteAddr.sin_port);
			memcpy(stNeighbor.szSysName, stNdpPacket.szSysName, sizeof(stNeighbor.szSysName));
			(void)NDP_NeighborAdd(&stNeighbor);
				
			stNdpPacket.ulMsgType = NDP_MSG_TYPE_ECHO;
			sendto(NDP_GetServerSocket(), (char*)&stNdpPacket, sizeof(NDP_PACKET_ST), 0, (const sockaddr*)&remoteAddr, nAddrLen);
		}
		
		Sleep(1);
	}

	NDP_Debug(DEBUG_TYPE_FUNC, "NDP server listen thread closed.");
	
	return 0;
}


int NDP_ClientListenThread(void *pEntry)
{
	sockaddr_in remoteAddr;
	int nAddrLen = sizeof(remoteAddr);
	NDP_PACKET_ST stNdpPacket = {0};

	NDP_Debug(DEBUG_TYPE_FUNC, "NDP client listen thread run.");
	
	while(g_ulNdpClientEnable)
	{
		int ret = recvfrom(NDP_GetClientSocket(), (char *)&stNdpPacket, sizeof(NDP_PACKET_ST), 0, (sockaddr *)&remoteAddr, &nAddrLen);
		if(ret <= 0)
		{
			continue;
		}
		else
		{
			NDP_Debug(DEBUG_TYPE_FUNC, "Client Recv msg from %s(%u): %u.", inet_ntoa(remoteAddr.sin_addr), htons(remoteAddr.sin_port), stNdpPacket.ulMsgType);
		}
		
		Sleep(1);
	}

	NDP_Debug(DEBUG_TYPE_FUNC, "NDP client listen thread closed.");
	
	return 0;
}


ULONG NDP_ServerEnable()
{
	ULONG ulRet = OS_OK;
	WSADATA wsaData;
    WORD sockVersion = MAKEWORD(2, 2);

	ulRet = NDP_InitWinSock();
	if (OS_OK != ulRet)
	{
		return OS_ERR;
	}

	g_ndpServerSocket = NDP_mksock(SOCK_DGRAM);
	if (INVALID_SOCKET == g_ndpServerSocket)
	{
		return OS_ERR;
	}
	
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(g_ulNdpServerPort);
	sin.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	ulRet = bind(g_ndpServerSocket,(LPSOCKADDR)&sin,sizeof(sin));
	if(ulRet == SOCKET_ERROR)
	{
		NDP_Debug(DEBUG_TYPE_ERROR, "NDP_ServerEnable bind failed. ");
		closesocket(g_ndpServerSocket);
		return OS_ERR;
	}

	NDP_Debug(DEBUG_TYPE_FUNC, "NDP server socket bind port %u.", g_ulNdpServerPort);

	hServerThread = (HANDLE)thread_create(NDP_ServerListenThread, NULL);

	return OS_OK;
}

ULONG NDP_ClientEnable()
{
	ULONG ulRet = OS_OK;

	ulRet = NDP_InitWinSock();
	if (OS_OK != ulRet)
	{
		return OS_ERR;
	}

	g_ndpClientSocket = NDP_mksock(SOCK_DGRAM);
	if(g_ndpClientSocket == INVALID_SOCKET)
	{
		NDP_Debug(DEBUG_TYPE_ERROR, "NDP_ConnectToServer socket error");
		return OS_ERR;
	}

	sockaddr_in sin;
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	sin.sin_family = AF_INET;
	sin.sin_port = 0;
	
	if (bind(g_ndpClientSocket, (struct sockaddr*)&sin, sizeof(sin)) < 0)
	{
		NDP_Debug(DEBUG_TYPE_ERROR, "NDP client bind socket error");
		return OS_ERR;
	}	

	 int len = sizeof(sin);
	 getsockname(g_ndpClientSocket,(sockaddr*)&sin,&len);

	 NDP_Debug(DEBUG_TYPE_FUNC, "NDP client socket bind port %u.", ntohs(sin.sin_port));

	hClientThread = (HANDLE)thread_create(NDP_ClientListenThread, NULL);
	
	return OS_OK;
}

ULONG NDP_ServerDisable()
{
	if (OS_NO == g_ulNdpServerEnable)
	{
		NDP_Debug(DEBUG_TYPE_FUNC, "ndp server is not enable.");
		return OS_ERR;
	}

	CloseHandle(hServerThread);
	hServerThread = NULL;
	
	g_ulNdpServerEnable = OS_NO;
	
	closesocket(g_ndpServerSocket);
	g_ndpServerSocket = INVALID_SOCKET;

	NDP_NeighborAll();
	
	NDP_Debug(DEBUG_TYPE_FUNC, "NDP_ServerDisable ok.");
}


ULONG NDP_ClientDisable()
{
	if (OS_NO == g_ulNdpClientEnable)
	{
		NDP_Debug(DEBUG_TYPE_FUNC, "ndp client is not enable.");
		return OS_ERR;
	}

	CloseHandle(hClientThread);
	hClientThread = NULL;
	
	g_ulNdpClientEnable = OS_NO;
	
	closesocket(g_ndpClientSocket);
	g_ndpClientSocket = INVALID_SOCKET;
	
	NDP_Debug(DEBUG_TYPE_FUNC, "NDP_ClientDisable ok.");
}

ULONG NDP_ConnectToServer(char *zsServerip, int port)
{
	int iRet = OS_OK;
	sockaddr_in servAddr;
	SOCKET sockConn = NDP_GetClientSocket();

	if (INVALID_SOCKET == sockConn)
	{
		NDP_Debug(DEBUG_TYPE_ERROR, "NDP_GetClientSocket failed.");
		return OS_ERR;
	}
	
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(port);
	servAddr.sin_addr.S_un.S_addr = inet_addr(zsServerip);
	int nAddrLen = sizeof(servAddr);

	NDP_PACKET_ST stNdpPacket = {0};
	stNdpPacket.ulMsgType = NDP_MSG_TYPE_ONLINE;
	memcpy(stNdpPacket.szSysName, g_sysname, sizeof(stNdpPacket.szSysName));
	iRet = sendto(sockConn, (const char*)&stNdpPacket, sizeof(NDP_PACKET_ST), 0, (const sockaddr*)&servAddr, nAddrLen);
	if (SOCKET_ERROR == iRet)
	{
		NDP_Debug(DEBUG_TYPE_ERROR, "NDP_ConnectToServer send packet failed. (zsServerip=%s, port=%u)", zsServerip, port);
	}

	NDP_Debug(DEBUG_TYPE_FUNC, "NDP_ConnectToServer send packet. (zsServerip=%s, port=%u)", zsServerip, port);
	
	return OS_OK;
}


ULONG NDP_SH_ServerEnable()
{
	ULONG ulRet = OS_OK;
	WSADATA wsaData;
    WORD sockVersion = MAKEWORD(2, 2);

	if (OS_YES == g_ulNdpServerEnable)
	{
		printf("Info: ndp is already enable.\r\n");
		return OS_ERR;
	}

	g_ulNdpServerEnable = OS_YES;

	ulRet = NDP_ServerEnable();
	if(ulRet != OS_OK)
	{
		printf("Error: ndp server enable failed.\r\n");
		g_ulNdpServerEnable = OS_NO;
		return OS_ERR;
	}
	
	return OS_OK;
}

ULONG NDP_SH_ClientEnable()
{
	ULONG ulRet = OS_OK;
	
	if (OS_YES == g_ulNdpClientEnable)
	{
		printf("Info: ndp client is already enable.\r\n");
		return OS_ERR;
	}

	g_ulNdpClientEnable = OS_YES;

	ulRet = NDP_ClientEnable();
	if(ulRet != OS_OK)
	{
		printf("Error: ndp client enable failed.\r\n");
		g_ulNdpClientEnable = OS_NO;
		return OS_ERR;
	}

	NDP_ConnectToServer(g_szNdpPeerIP, g_ulNdpPeerPort);
	
	return OS_OK;
}


void NDP_SetServerIPPort(char *ip, int port)
{
	strcpy(g_szNdpPeerIP, ip);
	g_ulNdpPeerPort = port;
		
	if (OS_NO == g_ulNdpClientEnable)
	{
		NDP_Debug(DEBUG_TYPE_ERROR, "ndp client is not enable.");
		return ;
	}

	(void)NDP_ClientDisable();

	Sleep(200);
	
	(void)NDP_SH_ClientEnable();
	
	return;
}

void NDP_SetServerBindPort(int port)
{
	g_ulNdpServerPort = port;
	
	if (OS_NO == g_ulNdpServerEnable)
	{
		NDP_Debug(DEBUG_TYPE_ERROR,"ndp server is not enable.");
		return ;
	}
	
	/* 去使能ndp server，在使能 */
	(void)NDP_ServerDisable();
	
	Sleep(200);

	(void)NDP_SH_ServerEnable();
	
	return;
}


int NDP_TimerThread(void *pEntry)
{
	ULONG ulLoop = 0;

	for (;;)
	{		
		/* 1s */
		if (0 == ulLoop % 1)
		{
			NDP_NeighborsAge();
		}
		
		/* 30s*/
		if (0 == ulLoop % 30)
		{
			if (OS_YES == g_ulNdpClientEnable)
			{
				NDP_ConnectToServer(g_szNdpPeerIP, g_ulNdpPeerPort);
			}
		}

		ulLoop++;
		
		Sleep(1000);
	}

	return 0;
}


ULONG NDP_BuildRun(CHAR **ppBuildrun, ULONG ulIncludeDefault)
{
	CHAR *pBuildrun = NULL;

	*ppBuildrun = (CHAR*)malloc(BDN_MAX_BUILDRUN_SIZE);
	if (NULL == *ppBuildrun)
	{
		return OS_ERR;
	}
	memset(*ppBuildrun, 0, BDN_MAX_BUILDRUN_SIZE);
	
	pBuildrun = *ppBuildrun;

	if (OS_YES == g_ulNdpServerEnable)
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"ndp server enable");		
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"ndp server bind port %u", g_ulNdpServerPort);
	}
	else
	{
		if (OS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"undo ndp server enable");
		}
	}

	if (OS_YES == g_ulNdpClientEnable)
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"ndp client enable");		
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"ndp server ip %s port %u", g_szNdpPeerIP, g_ulNdpPeerPort);
	}
	else
	{
		if (OS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"undo ndp client enable");
		}
	}

	return OS_OK;
}


int NDP_Init()
{

	return OS_OK;
}

int NDP_TaskEntry(void *pEntry)

{
	ULONG ulRet = OS_OK;

	//(void)BDN_RegistBuildRun(MID_NDP, VIEW_SYSTEM, BDN_PRIORITY_NORMAL, NDP_BuildRun);
	
	/* 启动定时器线程 */
	(VOID)thread_create(NDP_TimerThread, NULL);
	
	/* 循环读取消息队列 */
	for(;;)
	{
		/* 放权 */
		Sleep(1000);
	}
	
}

APP_INFO_S g_ndpAppInfo =
{
	NULL,
	"NDP",
	NDP_Init,
	NDP_TaskEntry
};

void NDP_RegAppInfo()
{
	APP_RegistInfo(&g_ndpAppInfo);

}

#endif
