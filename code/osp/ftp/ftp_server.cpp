#include "ftp_server.h"

#include "kernel.h"

#include "osp/debug/include/debug_center_inc.h"
#include "product/include/pdt_common_inc.h"
#include "osp/event/include/event_pub.h"
#include "osp/command/include/icli.h"
#include "osp/aaa/aaa.h"
#include "osp/util/util.h"

#if (OS_YES == OSP_MODULE_FTPS)

using namespace std;

BOOL g_ulFtpEnable = FALSE;
LONG g_sFtpListenSocket = INVALID_SOCKET;
ULONG g_Ftpsockport = 21;
thread_id_t hFtpsLisent = NULL;

FTP_CMD_HANDLE_S ftp_cmd_resolver[] = {
	{ FTP_CMD_ABOR, 	"ABOR", NULL },
	{ FTP_CMD_CWD, 		"CWD",  NULL },
	{ FTP_CMD_DELE, 	"DELE", NULL },
	{ FTP_CMD_LIST, 	"LIST", FTPS_Handler_LIST },
	{ FTP_CMD_MDTM, 	"MDTM", NULL },
	{ FTP_CMD_MKD, 		"MKD",  NULL },
	{ FTP_CMD_NLST, 	"NLST", NULL },
	{ FTP_CMD_PASS, 	"PASS", FTPS_Handler_PASS },
	{ FTP_CMD_PASV,   	"PASV", NULL },
	{ FTP_CMD_PORT, 	"PORT", FTPS_Handler_PORT },
	{ FTP_CMD_PWD, 		"PWD",  NULL },
	{ FTP_CMD_QUIT, 	"QUIT", FTPS_Handler_QUIT },
	{ FTP_CMD_RETR, 	"RETR", FTPS_Handler_RETR },
	{ FTP_CMD_RMD, 		"RMD",  NULL },
	{ FTP_CMD_RNFR, 	"RNFR", NULL },
	{ FTP_CMD_RNTO, 	"RNTO", NULL },
	{ FTP_CMD_SITE, 	"SITE", NULL },
	{ FTP_CMD_SIZE, 	"SIZE", NULL },
	{ FTP_CMD_STOR, 	"STOR", FTPS_Handler_STOR },
	{ FTP_CMD_TYPE, 	"TYPE", FTPS_Handler_TYPE },
	{ FTP_CMD_USER, 	"USER", FTPS_Handler_USER },
	{ FTP_CMD_NOOP, 	"NOOP", FTPS_Handler_NOOP },
};

LONG FTPS_Connect_DataSocket(FTP_USER_S *pUser)
{
	char buf[1024] = {0};	
	LONG lSockData = INVALID_SOCKET;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);

	/* get client address */
	getpeername(pUser->lSockCtrl, (struct sockaddr*)&client_addr, &len);
	strcpy(buf, inet_ntoa(client_addr.sin_addr));

	/* connect data socket */
	if ((lSockData = FTP_SOCK_Connect(pUser->ulDataPort, buf)) < 0)
	{
		return INVALID_SOCKET;
	}
	
	return lSockData;		
}


ULONG FTPS_Handler_LIST(FTP_USER_S *pUser)
{
	ULONG ulRet = 0;
	CHAR szCMD[FTP_CMD_MAXSIZE] = {0};
	int iSize = 0;	
	char *pData = NULL;
									
	FILE* fd;

	if (NULL == pUser)
	{
		return FTP_ERR;
	}
	
	FTP_debug("FTPS_Handler_List. (lSockCtrl=%u, lSockData=%u, ulDataPort=%u)",
		pUser->lSockCtrl,
		pUser->lSockData,
		pUser->ulDataPort);

	pUser->lSockData = FTPS_Connect_DataSocket(pUser);

	sprintf(szCMD, "150 opening ASCII mode data connection for *.\n");
	(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));

		
	string strinfo;		
	(VOID)util_get_directory_info(pUser->szCurrentDir, strinfo);

	iSize = strinfo.length();

	pData = (char *)malloc(iSize + 1);
	if (NULL == pData)
	{
		closesocket(pUser->lSockData);
		pUser->lSockData = INVALID_SOCKET;

		/* 200 Port command okay. */
		sprintf(szCMD, "200 Port command okay.\n");
		(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));

		return FTP_ERR;
	}

	strcpy(pData, strinfo.c_str());
		
	if (FTP_OK != FTP_Send(pUser->lSockData, pData, iSize)) 
	{
		FTP_debug("FTPS_Handler_List, send file error.");
	}

	/* 226 Transfer complete */
	sprintf(szCMD, "226 Transfer complete.\n");
	(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));

	closesocket(pUser->lSockData);
	pUser->lSockData = INVALID_SOCKET;

	return ulRet;

}

