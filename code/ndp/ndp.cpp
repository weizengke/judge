/*
	�ھӷ���ģ��: ���ڷֲ�ʽϵͳ�еĸ���client���Ա�server�����ȿ�����Դ���⡣
	
	���������:
	ndp server enable --- ʹ��ndp����ˣ���������ϵͳ�Ĵ���
		ndp server bind port XXX --- ����˵�socket�˿ڰ�(Ĭ�� 6000)
	ndp client enable --- ʹ��ndp �ͻ��ˣ����ڷֲ�ʽ����ʱclient�������˷�������
		ndp server ip x.x.x.x port xxx --- client���������÷���˵�ip�Ͷ˿ںţ��Ա�client��������(Ĭ�� 127.0.0.1: 6000)

	display ndp neighbor --- server�˲鿴���ߵĿͻ���
	
*/

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
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#else
#include <io.h>
#include <time.h>
#include <windows.h>
#endif

#include "util/utlist.h"
#include "kernel.h"
#include "icli.h"
#include "pdt_common_inc.h"
#include "root.h"
#include "osp_common_def.h"
#include "event/include/event_pub.h"
#include "ic/include/debug_center_inc.h"

#if (OS_YES == OSP_MODULE_NDP)

using namespace std;


#define NDP_CLIENT_MAX_SPEC 512 /* ֧��client 512�� */
#define NDP_IP_LEN 16

ULONG g_ulNdpServerEnable = OS_NO;
ULONG g_ulNdpClientEnable = OS_NO;

char g_szNdpServerIP[NDP_IP_LEN]="127.0.0.1";
ULONG g_ulNdpServerPort = 6000;
socket_t g_ndpClientSocket = INVALID_SOCKET;
socket_t g_ndpServerSocket = INVALID_SOCKET;

char g_szNdpPeerIP[NDP_IP_LEN]="255.255.255.255"; /* for client */
ULONG g_ulNdpPeerPort = 6000; /* for client*/

#define NDP_AGE_TIME 120


thread_id_t hServerThread = NULL;
thread_id_t hClientThread = NULL;

enum NDP_CMO_TBLID_EM {
	NDP_CMO_TBL_SHOW = 0,
	NDP_CMO_TBL_CFG,		
};

enum NDP_CMO_SHOW_ID_EM {
	NDP_CMO_SHOW_NEIGHBOR = CMD_ELEMID_DEF(MID_NDP, NDP_CMO_TBL_SHOW, 0),
};

enum NDP_CMO_CFG_ID_EM {
	NDP_CMO_CFG_UNDO = CMD_ELEMID_DEF(MID_NDP, NDP_CMO_TBL_CFG, 0),
	NDP_CMO_CFG_SERVER,
	NDP_CMO_CFG_CLIENT,
	NDP_CMO_CFG_ENABLE,
	NDP_CMO_CFG_SERVER_PORT,
	NDP_CMO_CFG_PEER_IP,
	NDP_CMO_CFG_PEER_PORT,
};

enum NDP_MSG_TYPE_EM {
	NDP_MSG_TYPE_NONE,
	NDP_MSG_TYPE_ECHO,
	NDP_MSG_TYPE_DOWN,

	NDP_MSG_TYPE_MAX
};

typedef struct tag_NDP_PACKET_ST
{
	ULONG ulMsgType;
	char szSysName[24];
}NDP_PACKET_ST;

typedef struct tag_ndp_neighbors_s 
{
	char szSysName[24];
	ULONG ulIp;
	ULONG ulPort;
	ULONG ulAgeTime;
	struct tag_ndp_neighbors_s *prev;
	struct tag_ndp_neighbors_s *next;
} ndp_neighbors_s;

ndp_neighbors_s *g_ndp_neighbors_head = NULL;

#define NDP_Debug(x, format, ...) debugcenter_print(MID_NDP, x, format, ##__VA_ARGS__)

