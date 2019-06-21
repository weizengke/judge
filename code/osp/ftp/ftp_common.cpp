
#include "ftp_common.h"

#include "osp/debug/include/debug_center_inc.h"

#if (OS_YES == OSP_MODULE_FTPS)

#if 0
VOID FTP_debug(const CHAR *format, ...)
{
    CHAR buf[FTP_DEBUG_BUFSIZE] = {0};
	CHAR buf_t[FTP_DEBUG_BUFSIZE] = {0};
	
	va_list args;
	va_start(args, format);
	vsnprintf(buf_t, FTP_DEBUG_BUFSIZE, format, args);
	snprintf(buf, FTP_DEBUG_BUFSIZE, "%s%s", buf,buf_t);
	va_end(args);
	
	printf("\r\n%s", buf);

	return;
}
#endif

LONG FTP_SOCK_Create(ULONG ulPort)
{
	LONG lSockfd;
	int yes = 1;
	struct sockaddr_in sock_addr;

	if ((lSockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		FTP_debug("FTP_SOCK_Create, socket() error.");
		return INVALID_SOCKET; 
	}

	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(ulPort);
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);		

	if (setsockopt(lSockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(int)) == -1)
	{
		close(lSockfd);
		FTP_debug("FTP_SOCK_Create, setsockopt() error.");
		return INVALID_SOCKET; 
	}

	if (bind(lSockfd, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) < 0)
	{
		close(lSockfd);
		FTP_debug("FTP_SOCK_Create, bind() error."); 
		return INVALID_SOCKET; 
	}
   
	if (listen(lSockfd, 10) < 0)
	{
		close(lSockfd);
		FTP_debug("FTP_SOCK_Create, listen() error."); 
		return INVALID_SOCKET;
	}              
	return lSockfd;
}

LONG FTP_SOCK_Accept(LONG lSockListen)
{
	LONG lSockfd;
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);

	lSockfd = accept(lSockListen, (struct sockaddr *) &addr, &len);
	if (lSockfd < 0)
	{
		FTP_debug("FTP_SOCK_Accept, accept() error.");
		return INVALID_SOCKET; 
	}
	return lSockfd;
}

LONG FTP_SOCK_Connect(ULONG ulPort, CHAR* szIP)
{
	LONG lSockfd;  					
	struct sockaddr_in addr;

	// create socket
	if ((lSockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{ 
    	FTP_debug("FTP_SOCK_Connect, socket() error.");
    	return INVALID_SOCKET;
    }

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(ulPort);
	addr.sin_addr.s_addr = inet_addr(szIP);

	if(connect(lSockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0 )
	{
        FTP_debug("FTP_SOCK_Connect, connect() error.");
		return INVALID_SOCKET;
    }    
	
	return lSockfd;
}


ULONG FTP_Recv(LONG lSock, CHAR *szMessage, ULONG ulSize)
{
	int iRecvByte = 0;
	
	memset(szMessage, 0, ulSize);
	
	iRecvByte = recv(lSock, szMessage, ulSize, 0);

	//FTP_debug("FTP_Recv: iRecvByte=%d, szMessage=%s", iRecvByte, szMessage);
	
	if (iRecvByte <= 0)
	{		
		return 0;
	}
	
	return iRecvByte;
}

ULONG FTP_Send(LONG lSock, CHAR *szMessage, ULONG ulSize)
{
	//FTP_debug("\r\n FTP_Send: szMessage=%s, ulSize=%d", szMessage, ulSize);
	
	if (send(lSock, (char*)szMessage, ulSize, 0) < 0 )
	{
		FTP_debug("FTP_Send failed: ulSize=%d, szMessage=%s", ulSize, szMessage);
		return FTP_ERR;
	}
	
	return FTP_OK;
}

ULONG FTP_RecvCommand(LONG lSock, CHAR* szCmd, CHAR* arg)
{	
	CHAR buffer[FTP_CMD_MAXSIZE];
	
	memset(buffer, 0, FTP_CMD_MAXSIZE);
	memset(szCmd, 0, FTP_CMD_CODE_SIZE);
	memset(arg, 0, FTP_CMD_ARGV_SIZE);

	// Wait to recieve command
	if (0 == FTP_Recv(lSock, buffer, sizeof(buffer)))
	{		
		return FTP_ERR;
	}

	sscanf(buffer,"%5s %512s", szCmd, arg);

	FTP_debug("FTP_RecvCommand. (cmd:%s, arg:%s)", szCmd, arg);

	return FTP_OK;
}


ULONG FTP_Response(LONG lSock, ULONG ulCode)
{
	CHAR szMsg[FTP_CMD_MAXSIZE] = {0};

	sprintf(szMsg, "%u\n", ulCode);

	//FTP_debug("FTP_Response: ulCode=%u", ulCode);
	
	if (send(lSock, szMsg, strlen(szMsg), 0) < 0 )
	{
		FTP_debug("FTP_Response failed: ulCode=%u", ulCode);
		return FTP_ERR;
	}
	
	return FTP_OK;
}

#endif