ULONG FTPS_Handler_PORT(FTP_USER_S *pUser)
{
	ULONG ulRet = 0;
	CHAR szCMD[FTP_CMD_MAXSIZE] = {0};
	int ip1 = 0, ip2 = 0, ip3 = 0, ip4 = 0;
	int port1  = 0, port2 = 0;
	
	if (NULL == pUser)
	{
		return FTP_ERR;
	}

	if (0 == strlen(pUser->stCMD.arg))
	{
		return FTP_ERR;
	}
	
	FTP_debug("FTPS_Handler_Port. (argv=%s)", pUser->stCMD.arg);

	sscanf(pUser->stCMD.arg, "%d,%d,%d,%d,%d,%d", &ip1, &ip2, &ip3, &ip4, &port1, &port2);

	/* calc data socket port */
	pUser->ulDataPort = 256 * port1 + port2;

	/* 200 Port command okay. */
	sprintf(szCMD, "200 Port command okay.\n");
	(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));

	return ulRet;
}

ULONG FTPS_Handler_PASS(FTP_USER_S *pUser)
{
	ULONG ulRet = 0;
	ULONG i = 0;
	CHAR szPASS[FTP_PASSWORD_SIZE] = {0};
	CHAR szCMD[FTP_CMD_MAXSIZE] = {0};
	
	if (NULL == pUser)
	{
		return FTP_ERR;
	}

	FTP_debug("FTPS_Handler_PASS. (argv=%s)",
		pUser->stCMD.arg);

	while (pUser->stCMD.arg[i] != '\0' 
		&& pUser->stCMD.arg[i] != '\r'
		&& pUser->stCMD.arg[i] != '\n')
	{
		szPASS[i] = pUser->stCMD.arg[i];
		i++;
	}

	/* save username */
	memset(pUser->szPassword, 0, FTP_PASSWORD_SIZE);
	strcpy(pUser->szPassword, szPASS);

	/* User Authen */
	ulRet = FTPS_UserAuthen(pUser->szUsername, pUser->szPassword);
	if (FTP_OK != ulRet)
	{
		pUser->ulAccess = FALSE;
		
		/* FTP client authen failed. */
		(VOID)FTP_Response(pUser->lSockCtrl, 430);	
		return ulRet;
	}
	else
	{		
		pUser->ulAccess = TRUE;
		
		/* FTP client authen ok, send login code 230. */
		sprintf(szCMD, "230 User logged in.\n");
		(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));	
	}

	return ulRet;
}

ULONG FTPS_Handler_QUIT(FTP_USER_S *pUser)
{
	ULONG ulRet = 0;
	CHAR szCMD[FTP_CMD_MAXSIZE] = {0};
	
	if (NULL == pUser)
	{
		return FTP_ERR;
	}

	FTP_debug("FTPS_Handler_QUIT. (argv=%s)", 
		pUser->stCMD.arg);

	/* send server closing code 221. */
	sprintf(szCMD, "221 Server closing.\n");
	(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));	

	/* close socket */
	closesocket(pUser->lSockCtrl);
	
	return ulRet;
}