char* NDP_GetIpFromULong(ULONG uIp)   
{   
    in_addr addr;   
  
    memcpy(&addr, &uIp, sizeof(uIp));   
  
    return inet_ntoa(addr);   
}   

int NDP_NeighborCompare(void *a, void *b) {
	ndp_neighbors_s *key1 = (ndp_neighbors_s*)a;
	ndp_neighbors_s *key2 = (ndp_neighbors_s*)b;

	if (key1->ulIp == key2->ulIp) {
		return key1->ulPort - key2->ulPort;
	}

    return key1->ulIp - key2->ulIp;
}

ULONG NDP_NeighborAdd(ndp_neighbors_s * pstNeighbor)
{
	if (NULL == pstNeighbor) {
		return OS_ERR;
	}

	ndp_neighbors_s *neibor = NULL;
	ndp_neighbors_s key = {0};
	key.ulIp = pstNeighbor->ulIp;
	key.ulPort = pstNeighbor->ulPort;
	DL_SEARCH(g_ndp_neighbors_head, neibor, &key, NDP_NeighborCompare);
	if (neibor != NULL) {
		memcpy(neibor->szSysName, pstNeighbor->szSysName, sizeof(neibor->szSysName));
		neibor->ulAgeTime = NDP_AGE_TIME;
		NDP_Debug(DEBUG_TYPE_MSG, "A NDP neighbor update. (sysname:%s, ip:%s, port:%u)",
					pstNeighbor->szSysName,
					NDP_GetIpFromULong(htonl(pstNeighbor->ulIp)),
					pstNeighbor->ulPort);		
		return OS_OK;
	}

	ndp_neighbors_s *neibor_new = (ndp_neighbors_s*)malloc(sizeof(ndp_neighbors_s));
	memset(neibor_new, 0, sizeof(ndp_neighbors_s));
	neibor_new->ulIp = pstNeighbor->ulIp;
	neibor_new->ulPort = pstNeighbor->ulPort;
	neibor_new->ulAgeTime =  NDP_AGE_TIME;
	strcpy(neibor_new->szSysName, pstNeighbor->szSysName);
	
	DL_APPEND(g_ndp_neighbors_head, neibor_new);

	NDP_Debug(DEBUG_TYPE_MSG, "A NDP neighbor add. (sysname:%s, ip:%s, port:%u)",
				pstNeighbor->szSysName,
				NDP_GetIpFromULong(htonl(pstNeighbor->ulIp)),
				pstNeighbor->ulPort);
	
	return OS_OK;
}

ULONG NDP_NeighborClearAll()
{
	ndp_neighbors_s *neibor, *tmp;
    DL_FOREACH_SAFE(g_ndp_neighbors_head, neibor, tmp) {
		DL_DELETE(g_ndp_neighbors_head, neibor);
		free(neibor);
	}

	return OS_OK;
}


VOID NDP_NeighborDel(ndp_neighbors_s * pstNeighbor)
{
	if (NULL == pstNeighbor) {
		return ;
	}

	ndp_neighbors_s *neibor, *tmp;
	DL_FOREACH_SAFE(g_ndp_neighbors_head, neibor, tmp) {
		if (pstNeighbor->ulIp == neibor->ulIp
			&& pstNeighbor->ulPort == neibor->ulPort) {
			DL_DELETE(g_ndp_neighbors_head, neibor);
			free(neibor);
			return ;
		}
	}

	return ;
}

void NDP_NeighborsAge()
{
	ndp_neighbors_s *neibor, *tmp;
	DL_FOREACH_SAFE(g_ndp_neighbors_head, neibor, tmp) {		
		if (neibor->ulAgeTime <= 1) {
			DL_DELETE(g_ndp_neighbors_head, neibor);
			free(neibor);
			continue;
		}
		neibor->ulAgeTime--;
	}
}

ULONG NDP_NeighborsCount()
{
	ndp_neighbors_s *neibor;
	ULONG count = 0;
	DL_COUNT(g_ndp_neighbors_head, neibor, count);

	return count;
}

