#include "ftp_server.h"

#include "../include/icli.h"
#include "kernel.h"

#include "ic/include/debug_center_inc.h"
#include "include/pdt_common_inc.h"
#include "event/include/event_pub.h"
#include "aaa/aaa.h"
#include "util/util.h"

#if (OS_YES == OSP_MODULE_FTPS)

using namespace std;

BOOL g_ulFtpEnable = FALSE;
ULONG g_Ftpsockport = 21;
ULONG g_FtpsockSource = 0;
socket_t g_sFtpListenSocket;

FTP_CMD_HANDLE_S ftp_cmd_resolver[] = {
	{ FTP_CMD_ABOR, 	"ABOR", NULL },
	{ FTP_CMD_CWD, 		"CWD",  FTPS_Handler_CWD },
	{ FTP_CMD_DELE, 	"DELE", NULL },
	{ FTP_CMD_LIST, 	"LIST", FTPS_Handler_LIST },
	{ FTP_CMD_MDTM, 	"MDTM", NULL },
	{ FTP_CMD_MKD, 		"MKD",  NULL },
	{ FTP_CMD_NLST, 	"NLST", NULL },
	{ FTP_CMD_PASS, 	"PASS", FTPS_Handler_PASS },
	{ FTP_CMD_PASV,   	"PASV", FTPS_Handler_PASV },
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
	{ FTP_CMD_SIZE, 	"SYST", FTPS_Handler_SYST },
};

VOID FTPS_InitUserDirectory(FTP_USER_S *pUser)
{
	char *dir =	AAA_GetUserFtpDirectory(pUser->szUsername);
	if (dir == NULL || strlen(dir) == 0) {
		get_current_directory(sizeof(pUser->szRootDir), pUser->szRootDir);
		get_current_directory(sizeof(pUser->szCurrentDir), pUser->szCurrentDir);
		//strcpy(pUser->szCurrentDir, ".\/");
	} else {
		strcpy(pUser->szRootDir, dir);
		strcpy(pUser->szCurrentDir, dir);
	}

	FTP_debug("FTPS_InitUserDirectory. (szUsername=%s, szRootDir=%s, dir=%s)", 
			  pUser->szUsername, pUser->szRootDir, dir);
}