ULONG FTPS_Handler_RETR(FTP_USER_S *pUser)
{
	ULONG ulRet = 0;
	CHAR szFilename[FTP_FILEPATH_SIZE] = {0};
	CHAR szCMD[FTP_CMD_MAXSIZE] = {0};	
	CHAR szData[FTP_DATABUF_SIZE] = {0};
	FILE* fd = NULL;
	size_t num_read;	
	
	if (NULL == pUser)
	{
		return FTP_ERR;
	}

	FTP_debug("FTPS_Handler_RETR. (argv=%s)", 
		pUser->stCMD.arg);

	strcpy(szFilename, pUser->stCMD.arg);
	
	/* open file */
	fd = fopen(szFilename, "rb");	
	if (!fd) 
	{	
		sprintf(szCMD, "550 Requested action not taken.\n");
		(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));	

		return FTP_ERR;
	} 
	else
	{	
		/* open data connection */
		pUser->lSockData = FTPS_Connect_DataSocket(pUser);

		/* 200 Port command okay. */
		//sprintf(szCMD, "200 Port command okay.\n");
		///(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));

		sprintf(szCMD, "150 opening ASCII mode data connection for %s.\n", szFilename);
		(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));

		do {
			num_read = fread(szData, 1, FTP_DATABUF_SIZE, fd);
			if (num_read < 0)
			{
				FTP_debug("FTPS_Handler_RETR, error in fread. (num_read=%u)", num_read);
			}

			/* send block */
			if (FTP_OK != FTP_Send(pUser->lSockData, szData, num_read))
			{
				FTP_debug("FTPS_Handler_RETR, error in send. (num_read=%u)", num_read);
				break;
			}
		} while (num_read > 0);													

		/* 226 Transfer complete */
		sprintf(szCMD, "226 Transfer complete.\n");
		(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));

		/* close file handle */
		fclose(fd);

		/* close data socket */
		closesocket(pUser->lSockData);
		pUser->lSockData = INVALID_SOCKET;
		
	}

	return ulRet;
}

ULONG FTPS_Handler_STOR(FTP_USER_S *pUser)
{
	ULONG ulRet = 0;
	CHAR szFilename[FTP_FILEPATH_SIZE] = {0};
	CHAR szCMD[FTP_CMD_MAXSIZE] = {0};	
	CHAR szData[FTP_DATABUF_SIZE] = {0};
	FILE* fd = NULL;
	size_t num_read;	
	
	if (NULL == pUser)
	{
		return FTP_ERR;
	}

	FTP_debug("FTPS_Handler_STOR. (argv=%s)", 
		pUser->stCMD.arg);

	strcpy(szFilename, pUser->stCMD.arg);
	
	/* open file */
	fd = fopen(szFilename, "wb+");	
	if (!fd) 
	{	
		sprintf(szCMD, "550 Requested action not taken.\n");
		(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));	

		return FTP_ERR;
	}
	else
	{	
		/* open data connection */
		pUser->lSockData = FTPS_Connect_DataSocket(pUser);

		sprintf(szCMD, "150 opening ASCII mode data connection for %s.\n", szFilename);
		(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));

		do {
			/* recv block */
			num_read = FTP_Recv(pUser->lSockData, szData, FTP_DATABUF_SIZE);
			if (0 == num_read)
			{
				FTP_debug("FTPS_Handler_STOR, error in recv. (num_read=%u)", num_read);
				break;
			}
			
			num_read = fwrite(szData, 1, num_read, fd);
			if (num_read < 0)
			{
				FTP_debug("FTPS_Handler_STOR, error in fwrite. (num_read=%u)", num_read);
			}
		} while (num_read > 0);													

		/* 226 Transfer complete */
		sprintf(szCMD, "226 Transfer complete.\n");
		(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));

		/* close file handle */
		fclose(fd);

		/* close data socket */
		closesocket(pUser->lSockData);
		pUser->lSockData = INVALID_SOCKET;
		
	}

	return ulRet;
}

ULONG FTPS_Handler_TYPE(FTP_USER_S *pUser)
{
	ULONG ulRet = 0;
	ULONG i = 0;
	CHAR szUname[FTP_USERNAME_SIZE] = {0};
	CHAR szCMD[FTP_CMD_MAXSIZE] = {0};
	
	if (NULL == pUser)
	{
		return FTP_ERR;
	}

	if (0 == strlen(pUser->stCMD.arg))
	{
		/* 501 param invalid */
		(VOID)FTP_Response(pUser->lSockCtrl, 501);
		return FTP_ERR;
	}
	
	FTP_debug("FTPS_Handler_TYPE. (argv=%s)", 
		pUser->stCMD.arg);

	switch (pUser->stCMD.arg[0])
	{
		case 'A':
		case 'a':
			sprintf(szCMD, "200 Type set to A.\n");
			return FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));
		case 'I':
		case 'i':
			sprintf(szCMD, "200 Type set to I.\n");
			return FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));
	}

	(VOID)FTP_Response(pUser->lSockCtrl, 501);

	
	return ulRet;
}