void NDP_ShowAllNeighbors(ULONG vtyId)
{
	vty_printf(vtyId, "Neighbors (%u):\r\n", NDP_NeighborsCount());
	vty_printf(vtyId, " Sysname        IP               Port     Age(s)\r\n");
	vty_printf(vtyId, " -----------------------------------------------\r\n");

	ndp_neighbors_s *neibor;
	DL_FOREACH(g_ndp_neighbors_head, neibor) {
		vty_printf(vtyId, " %-13s  %-16s %-8u %-4u\r\n", 
					neibor->szSysName, 
					NDP_GetIpFromULong(htonl(neibor->ulIp)), 
					neibor->ulPort, 
					neibor->ulAgeTime);
	}
}

socket_t NDP_GetServerSocket()
{
	return g_ndpServerSocket;
}

socket_t NDP_GetClientSocket()
{
	return g_ndpClientSocket;
}

socket_t NDP_Socket(int type)
{
	socket_t sock = socket(AF_INET, type, IPPROTO_IP);
	if (sock < 0) {
        NDP_Debug(DEBUG_TYPE_ERROR, "create socket error.");
		return INVALID_SOCKET;
	}
	
	return sock;
}

int NDP_ProcessMsgEcho(ULONG ip, USHORT port, NDP_PACKET_ST *packet)
{
	ndp_neighbors_s stNeighbor = {0};
	stNeighbor.ulIp = ip; ;
	stNeighbor.ulPort = port;
	memcpy(stNeighbor.szSysName, packet->szSysName, sizeof(packet->szSysName));
	(void)NDP_NeighborAdd(&stNeighbor);

	return OS_OK;
}

int NDP_ProcessMsgDown(ULONG ip, USHORT port, NDP_PACKET_ST *packet)
{
	ndp_neighbors_s stNeighbor = {0};
	stNeighbor.ulIp = ip; ;
	stNeighbor.ulPort = port;
	(void)NDP_NeighborDel(&stNeighbor);

	return OS_OK;
}

int NDP_ProcessMsg(ULONG ip, USHORT port, VOID *data, ULONG dataLen)
{
	NDP_PACKET_ST *packet = (NDP_PACKET_ST *)data;
	if (dataLen != sizeof(NDP_PACKET_ST)) {
		NDP_Debug(DEBUG_TYPE_ERROR, "process msg, data length %u is invalid.", dataLen);
		return OS_ERR;
	}

	switch (packet->ulMsgType) {
		case NDP_MSG_TYPE_ECHO:
			(VOID)NDP_ProcessMsgEcho(ip, port, packet);
			break;
		case NDP_MSG_TYPE_DOWN:
			(VOID)NDP_ProcessMsgDown(ip, port, packet);
		default:
			break;
	}

	return OS_OK;
}