socket_t FTPS_Connect_DataSocket(FTP_USER_S *pUser)
{
	char buf[1024] = {0};
	socket_t lSockData = INVALID_SOCKET;
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

ULONG FTPS_CWD_IsPathGoodDot(CHAR *path)
{
	ULONG i = 0;
	ULONG dotNum = 0;
	ULONG len = strlen(path);
	
	for (i = 0; i < len; i++) {
		if (path[i] == '.') {
			dotNum++;
		}
	}

	if (dotNum == 0) {
		return FTP_ERR;
	}

	if (dotNum == 2) {
		if (path[0] == '.' && path[1] == '.') {
			if (strlen(path) == 2) {
				/*  only support cmd "cd .." if with dot */
				return FTP_OK;
			}
		}
	}

	return FTP_CWD_BAD_DOT;
}

ULONG FTPS_CWD_CalcCurrentDir(FTP_USER_S *pUser)
{
	ULONG ret;

	if (strlen(pUser->stCMD.arg) == 1) {
		/* cmd '.': current dir*/
		if (pUser->stCMD.arg[0] == '.') {
			return FTP_OK;
		}

		if (pUser->stCMD.arg[0] == '\/') {
			/* cmd '/': current-dir back to root-dir*/
			strcpy(pUser->szCurrentDir, pUser->szRootDir);
			return FTP_OK;
		}
	}

	ret = FTPS_CWD_IsPathGoodDot(pUser->stCMD.arg);
	if (ret == FTP_OK) {
		/* back to parent dir */
		if (0 != strnicmp(pUser->szCurrentDir, pUser->szRootDir, strlen(pUser->szRootDir))
		|| strlen(pUser->szCurrentDir) <= strlen(pUser->szRootDir)) {
			FTP_debug("FTPS_Handler_CWD. CalcCurrentDir, NO_PERMISSION 1");
			return FTP_CWD_NO_PERMISSION;
		} else {
			for (int i = strlen(pUser->szCurrentDir) - 1; i >= strlen(pUser->szRootDir); i--) {				
				if (pUser->szCurrentDir[i] == '\/') {
					pUser->szCurrentDir[i] = '\0';
					return FTP_OK;
				}
				pUser->szCurrentDir[i] = '\0';
			}

			FTP_debug("FTPS_Handler_CWD. CalcCurrentDir, NO_PERMISSION 2");
			return FTP_CWD_NO_PERMISSION;
		}
	} else {
		if (ret == FTP_CWD_BAD_DOT) {
			return FTP_CWD_BAD_DOT;
		}
	}
	
	if (strlen(pUser->szCurrentDir) + strlen(pUser->stCMD.arg) >= sizeof(pUser->szCurrentDir)) {
		return FTP_CWD_INVALID_PATH;
	}

	if (strlen(pUser->szCurrentDir) > 0 
		&& pUser->szCurrentDir[strlen(pUser->szCurrentDir) - 1] != '\/') {
			strcat(pUser->szCurrentDir, "\/");
	}

	strcat(pUser->szCurrentDir, pUser->stCMD.arg);
	return FTP_OK;
}

ULONG FTPS_Handler_CWD(FTP_USER_S *pUser)
{
	ULONG ulRet = 0;
	CHAR szCMD[FTP_CMD_MAXSIZE] = {0};

	if (NULL == pUser) {
		return FTP_ERR;
	}

	FTP_debug("FTPS_Handler_CWD. (lSockCtrl=%u, szCurrentDir=%s, szRootDir=%s)",
		pUser->lSockCtrl,
		pUser->szCurrentDir,
		pUser->szRootDir);

	ulRet = FTPS_CWD_CalcCurrentDir(pUser);
	if (ulRet != FTP_OK) {
		if (ulRet == FTP_CWD_NO_PERMISSION) {
			sprintf(szCMD, "501 CWD failed. No permission\n");
		} else if (ulRet == FTP_CWD_BAD_DOT) {
			sprintf(szCMD, "501 CWD failed. Cannot accept relative path using dot notation\n");
		} else {
			sprintf(szCMD, "501 CWD failed.\n");
		}

		FTP_debug("FTPS_Handler_CWD. (szCMD=%s)", szCMD);
		(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));
		return FTP_OK;
	}

	sprintf(szCMD, "250 CWD command successful. \"%s\" is current directory.\n", pUser->szCurrentDir);
	(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));

	return ulRet;

}
ULONG FTPS_Handler_LIST(FTP_USER_S *pUser)
{
	ULONG ulRet = 0;
	CHAR szCMD[FTP_CMD_MAXSIZE] = {0};
	int iSize = 0;
	char *pData = NULL;

	if (NULL == pUser) {
		return FTP_ERR;
	}

	/* port mode */
	if (pUser->lSockPasv == INVALID_SOCKET) {
		/* open data connection */
		pUser->lSockData = FTPS_Connect_DataSocket(pUser);
		sprintf(szCMD, "150 opening ASCII mode data connection for *.\n");
		(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));
	} else {
		/* pasv mode */
		pUser->lSockData = FTP_SOCK_Accept(pUser->lSockPasv);
		if (INVALID_SOCKET == pUser->lSockData) {
			closesocket(pUser->lSockPasv);
			pUser->lSockPasv = INVALID_SOCKET;
		}
		sprintf(szCMD, "150 Opening data connection for *.\n");
		(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));
	}
	
	FTP_debug("FTPS_Handler_List. (lSockCtrl=%u, lSockData=%u, ulDataPort=%u, szCurrentDir=%s)",
		pUser->lSockCtrl,
		pUser->lSockData,
		pUser->ulDataPort,
		pUser->szCurrentDir);

	string string_dir = pUser->szCurrentDir;
	string strinfo;
	(VOID)util_get_directory_info(string_dir, strinfo);
	iSize = strinfo.length();

	pData = (char *)malloc(iSize + 512);
	if (NULL == pData) {
		closesocket(pUser->lSockData);
		pUser->lSockData = INVALID_SOCKET;

		/* 200 Port command okay. */
		sprintf(szCMD, "200 Port command okay.\n");
		(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));

		return FTP_ERR;
	}
	memset(pData, 0, iSize + 512);

	if (iSize == 0) {
		sprintf(pData, "%s not found any file", pUser->szCurrentDir);
		iSize = strlen(pData);
	} else {
		strcpy(pData, strinfo.c_str());
	}
	FTP_debug("FTPS_Handler_List: size=%d. \n%s.", iSize, pData);

	if (FTP_OK != FTP_Send(pUser->lSockData, pData, iSize)) {
		FTP_debug("FTPS_Handler_List, send file error.");
	}
	
	/* 226 Transfer complete */
	sprintf(szCMD, "226 Transfer complete.\n");
	(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));

	closesocket(pUser->lSockData);
	pUser->lSockData = INVALID_SOCKET;

	free(pData);

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
			
		/* update current & root dir*/
		FTPS_InitUserDirectory(pUser);

		/* FTP client authen ok, send login code 230. */
		sprintf(szCMD, "230 User logged in.\n");
		(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));
	}

	return ulRet;
}