ULONG FTPS_Handler_USER(FTP_USER_S *pUser)
{
	ULONG ulRet = 0;
	ULONG i = 0;
	CHAR szUname[FTP_USERNAME_SIZE] = {0};
	CHAR szCMD[FTP_CMD_MAXSIZE] = {0};
	
	if (NULL == pUser)
	{
		return FTP_ERR;
	}

	FTP_debug("FTPS_Handler_USER. (argv=%s)", 
		pUser->stCMD.arg);

	
	while (pUser->stCMD.arg[i] != '\0' 
		&& pUser->stCMD.arg[i] != '\r'
		&& pUser->stCMD.arg[i] != '\n')
	{
		szUname[i] = pUser->stCMD.arg[i];
		i++;
	}

	/* save username */
	memset(pUser->szUsername, 0, FTP_USERNAME_SIZE);
	strcpy(pUser->szUsername, szUname);
	pUser->ulAccess = FALSE;
	
	/* Send 331 password required */
	if (strlen(szUname) > 0)
	{
		sprintf(szCMD, "331 Password required for %s.\n", szUname);
	}
	else
	{
		sprintf(szCMD, "331 Password required.\n");
	}
	(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));	

	return ulRet;
}

ULONG FTPS_Handler_NOOP(FTP_USER_S *pUser)
{
	ULONG ulRet = 0;
	ULONG i = 0;
	CHAR szCMD[FTP_CMD_MAXSIZE] = {0};
	
	if (NULL == pUser)
	{
		return FTP_ERR;
	}

	FTP_debug("FTPS_Handler_NOOP. (argv=%s)",
		pUser->stCMD.arg);

	(VOID)FTP_Response(pUser->lSockCtrl, 200);
	
	return ulRet;
}

ULONG FTPS_GetSocketPort()
{
	return g_Ftpsockport;
}

LONG FTPS_GetSocket()
{
	return g_sFtpListenSocket;
}

VOID FTPS_SetSocket(LONG lSocket)
{
	g_sFtpListenSocket = lSocket;
	return ;
}

ULONG FTPS_UserAuthen(char*user, char*pass)
{
	FTP_debug("FTPS_UserAuthen. (user=%s, pass=%s)", user, pass);

	return AAA_UserAuth(user, pass, AAA_SERVICE_TYPE_FTP);
}

FTP_CMD_HANDLE_S * FTPS_GetHandler(CHAR *szCMD)
{
	ULONG ulLoop = 0;
	ULONG ulSize = sizeof(ftp_cmd_resolver)/sizeof(FTP_CMD_HANDLE_S);
	
	for (ulLoop = 0; ulLoop < ulSize; ulLoop++)
	{
		if (0 == strcmp(ftp_cmd_resolver[ulLoop].szCmdStr, szCMD))
		{
			FTP_debug("FTPS_GetHandler. (ulCmdCode=%u, szCmdStr=%s)",
				ftp_cmd_resolver[ulLoop].ulCmdCode,
				ftp_cmd_resolver[ulLoop].szCmdStr);
			
			return &ftp_cmd_resolver[ulLoop];
		}
	}

	return NULL;
}