ULONG NDP_SendMsg(socket_t sock, ULONG ip, USHORT port, VOID *data, ULONG dataLen)
{
	int iRet = OS_OK;
	sockaddr_in servAddr;

	if (INVALID_SOCKET == sock) {
		NDP_Debug(DEBUG_TYPE_ERROR, "socket invalid.");
		return OS_ERR;
	}

	/* 广播，获取网卡广播地址 */
#ifdef _LINUX_	
	if (ip == 0xFFFFFFFF) {
		int j = 0;
		struct ifconf ifc;
		struct ifreq *ifr;
		char buffer[1024];
		ifc.ifc_len = sizeof(buffer);
		ifc.ifc_buf = buffer;
		if (ioctl(sock, SIOCGIFCONF, (char *) &ifc) < 0){
			NDP_Debug(DEBUG_TYPE_ERROR, "NDP ioctl-conf:");
			return -1;
		}
		ifr = ifc.ifc_req;
		for (j = ifc.ifc_len / sizeof(struct ifreq); --j >= 0; ifr++) {
			if (!strcmp(ifr->ifr_name, "eth0")) {
				if (ioctl(sock, SIOCGIFFLAGS, (char *) ifr) < 0) {
					NDP_Debug(DEBUG_TYPE_ERROR, "NDP ioctl-get flag failed:");
				}
				break;
			}
		}

		if (ioctl(sock, SIOCGIFBRDADDR, ifr) == -1) {
			NDP_Debug(DEBUG_TYPE_ERROR, "NDP ioctl get BRDADDR failed.");
			return OS_ERR;
		}		
		memcpy(&servAddr, (char *)&ifr->ifr_broadaddr, sizeof(sockaddr_in));
	} else 
#endif	
	{
		servAddr.sin_addr.s_addr = htonl(ip);
	}
	
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(port);
	
	iRet = sendto(sock, (const char*)data, dataLen, 0, (const sockaddr*)&servAddr, sizeof(servAddr));
	if (SOCKET_ERROR == iRet) {
		NDP_Debug(DEBUG_TYPE_ERROR, "NDP send packet failed. (ip=%s, port=%u)",
				  inet_ntoa(servAddr.sin_addr), port);
	}

	NDP_Debug(DEBUG_TYPE_FUNC, "NDP send packet. (ip=%s, port=%u)", 
			  inet_ntoa(servAddr.sin_addr), port);
	
	return OS_OK;
}

ULONG NDP_SendMsgEcho(socket_t sock, ULONG ip, USHORT port)
{
	NDP_PACKET_ST stNdpPacket = {0};

	stNdpPacket.ulMsgType = NDP_MSG_TYPE_ECHO;
	memcpy(stNdpPacket.szSysName, g_sysname, sizeof(stNdpPacket.szSysName));

	return NDP_SendMsg(sock, ip, port, &stNdpPacket, sizeof(NDP_PACKET_ST));
}	

ULONG NDP_SendMsgDown(socket_t sock, ULONG ip, USHORT port)
{
	NDP_PACKET_ST stNdpPacket = {0};

	stNdpPacket.ulMsgType = NDP_MSG_TYPE_DOWN;
	return NDP_SendMsg(sock, ip, port, &stNdpPacket, sizeof(NDP_PACKET_ST));
}	

void NDP_SendEchoToAllClient()
{
	ndp_neighbors_s *neibor;
	DL_FOREACH(g_ndp_neighbors_head, neibor) {
		NDP_SendMsgEcho(NDP_GetServerSocket(), neibor->ulIp, neibor->ulPort);
	}
}

int NDP_ServerListenThread(void *pEntry)
{
	sockaddr_in remoteAddr;
	socklen_t nAddrLen = sizeof(remoteAddr);
	NDP_PACKET_ST stNdpPacket = {0};

	NDP_Debug(DEBUG_TYPE_FUNC, "NDP server listen thread run.");
	
	while (g_ulNdpServerEnable) {
		int ret = recvfrom(NDP_GetServerSocket(), (char *)&stNdpPacket, sizeof(NDP_PACKET_ST),
						   0, (sockaddr *)&remoteAddr, &nAddrLen);
		if(ret <= 0) {
			NDP_Debug(DEBUG_TYPE_FUNC, "recvfrom failed. ret=%d", ret);
			Sleep(1);
			continue;
		} else {
			NDP_Debug(DEBUG_TYPE_FUNC, "Server Recv msg from %x:%u, msgType:%u.",
					  ntohl(remoteAddr.sin_addr.s_addr), htons(remoteAddr.sin_port), stNdpPacket.ulMsgType);

			(VOID)NDP_ProcessMsg(ntohl(remoteAddr.sin_addr.s_addr), 
								 ntohs(remoteAddr.sin_port),
								 (VOID*)&stNdpPacket, 
								 ret);
		}
		Sleep(1);
	}

	NDP_Debug(DEBUG_TYPE_FUNC, "NDP server listen thread closed.");
	
	return 0;
}