ULONG FTPS_Handler_PASV(FTP_USER_S *pUser)
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

	typedef struct Port
	{
		int p1;
		int p2;
	} Port;

	int ip[4];
	int ipPeer[4];
	char buff[255];
	char *response = "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)\n";
	Port port = {0};
	srand(time(NULL));
    port.p1 = 128 + (rand() % 64);
    port.p2 = rand() % 0xff;

	if (g_FtpsockSource == 0) {
		FTP_GetLocalIpBySocket(pUser->lSockCtrl, ip);
	} else {
		for(i=0; i<4; i++){
        	ip[i] = (g_FtpsockSource>>i*8)&0xff;
		}
    }
	
	FTP_GetPeerIpBySocket(pUser->lSockCtrl, ipPeer);

	FTP_debug("FTPS_Handler_PASS. ip:%d.%d.%d.%d, ip_peer:%d.%d.%d.%d, port: %d.",
			  ip[0],ip[1],ip[2],ip[3], ipPeer[0],ipPeer[1],ipPeer[2],ipPeer[3],
			  256*port.p1+port.p2);

	pUser->lSockPasv = FTP_SOCK_Create(g_FtpsockSource, (256*port.p1) + port.p2);

	sprintf(szCMD, response,ip[0],ip[1],ip[2],ip[3],port.p1,port.p2);
	(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));

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
	CHAR szFilePath[FTP_FILEPATH_SIZE] = {0};
	CHAR szFilename[FTP_FILEPATH_SIZE] = {0};
	CHAR szCMD[FTP_CMD_MAXSIZE] = {0};
	CHAR szData[FTP_DATABUF_SIZE] = {0};
	FILE* fd = NULL;
	size_t num_read;

	if (NULL == pUser) {
		return FTP_ERR;
	}

	strcpy(szFilename, pUser->stCMD.arg);
	sprintf(szFilePath, "%s\/%s", pUser->szCurrentDir, szFilename);

	FTP_debug("FTPS_Handler_RETR. (argv=%s, szFilePath=%s, szFilename=%s)",
		pUser->stCMD.arg, szFilePath, szFilename);

	/* open file */
	fd = fopen(szFilePath, "rb");
	if (!fd) {
		sprintf(szCMD, "550 Requested action not taken.\n");
		(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));
		return FTP_ERR;
	} else {
		/* open data connection */
		pUser->lSockData = FTPS_Connect_DataSocket(pUser);

		/* 200 Port command okay. */
		//sprintf(szCMD, "200 Port command okay.\n");
		///(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));

		sprintf(szCMD, "150 opening ASCII mode data connection for %s.\n", szFilename);
		(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));

		do {
			num_read = fread(szData, 1, FTP_DATABUF_SIZE, fd);
			if (num_read < 0) {
				FTP_debug("FTPS_Handler_RETR, error in fread. (num_read=%u)", num_read);
			}

			/* send block */
			if (FTP_OK != FTP_Send(pUser->lSockData, szData, num_read)) {
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
	CHAR szFilePath[FTP_FILEPATH_SIZE] = {0};
	CHAR szFilename[FTP_FILEPATH_SIZE] = {0};
	CHAR szCMD[FTP_CMD_MAXSIZE] = {0};
	CHAR szData[FTP_DATABUF_SIZE] = {0};
	FILE* fd = NULL;
	size_t num_read;

	if (NULL == pUser)
	{
		return FTP_ERR;
	}

	strcpy(szFilename, pUser->stCMD.arg);
	sprintf(szFilePath, "%s\/%s", pUser->szCurrentDir, szFilename);

	FTP_debug("FTPS_Handler_STOR. (argv=%s, szFilePath=%s, szFilename=%s)",
		pUser->stCMD.arg, szFilename, szFilePath);

	/* open file */
	fd = fopen(szFilePath, "wb+");
	if (!fd)
	{
		sprintf(szCMD, "550 Requested action not taken.\n");
		(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));

		return FTP_ERR;
	}
	else
	{
		/* port mode */
		if (pUser->lSockPasv == INVALID_SOCKET) {
			/* open data connection */
			pUser->lSockData = FTPS_Connect_DataSocket(pUser);

			sprintf(szCMD, "150 opening ASCII mode data connection for %s.\n", szFilename);
			(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));
		} else {
			/* pasv mode */
            pUser->lSockData = FTP_SOCK_Accept(pUser->lSockPasv);
            if (INVALID_SOCKET == pUser->lSockData) {
				closesocket(pUser->lSockPasv);
				pUser->lSockPasv = INVALID_SOCKET;
			}
            sprintf(szCMD, "150 Opening data connection for %s.\n", szFilename);
			(VOID)FTP_Send(pUser->lSockCtrl, szCMD, strlen(szCMD));
		}
		
		do {
			/* recv block */
			num_read = FTP_Recv(pUser->lSockData, szData, FTP_DATABUF_SIZE);
			if (0 == num_read) {
				break;
			}

			num_read = fwrite(szData, 1, num_read, fd);
			if (num_read < 0) {
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
		closesocket(pUser->lSockPasv);
		pUser->lSockPasv = INVALID_SOCKET;
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

	if (NULL == pUser) {
		return FTP_ERR;
	}

	FTP_debug("FTPS_Handler_USER. (argv=%s)",
		pUser->stCMD.arg);


	while (pUser->stCMD.arg[i] != '\0'
		&& pUser->stCMD.arg[i] != '\r'
		&& pUser->stCMD.arg[i] != '\n') {
		szUname[i] = pUser->stCMD.arg[i];
		i++;
	}

	/* save username */
	memset(pUser->szUsername, 0, FTP_USERNAME_SIZE);
	strcpy(pUser->szUsername, szUname);
	pUser->ulAccess = FALSE;

	/* Send 331 password required */
	if (strlen(szUname) > 0) {
		sprintf(szCMD, "331 Password required for %s.\n", szUname);
	} else {
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

	if (NULL == pUser) {
		return FTP_ERR;
	}

	FTP_debug("FTPS_Handler_NOOP. (argv=%s)",
		pUser->stCMD.arg);

	(VOID)FTP_Response(pUser->lSockCtrl, 200);

	return ulRet;
}

ULONG FTPS_Handler_SYST(FTP_USER_S *pUser)
{
	ULONG ulRet = 0;
	ULONG i = 0;
	CHAR szCMD[FTP_CMD_MAXSIZE] = {0};

	if (NULL == pUser) {
		return FTP_ERR;
	}

	FTP_debug("FTPS_Handler_SYST. (argv=%s)",
		pUser->stCMD.arg);

	(VOID)FTP_Response(pUser->lSockCtrl, 200);

	return ulRet;
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

	for (ulLoop = 0; ulLoop < ulSize; ulLoop++) {
		if (0 == strcmp(ftp_cmd_resolver[ulLoop].szCmdStr, szCMD)) {
			FTP_debug("FTPS_GetHandler. (ulCmdCode=%u, szCmdStr=%s)",
				ftp_cmd_resolver[ulLoop].ulCmdCode,
				ftp_cmd_resolver[ulLoop].szCmdStr);
			return &ftp_cmd_resolver[ulLoop];
		}
	}

	return NULL;
}

VOID FTPS_Run(socket_t lSockCtrl)
{
	ULONG ulRet = 0;
	CHAR cmd[FTP_CMD_CODE_SIZE];
	CHAR arg[FTP_CMD_ARGV_SIZE];
	CHAR szCMD[FTP_CMD_MAXSIZE] = {0};
	FTP_CMD_HANDLE_S *pstFtphandle = NULL;
	FTP_USER_S *pstUser = NULL;

	pstUser = (FTP_USER_S *)malloc(sizeof(FTP_USER_S));
	if (NULL == pstUser) {
		return ;
	}
	memset(pstUser, 0, sizeof(FTP_USER_S));

    pstUser->ulAccess = FALSE;
	pstUser->lSockCtrl = lSockCtrl;
	pstUser->lSockData = INVALID_SOCKET;
	pstUser->lSockPasv = INVALID_SOCKET;

	/* FTP server is ready, send welcome code 220 */
	sprintf(szCMD, "220 FTP service ready.\n");
	ulRet = FTP_Send(lSockCtrl, szCMD, strlen(szCMD));
	if (FTP_OK != ulRet) {
		free(pstUser);
		return ;
	}

	while (TRUE == g_ulFtpEnable) {
		/* Wait for command */
		ulRet = FTP_RecvCommand(lSockCtrl, cmd, arg);
		if (FTP_OK != ulRet) {
			FTP_debug("FTP RecvCommand error.");
			break;
		}

		/* copy new command */
		strcpy(pstUser->stCMD.cmd, cmd);
		strcpy(pstUser->stCMD.arg, arg);

		/* get command handler */
		pstFtphandle = FTPS_GetHandler(cmd);
		if (NULL == pstFtphandle) {
			FTP_debug("FTP getHandler failed. (cmd=%s)", cmd);			
			sprintf(szCMD, "202 Command not support.\n");
			(VOID)FTP_Send(lSockCtrl, szCMD, strlen(szCMD));			
			continue;
		}

         if (FTP_CMD_PASS == pstFtphandle->ulCmdCode
            || FTP_CMD_USER == pstFtphandle->ulCmdCode) {
         } else {
        		/* reauthen for local-user password changed */
        		if (FTP_OK != FTPS_UserAuthen(pstUser->szUsername, pstUser->szPassword)) {
        			FTP_debug("FTP UserAuthen failed.");
        			sprintf(szCMD, "421 Authentication failed.\n");
        			(VOID)FTP_Send(lSockCtrl, szCMD, strlen(szCMD));
        			break;
        		}
		}

		/* handle command callback */
		if (NULL != pstFtphandle->pKeyCallbackfunc) {
			ulRet = pstFtphandle->pKeyCallbackfunc(pstUser);
			if (FTP_OK != ulRet) {
				FTP_debug("FTP Handler failed. (ulRet=%u, ulCmdCode=%u)",
							ulRet, pstFtphandle->ulCmdCode);
			}
		} else {
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
	socket_t lSockCtrl = *(socket_t*)pEntry;

	/* run a FTP Instance */
	FTPS_Run(lSockCtrl);

	return FTP_OK;
}

int FTPS_ListenThread(void *pEntry)
{
	sockaddr_in remoteAddr;
	socket_t sClient;
	socklen_t nAddrLen = sizeof(remoteAddr);

	while(TRUE == g_ulFtpEnable) {
		/* access a new user socket */
		sClient = accept(g_sFtpListenSocket, (SOCKADDR *)&remoteAddr, &nAddrLen);
		if (sClient == INVALID_SOCKET) {
			continue;
		}

		/* create a user thread */
		thread_create(FTPS_UserThread, (VOID*)&sClient);

		Sleep(1);
	}

	return FTP_OK;
}

ULONG FTPS_Enable()
{
	/* create ftp socket */
	g_sFtpListenSocket = FTP_SOCK_Create(g_FtpsockSource, g_Ftpsockport);
	if (INVALID_SOCKET == g_sFtpListenSocket) {
		FTP_debug("FTPS_Main, socket_create failed.");
		return FTP_ERR;
	}

	/* create FTP Listen Thread */
	thread_create(FTPS_ListenThread, NULL);

	FTP_debug("FTP server enable ok. (port=%u)", g_Ftpsockport);

	return FTP_OK;
}

ULONG FTPS_Disable()
{
	closesocket(g_sFtpListenSocket);
	g_sFtpListenSocket = INVALID_SOCKET;

	FTP_debug("FTP server disable ok. (port=%u)", g_Ftpsockport);

	return FTP_OK;
}

ULONG FTPS_SH_Enable()
{
	if (TRUE == g_ulFtpEnable) {
		return FTP_OK;
	}

	g_ulFtpEnable = TRUE;
	if (FTP_OK != FTPS_Enable()) {
		g_ulFtpEnable = FALSE;
		return FTP_ERR;
	}

	return FTP_OK;
}

ULONG FTPS_SH_Disable()
{
	if (FALSE == g_ulFtpEnable) {
		return FTP_OK;
	}

	g_ulFtpEnable = FALSE;
	FTPS_Disable();

	return FTP_OK;
}

ULONG FTPS_SH_SetPort(ULONG port)
{
	ULONG portOld = g_Ftpsockport;

	if (port == g_Ftpsockport) {
		return FTP_OK;
	}
	
	FTPS_Disable();
	g_Ftpsockport = port;

	if (TRUE != g_ulFtpEnable) {
		return FTP_OK;
	}

	if (FTP_OK != FTPS_Enable()) {
		g_Ftpsockport = portOld;
		FTPS_Enable();
		return FTP_ERR;
	}

	return FTP_OK;
}

ULONG FTPS_SH_SetSource(ULONG ip)
{
	ULONG ipOld = g_FtpsockSource;

	if (ip == g_FtpsockSource) {
		return FTP_OK;
	}
	
	FTPS_Disable();
	g_FtpsockSource = ip;

	if (TRUE != g_ulFtpEnable) {
		return FTP_OK;
	}

	if (FTP_OK != FTPS_Enable()) {
		g_FtpsockSource = ipOld;
		FTPS_Enable();
		return FTP_ERR;
	}

	return FTP_OK;
}

ULONG FTPS_Main()
{
	return FTP_OK;
}

#endif