VOID FTPS_Run(LONG lSockCtrl)
{
	ULONG ulRet = 0;
	CHAR cmd[FTP_CMD_CODE_SIZE];
	CHAR arg[FTP_CMD_ARGV_SIZE];
	CHAR szCMD[FTP_CMD_MAXSIZE] = {0}; 
	FTP_CMD_HANDLE_S *pstFtphandle = NULL;
	FTP_USER_S *pstUser = NULL;

	pstUser = (FTP_USER_S *)malloc(sizeof(FTP_USER_S));
	if (NULL == pstUser)
	{
		return ;
	}
	memset(pstUser, 0, sizeof(FTP_USER_S));
	
	pstUser->lSockCtrl = lSockCtrl;
	pstUser->lSockData = INVALID_SOCKET;
	strcpy(pstUser->szCurrentDir, ".\/");
	strcpy(pstUser->szRootDir, ".\/");
	
	/* FTP server is ready, send welcome code 220 */
	sprintf(szCMD, "220 FTP service ready.\n");
	ulRet = FTP_Send(lSockCtrl, szCMD, strlen(szCMD));
	if (FTP_OK != ulRet)
	{
		free(pstUser);
		return ;
	}

	while (TRUE == g_ulFtpEnable)
	{
		/* Wait for command */
		ulRet = FTP_RecvCommand(lSockCtrl, cmd, arg);
		if (FTP_OK != ulRet)
		{
			FTP_debug("FTP RecvCommand error.");
			break;
		}

		/* copy new command */
		strcpy(pstUser->stCMD.cmd, cmd);
		strcpy(pstUser->stCMD.arg, arg);
		
		/* get command handler */
		pstFtphandle = FTPS_GetHandler(cmd);
		if (NULL == pstFtphandle)
		{
			FTP_debug("FTP getHandler failed. (cmd=%s)", cmd);
			continue;
		}

		/* reauthen for local-user password changed */
		if (TRUE == pstUser->ulAccess
			&& FTP_OK != FTPS_UserAuthen(pstUser->szUsername, pstUser->szPassword))
		{
			FTP_debug("FTP UserAuthen failed.");
			
			sprintf(szCMD, "421 Authentication failed.\n");
			(VOID)FTP_Send(lSockCtrl, szCMD, strlen(szCMD));	
			
			break;
		}

		/* handle command callback */
		if (NULL != pstFtphandle->pKeyCallbackfunc)
		{
			ulRet = pstFtphandle->pKeyCallbackfunc(pstUser);
			if (FTP_OK != ulRet)
			{
				FTP_debug("FTP Handler failed. (ulRet=%u, ulCmdCode=%u)",
							ulRet, pstFtphandle->ulCmdCode);
			}			
		}
		else
		{
			FTP_debug("FTP Handler pKeyCallbackfunc is NULL. (ulCmdCode=%u, szCmdStr=%s)",
						pstFtphandle->ulCmdCode, pstFtphandle->szCmdStr);

			
			sprintf(szCMD, "202 Command not support.\n");
			(VOID)FTP_Send(lSockCtrl, szCMD, strlen(szCMD));		
		}

		
	}

	closesocket(pstUser->lSockCtrl);
	free(pstUser);
	
	return;
}

int FTPS_UserThread(void *pEntry)
{
	LONG lSockCtrl = *(LONG*)pEntry;

	/* run a FTP Instance */
	FTPS_Run(lSockCtrl);	

	return FTP_OK;
}

int FTPS_ListenThread(void *pEntry)
{
	LONG lSockCtrl = 0;
	
	while(TRUE == g_ulFtpEnable)
	{	
		/* access a new user socket */
		if ((lSockCtrl = FTP_SOCK_Accept(FTPS_GetSocket())) < 0 )
		{
			Sleep(1);
			continue;
		}

		/* create a user thread */
		(VOID)thread_create(FTPS_UserThread, (VOID*)&lSockCtrl);
		
		Sleep(1);
	}

	return FTP_OK;
}

ULONG FTPS_Enable()
{
	ULONG ulSocket = 0;

	if (TRUE == g_ulFtpEnable)
	{
		return FTP_OK;
	}

	/* create ftp socket */
	ulSocket = FTP_SOCK_Create(FTPS_GetSocketPort());
	if (INVALID_SOCKET == ulSocket) 
	{
		FTP_debug("FTPS_Main, socket_create failed.");
		return FTP_ERR;
	}

	/* save global listen socket */
	FTPS_SetSocket(ulSocket);

	g_ulFtpEnable = TRUE;
	
	/* create FTP Listen Thread */
	hFtpsLisent = thread_create(FTPS_ListenThread, NULL);

	return FTP_OK;
}

ULONG FTPS_Disable()
{
	if (FALSE == g_ulFtpEnable)
	{
		return FTP_OK;
	}

	(VOID)thread_close(hFtpsLisent);
	hFtpsLisent = NULL;

	g_ulFtpEnable = FALSE;

	closesocket(FTPS_GetSocket());	
	FTPS_SetSocket(INVALID_SOCKET);
	
	return FTP_OK;
}

ULONG FTPS_Main()
{	
	//(VOID)FTPS_Enable();
	
	return FTP_OK;
}

#endif