int NDP_ClientListenThread(void *pEntry)
{
	sockaddr_in remoteAddr;
	socklen_t nAddrLen = sizeof(remoteAddr);
	NDP_PACKET_ST stNdpPacket = {0};

	NDP_Debug(DEBUG_TYPE_FUNC, "NDP client listen thread run.");
	
	while(g_ulNdpClientEnable) {
		int ret = recvfrom(NDP_GetClientSocket(), (char *)&stNdpPacket, sizeof(NDP_PACKET_ST), 0, (sockaddr *)&remoteAddr, &nAddrLen);
		if(ret <= 0) {
			NDP_Debug(DEBUG_TYPE_FUNC, "client recvfrom failed. ret=%d", ret);
			Sleep(1);
			continue;
		} else {
			NDP_Debug(DEBUG_TYPE_FUNC, "Client Recv msg(len=%d) from %x:%u, msgType:%u.", 
						ret, ntohl(remoteAddr.sin_addr.s_addr),
						htons(remoteAddr.sin_port), stNdpPacket.ulMsgType);

			(VOID)NDP_ProcessMsg(ntohl(remoteAddr.sin_addr.s_addr), 
								 ntohs(remoteAddr.sin_port),
								 (VOID*)&stNdpPacket, 
								 ret);
		}
		
		Sleep(1);
	}

	NDP_Debug(DEBUG_TYPE_FUNC, "NDP client listen thread closed.");
	
	return 0;
}


ULONG NDP_ServerEnable()
{
	ULONG ulRet = OS_OK;

	g_ndpServerSocket = NDP_Socket(SOCK_DGRAM);
	if (INVALID_SOCKET == g_ndpServerSocket) {
		return OS_ERR;
	}
	
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(g_ulNdpServerPort);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	ulRet = bind(g_ndpServerSocket, (const sockaddr*)&sin, sizeof(sin));
	if(ulRet == SOCKET_ERROR) {
		NDP_Debug(DEBUG_TYPE_ERROR, "NDP_ServerEnable bind failed. ");
		closesocket(g_ndpServerSocket);
		return OS_ERR;
	}

	NDP_Debug(DEBUG_TYPE_FUNC, "NDP server socket bind port %u.", g_ulNdpServerPort);

	hServerThread = thread_create(NDP_ServerListenThread, NULL);

	return OS_OK;
}

