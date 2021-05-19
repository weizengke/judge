
#include "ftp_common.h"

#include "ic/include/debug_center_inc.h"

#if (OS_YES == OSP_MODULE_FTPS)

void FTP_GetLocalIpBySocket(int sock, int *ip)
{
    socklen_t addr_size = sizeof(struct sockaddr_in);
    struct sockaddr_in addr;
    getsockname(sock, (struct sockaddr *)&addr, &addr_size);
    int host,i;

    host = (addr.sin_addr.s_addr);
    for(i=0; i<4; i++){
        ip[i] = (host>>i*8)&0xff;
    }
}

void FTP_GetPeerIpBySocket(int sock, int *ip)
{
    socklen_t addr_size = sizeof(struct sockaddr_in);
    struct sockaddr_in addr;
    getpeername(sock, (struct sockaddr *)&addr, &addr_size);
    int host,i;

    host = (addr.sin_addr.s_addr);
    for(i=0; i<4; i++){
        ip[i] = (host>>i*8)&0xff;
    }
}

socket_t FTP_SOCK_Create(ULONG Ip, ULONG ulPort)
{
	socket_t sock;	

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {
		FTP_debug("FTP_SOCK_Create, socket() error.");
		return INVALID_SOCKET; 
	}

	int one = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&one, sizeof(one)) < 0) {
		closesocket(sock);
		return INVALID_SOCKET;
	}

	struct sockaddr_in sock_addr;
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(ulPort);
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sock, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) < 0) {
		closesocket(sock);
		FTP_debug("FTP_SOCK_Create, bind(%d) error.", ulPort); 
		return INVALID_SOCKET; 
	}
   
	if (listen(sock, 20) < 0) {
		closesocket(sock);
		FTP_debug("FTP_SOCK_Create, listen() error."); 
		return INVALID_SOCKET;
	}          

	return sock;
}

socket_t FTP_SOCK_Accept(socket_t sockListen)
{
	socket_t sock;
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);

	if (sockListen == INVALID_SOCKET) {
		return INVALID_SOCKET;
	}

	sock = accept(sockListen, (struct sockaddr *) &addr, &len);
	if (sock < 0)
	{
		FTP_debug("FTP_SOCK_Accept, accept() error.");
		return INVALID_SOCKET; 
	}
	return sock;
}

socket_t FTP_SOCK_Connect(ULONG ulPort, CHAR* szIP)
{
	socket_t sock;  					
	struct sockaddr_in addr;

	FTP_debug("FTP_SOCK_Connect, %s:%d", szIP, ulPort);

	// create socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) { 
    	FTP_debug("FTP_SOCK_Connect, socket() error.");
    	return INVALID_SOCKET;
    }

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(ulPort);
	addr.sin_addr.s_addr = inet_addr(szIP);

	if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0 ) {
        FTP_debug("FTP_SOCK_Connect, connect() error.");
		closesocket(sock);
		return INVALID_SOCKET;
    }    
	
	return sock;
}


ULONG FTP_Recv(socket_t sock, CHAR *szMessage, ULONG ulSize)
{
	int iRecvByte = 0;
	
	memset(szMessage, 0, ulSize);
	
	iRecvByte = recv(sock, szMessage, ulSize, 0);

	//FTP_debug("FTP_Recv: iRecvByte=%d, szMessage=%s", iRecvByte, szMessage);
	
	if (iRecvByte <= 0) {		
		return 0;
	}
	
	return iRecvByte;
}

ULONG FTP_Send(socket_t sock, CHAR *szMessage, ULONG ulSize)
{
	//FTP_debug("\r\n FTP_Send: szMessage=%s, ulSize=%d", szMessage, ulSize);
	
	if (send(sock, (char*)szMessage, ulSize, 0) < 0 ) {
		FTP_debug("FTP_Send failed: ulSize=%d, szMessage=%s", ulSize, szMessage);
		return FTP_ERR;
	}
	
	return FTP_OK;
}

ULONG FTP_RecvCommand(socket_t sock, CHAR* szCmd, CHAR* arg)
{	
	CHAR buffer[FTP_CMD_MAXSIZE];
	
	memset(buffer, 0, FTP_CMD_MAXSIZE);
	memset(szCmd, 0, FTP_CMD_CODE_SIZE);
	memset(arg, 0, FTP_CMD_ARGV_SIZE);

	// Wait to recieve command
	if (0 == FTP_Recv(sock, buffer, sizeof(buffer)))
	{		
		return FTP_ERR;
	}

	sscanf(buffer,"%5s %512s", szCmd, arg);

	FTP_debug("FTP_RecvCommand. (cmd:%s, arg:%s)", szCmd, arg);

	return FTP_OK;
}


ULONG FTP_Response(socket_t sock, ULONG ulCode)
{
	CHAR szMsg[FTP_CMD_MAXSIZE] = {0};

	sprintf(szMsg, "%u\n", ulCode);

	//FTP_debug("FTP_Response: ulCode=%u", ulCode);
	
	if (send(sock, szMsg, strlen(szMsg), 0) < 0 )
	{
		FTP_debug("FTP_Response failed: ulCode=%u", ulCode);
		return FTP_ERR;
	}
	
	return FTP_OK;
}

#endif