ULONG NDP_ClientEnable()
{
	ULONG ulRet = OS_OK;

	g_ndpClientSocket = NDP_Socket(SOCK_DGRAM);
	if (g_ndpClientSocket == INVALID_SOCKET) {
		NDP_Debug(DEBUG_TYPE_ERROR, "NDP Socket socket error");
		return OS_ERR;
	}

	sockaddr_in sin;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_family = AF_INET;
	sin.sin_port = 0;
	
	if (bind(g_ndpClientSocket, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
		NDP_Debug(DEBUG_TYPE_ERROR, "NDP client bind socket error");
		return OS_ERR;
	}	

	bool optval = true;
    setsockopt(g_ndpClientSocket, SOL_SOCKET, SO_REUSEADDR | SO_BROADCAST, (char*)&optval, sizeof(bool));

	socklen_t len = sizeof(sin);
	getsockname(g_ndpClientSocket,(sockaddr*)&sin, &len);

	NDP_Debug(DEBUG_TYPE_FUNC, "NDP client socket bind port %u.", ntohs(sin.sin_port));

	hClientThread = thread_create(NDP_ClientListenThread, NULL);
	
	return OS_OK;
}

ULONG NDP_ServerDisable()
{
	if (OS_NO == g_ulNdpServerEnable) {
		NDP_Debug(DEBUG_TYPE_FUNC, "ndp server is not enable.");
		return OS_ERR;
	}

	g_ulNdpServerEnable = OS_NO;
	Sleep(1000);

	hServerThread = NULL;
	
	closesocket(g_ndpServerSocket);
	g_ndpServerSocket = INVALID_SOCKET;

	NDP_NeighborClearAll();
	
	NDP_Debug(DEBUG_TYPE_FUNC, "NDP_ServerDisable ok.");
}


ULONG NDP_ClientDisable()
{
	if (OS_NO == g_ulNdpClientEnable) {
		NDP_Debug(DEBUG_TYPE_FUNC, "ndp client is not enable.");
		return OS_ERR;
	}
	
	g_ulNdpClientEnable = OS_NO;
	Sleep(1000);
	//thread_close(hClientThread);
	hClientThread = NULL;
	
	closesocket(g_ndpClientSocket);
	g_ndpClientSocket = INVALID_SOCKET;
	
	NDP_Debug(DEBUG_TYPE_FUNC, "NDP_ClientDisable ok.");
}

ULONG NDP_SH_ServerEnable()
{
	ULONG ulRet = OS_OK;

	if (OS_YES == g_ulNdpServerEnable) {
		printf("Info: ndp is already enable.\r\n");
		return OS_ERR;
	}

	g_ulNdpServerEnable = OS_YES;

	ulRet = NDP_ServerEnable();
	if(ulRet != OS_OK) {
		printf("Error: ndp server enable failed.\r\n");
		g_ulNdpServerEnable = OS_NO;
		return OS_ERR;
	}
	
	return OS_OK;
}

ULONG NDP_SH_ClientEnable()
{
	ULONG ulRet = OS_OK;
	
	if (OS_YES == g_ulNdpClientEnable) {
		printf("Info: ndp client is already enable.\r\n");
		return OS_ERR;
	}

	g_ulNdpClientEnable = OS_YES;

	ulRet = NDP_ClientEnable();
	if(ulRet != OS_OK) {
		printf("Error: ndp client enable failed.\r\n");
		g_ulNdpClientEnable = OS_NO;
		return OS_ERR;
	}

	NDP_SendMsgEcho(NDP_GetClientSocket(), ntohl(inet_addr(g_szNdpPeerIP)), g_ulNdpPeerPort);

	return OS_OK;
}


void NDP_SetServerIPPort(char *ip, int port)
{
	strcpy(g_szNdpPeerIP, ip);
	g_ulNdpPeerPort = port;
		
	if (OS_NO == g_ulNdpClientEnable) {
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
	
	/* ȥʹ��ndp server����ʹ�� */
	(void)NDP_ServerDisable();
	
	Sleep(200);

	(void)NDP_SH_ServerEnable();
	
	return;
}


int NDP_TimerThread(void *pEntry)
{
	ULONG ulLoop = 0;

	for (;;) {		
		/* 1s */
		if (0 == ulLoop % 1) {
			NDP_NeighborsAge();
		}
		
		/* 30s*/
		if (0 == ulLoop % 30) {
			if (OS_YES == g_ulNdpClientEnable) {
				NDP_SendMsgEcho(NDP_GetClientSocket(), ntohl(inet_addr(g_szNdpPeerIP)), g_ulNdpPeerPort);
			}

			if (OS_YES == g_ulNdpServerEnable) {
				NDP_SendEchoToAllClient();
			}
		}

		ulLoop++;
		
		Sleep(1000);
	}

	return 0;
}

ULONG NDP_SHOW_Callback(VOID *pRcvMsg)
{
	ULONG ulRet = OS_OK;
	ULONG iLoop = 0;
	ULONG iElemNum = 0;
	ULONG iElemID = 0;
	VOID *pElem = NULL;
	ULONG vtyId = 0;
	ULONG isShowNeighbor = OS_NO;

	vtyId = cmd_get_vty_id(pRcvMsg);
	iElemNum = cmd_get_elem_num(pRcvMsg);

	for (iLoop = 0; iLoop < iElemNum; iLoop++) {
		pElem = cmd_get_elem_by_index(pRcvMsg, iLoop);
		iElemID = cmd_get_elemid(pElem);
		switch (iElemID) {
			case NDP_CMO_SHOW_NEIGHBOR:
				isShowNeighbor = OS_YES;
				break;
			default:
				break;
		}
	}

	if (OS_YES == isShowNeighbor) {
		NDP_ShowAllNeighbors(vtyId);
	}

	return ulRet;
}

ULONG NDP_CFG_Callback(VOID *pRcvMsg)
{
	ULONG ulRet = OS_OK;
	ULONG iLoop = 0;
	ULONG iElemNum = 0;
	ULONG iElemID = 0;
	VOID *pElem = NULL;
	ULONG vtyId = 0;
	ULONG isUndo = 0;
	ULONG isEnable = 0;
	ULONG isClient = 0;
	ULONG isServer = 0;
	ULONG isPort = 0;
	ULONG isIP = 0;
	ULONG isPeerPort = 0;
	ULONG port = 6000;
	char ip[25] = "255.255.255.255";

	vtyId = cmd_get_vty_id(pRcvMsg);
	iElemNum = cmd_get_elem_num(pRcvMsg);

	for (iLoop = 0; iLoop < iElemNum; iLoop++)
	{	
		pElem = cmd_get_elem_by_index(pRcvMsg, iLoop);
		iElemID = cmd_get_elemid(pElem);

		switch(iElemID)
		{
			case NDP_CMO_CFG_UNDO:
				isUndo = 1;
				break;
			case NDP_CMO_CFG_SERVER:
				isServer = 1;
				break;
			case NDP_CMO_CFG_CLIENT:
				isClient = 1;
				break;
			case NDP_CMO_CFG_ENABLE:				
				isEnable = 1;
				break;		
			case NDP_CMO_CFG_SERVER_PORT:
				isPort = 1;
				port = cmd_get_ulong_param(pElem);
				break;					
			case NDP_CMO_CFG_PEER_PORT:
				isPeerPort = 1;
				port = cmd_get_ulong_param(pElem);
				break;	
			case NDP_CMO_CFG_PEER_IP:				
			{
				ULONG ulIp = 0;
				extern VOID cmd_ip_ulong_to_string(ULONG ip, CHAR *buf);
				isIP = 1;
				ulIp = cmd_get_ip_ulong_param(pElem);
				cmd_ip_ulong_to_string(ulIp, ip);
			}				
			default:
				break;
		}
	}

	if (isEnable) {
		if (isServer) {
			if (isUndo) {
				(void)NDP_ServerDisable();
			} else {
				(void)NDP_SH_ServerEnable();
			}
		}
		if (isClient) {
			if (isUndo) {
				(void)NDP_ClientDisable();
			} else {
				(void)NDP_SH_ClientEnable();
			}
		}		
	}

	if (isPort) {
		NDP_SetServerBindPort(port);
	}

	if (isIP) {
		NDP_SetServerIPPort(ip, port);
	}

	return ulRet;

}

ULONG NDP_CMD_Callback(VOID *pRcvMsg)
{
	ULONG iRet = 0;
	ULONG iTBLID = 0;
	
	iTBLID = cmd_get_first_elem_tblid(pRcvMsg);
			
	switch(iTBLID)
	{
		case NDP_CMO_TBL_CFG:
			iRet = NDP_CFG_Callback(pRcvMsg);
			break;
		case NDP_CMO_TBL_SHOW:
			iRet = NDP_SHOW_Callback(pRcvMsg);
			break;
		default:
			break;
	}

	return iRet;
}

ULONG NDP_RegShowCmd()
{
	CMD_VECTOR_S * vec = NULL;

	CMD_VECTOR_NEW(vec);

	// 1
	cmd_regelement_new(CMD_ELEMID_NULL,			CMD_ELEM_TYPE_KEY,	  "display",		    "Display", vec);
	// 2
	cmd_regelement_new(CMD_ELEMID_NULL,			CMD_ELEM_TYPE_KEY,	  "ndp",		   		"Neighbor Discover Protocol", vec);
 	// 3
	cmd_regelement_new(NDP_CMO_SHOW_NEIGHBOR,	CMD_ELEM_TYPE_KEY,	  "neighbor",		    "Show neighbor", vec);
 
	cmd_install_command(MID_NDP, VIEW_GLOBAL,  " 1 2 3 ", vec);

	CMD_VECTOR_FREE(vec);

	return 0;
}
ULONG NDP_RegCfgCmd()
{
	CMD_VECTOR_S * vec = NULL;

	CMD_VECTOR_NEW(vec);

	// 1
	cmd_regelement_new(NDP_CMO_CFG_UNDO,		CMD_ELEM_TYPE_KEY,	  "undo",		    "Undo Operation", vec);
	// 2
	cmd_regelement_new(CMD_ELEMID_NULL,			CMD_ELEM_TYPE_KEY,	  "ndp",		    "Neighbor Discover Protocol", vec);
    // 3
	cmd_regelement_new(NDP_CMO_CFG_SERVER,		CMD_ELEM_TYPE_KEY,	  "server",		    "NDP server role", vec);
    // 4
	cmd_regelement_new(NDP_CMO_CFG_CLIENT,		CMD_ELEM_TYPE_KEY,	  "client",		    "NDP client role", vec);	
    // 5
	cmd_regelement_new(NDP_CMO_CFG_ENABLE,		CMD_ELEM_TYPE_KEY,	  "enable",		    "Enable", vec);
    // 6
	cmd_regelement_new(CMD_ELEMID_NULL,			CMD_ELEM_TYPE_INTEGER,"port", 			"Port", vec);
	// 7
	cmd_regelement_new(NDP_CMO_CFG_SERVER_PORT,	CMD_ELEM_TYPE_INTEGER,"INTEGER<2000-65535>", "NDP server port, default is 6000", vec);
	// 8
	cmd_regelement_new(CMD_ELEMID_NULL,			CMD_ELEM_TYPE_KEY,	  "server-ip", 			"IP", vec);
	// 9
	cmd_regelement_new(NDP_CMO_CFG_PEER_IP,		CMD_ELEM_TYPE_IP,	  CMD_ELMT_IP,		"Connect to the server IP address, default is 127.0.0.1", vec);
	// 10
	cmd_regelement_new(NDP_CMO_CFG_PEER_PORT,	CMD_ELEM_TYPE_INTEGER,"INTEGER<2000-65535>", "NDP server port, default is 6000", vec);

	cmd_install_command(MID_NDP, VIEW_SYSTEM,  " 2 3 5 ", vec);
	cmd_install_command(MID_NDP, VIEW_SYSTEM,  " 1 2 3 5 ", vec);
	cmd_install_command(MID_NDP, VIEW_SYSTEM,  " 2 4 5 ", vec);
	cmd_install_command(MID_NDP, VIEW_SYSTEM,  " 1 2 4 5 ", vec);
	cmd_install_command(MID_NDP, VIEW_SYSTEM,  " 2 3 6 7 ", vec);
	cmd_install_command(MID_NDP, VIEW_SYSTEM,  " 2 8 9 6 10 ", vec);

	CMD_VECTOR_FREE(vec);

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
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"ndp server port %u", g_ulNdpServerPort);
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
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"ndp server-ip %s port %u", g_szNdpPeerIP, g_ulNdpPeerPort);
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

	(VOID)cli_bdn_regist(MID_NDP, VIEW_SYSTEM, BDN_PRIORITY_NORMAL, NDP_BuildRun);
	
	(VOID)cmd_regcallback(MID_NDP, NDP_CMD_Callback);	

	(VOID)NDP_RegCfgCmd();

	(VOID)NDP_RegShowCmd();

	(VOID)DEBUG_PUB_RegModuleDebugs(MID_NDP, "ndp", "Neighbor Discover Protocol");

	/* ������ʱ���߳� */
	(VOID)thread_create(NDP_TimerThread, NULL);
	
	/* ѭ����ȡ��Ϣ���� */
	for(;;)
	{
		/* ��Ȩ */
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
