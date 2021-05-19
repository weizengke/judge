
 #include "ftp_client.h"

#include "../include/icli.h"
#include "kernel.h"

#include "ic/include/debug_center_inc.h"
#include "include/pdt_common_inc.h"
#include "event/include/event_pub.h"
#include "aaa/aaa.h"
#include "util/util.h"

ULONG FTPC_SendCmd(FTPC_USER_S *user, char *cmd)
{
	int ret;
	FTP_debug("FTPC send command: %s", cmd);
	ret = FTP_Send(user->sockCtrl, cmd, (int)strlen(cmd));
	if (ret != FTP_OK) {
		FTP_debug("failed to send command: %s",cmd);
		return FTP_ERR;
	}
	vty_printf(user->vtyId, "> %s", cmd);
	return FTP_OK;
}
 
int FTPC_RecvRespond(FTPC_USER_S *user, char *resp, int len)
{
	int ret, off;
	len -= 1;
	for (off = 0; off < len; off += ret) {
		ret = FTP_Recv(user->sockCtrl, &resp[off], 1);
		if (ret < 0) {
			FTP_debug("recv respond error(ret=%d)!\r\n", ret);
			return 0;
		}

		if (resp[off] == '\n') {
			break;
		}
	}

	resp[off+1] = 0;
	FTP_debug("respond:%s", resp);
	vty_printf(user->vtyId, "< %s", resp);
	return atoi(resp);
}
int FTPC_ListenThread(void *pEntry)
{
	sockaddr_in remoteAddr;
	socklen_t nAddrLen = sizeof(remoteAddr);
	FTPC_USER_S *user = (FTPC_USER_S *)pEntry;
	while(INVALID_SOCKET != user->sockData) {
		/* access a new user socket */
		user->sockClient = accept(user->sockData, (SOCKADDR *)&remoteAddr, &nAddrLen);
		if (user->sockClient == INVALID_SOCKET) {
			break;
		}

		Sleep(1);
	}

	return FTP_OK;
}
ULONG FTPC_EnterPasv(FTPC_USER_S *user, char *ip, int *port)
{
	int ret;
	char *find;
	int a,b,c,d;
	int pa,pb;
	char recvBuff[FTP_DATABUF_SIZE];

	if(FTP_OK != FTPC_SendCmd(user, "PASV\r\n")) {
		return FTP_ERR;
	}

	/* waiting for msg: 227 Entering Passive Mode (127,0,0,1,213,80).*/
	ret = FTPC_RecvRespond(user, recvBuff, sizeof(recvBuff));
	if(ret != 227) {
		return FTP_ERR;
	}

	find = strrchr(recvBuff, '(');
	sscanf(find, "(%d,%d,%d,%d,%d,%d)", &a, &b, &c, &d, &pa, &pb);
	sprintf(ip, "%d.%d.%d.%d", a, b, c, d);
	*port = pa * 256 + pb;

	user->sockData = FTP_SOCK_Connect(*port, ip);
	if (user->sockData == INVALID_SOCKET) {
		return FTP_ERR;
	}

	return FTP_OK;
}

ULONG FTPC_EnterPortMode(FTPC_USER_S *user, char *ip, int *port)
{
	int ret;
	char recvBuff[FTP_DATABUF_SIZE];

	int ipaddr[4] = {0};
	FTP_GetLocalIpBySocket(user->sockCtrl, ipaddr);

	typedef struct Port
	{
		int p1;
		int p2;
	} Port;
	Port port_ = {0};
	srand(time(NULL));
    port_.p1 = 128 + (rand() % 64);
    port_.p2 = rand() % 0xff;

	CHAR szCMD[FTP_CMD_MAXSIZE] = {0};
	sprintf(szCMD, "PORT %d,%d,%d,%d,%d,%d\r\n",
		ipaddr[0],ipaddr[1],ipaddr[2], ipaddr[3], port_.p1, port_.p2);
	if(FTP_OK != FTPC_SendCmd(user, szCMD)) {
		return FTP_ERR;
	}

	/* waiting for msg: 200 port command okay.*/
	ret = FTPC_RecvRespond(user, recvBuff, sizeof(recvBuff));
	if(ret != 200) {
		return FTP_ERR;
	}

	sprintf(ip, "%d.%d.%d.%d", ipaddr[0], ipaddr[1],ipaddr[2], ipaddr[3]);
	*port = port_.p1 * 256 + port_.p2;

	user->sockData = FTP_SOCK_Create(inet_addr(ip), *port);
	
	thread_create(FTPC_ListenThread, user);

	return FTP_OK;
}
ULONG FTPC_Upload(FTPC_USER_S *user, CHAR *filePath, char *fileName)
{
	ULONG  ret;
	char ipaddr[32] = "127.0.0.1";
	int  port = 21;
	char sendBuff[FTP_DATABUF_SIZE];
	char recvBuff[FTP_DATABUF_SIZE];

	if (strlen(filePath) == 0 || strlen(fileName) == 0) {
		return FTP_ERR;
	}

	if (user->isPasv) {
		ret = FTPC_EnterPasv(user, ipaddr, &port);
	} else {
		ret = FTPC_EnterPortMode(user, ipaddr, &port);		
	}
	if (ret != FTP_OK) {
		return FTP_ERR;
	}

	sprintf(sendBuff, "STOR %s\r\n", fileName);
	ret = FTPC_SendCmd(user, sendBuff);
	if(ret != FTP_OK) {
		FTP_CLOSESOCKET(user->sockData);
		FTP_CLOSESOCKET(user->sockClient);
		return FTP_ERR;
	}

	/* 150 Opening data connection for directory list. */
	ret = FTPC_RecvRespond(user, recvBuff, sizeof(recvBuff));
	if (ret != 150) {
		FTP_CLOSESOCKET(user->sockData);
		FTP_CLOSESOCKET(user->sockClient);
		return FTP_ERR;
	}

    char buff[FTP_DATABUF_SIZE] = {0};
    size_t num_read;
    FILE *fd = fopen(filePath, "rb");
    if (!fd) {
		FTP_CLOSESOCKET(user->sockData);
		FTP_CLOSESOCKET(user->sockClient);
        FTP_debug("FTPC_PUB_Upload, fopen error");
		return FTP_ERR;
    }

    do {
        num_read = fread(buff, 1, FTP_DATABUF_SIZE, fd);
        if (num_read <= 0) {
            FTP_debug("FTPC_PUB_Upload, fread finish. (num_read=%u)", num_read);
            break;
        }

        /* send block */
        if (FTP_OK != FTP_Send(user->sockClient, buff, num_read)) {
            FTP_debug("FTPC_PUB_Upload, send error in send. (num_read=%u)", num_read);
            break;
        }
    } while (num_read > 0);

    fclose(fd);
	FTP_CLOSESOCKET(user->sockData);
	FTP_CLOSESOCKET(user->sockClient);
	
	/* 226 File sent ok */
	ret = FTPC_RecvRespond(user, recvBuff, sizeof(recvBuff));
    if (ret != 226) {
        return FTP_ERR;
    }

	return FTP_OK;
}

ULONG FTPC_Download(FTPC_USER_S *user, char *filename)
{
	int   i;
	ULONG ret;
	char  ipaddr[32];
	int   port;
	char recvBuff[FTP_DATABUF_SIZE];
	char sendBuff[FTP_DATABUF_SIZE];

	if (strlen(filename) == 0) {
		return FTP_ERR;
	}

	if (user->isPasv) {
		ret = FTPC_EnterPasv(user, ipaddr, &port);
	} else {
		ret = FTPC_EnterPortMode(user, ipaddr, &port);		
	}
	if (ret != FTP_OK) {
		return FTP_ERR;
	}

	sprintf(sendBuff, "RETR %s\r\n", filename);
	ret = FTPC_SendCmd(user, sendBuff);
	if (ret != FTP_OK) {
		FTP_CLOSESOCKET(user->sockData);
		FTP_CLOSESOCKET(user->sockClient);
		return FTP_ERR;
	}

	/* waiting for server msg: 150 Opening data connection for directory list. */
	ret = FTPC_RecvRespond(user, recvBuff, sizeof(recvBuff));
	if (ret != 150) {
		FTP_CLOSESOCKET(user->sockData);
		FTP_CLOSESOCKET(user->sockClient);
		return FTP_ERR;
	}

	/* open file */
	FILE* fd = fopen(filename, "wb+");
	if (!fd) {
		sprintf(sendBuff, "550 Requested action not taken.\n");
		(VOID)FTP_Send(user->sockCtrl, sendBuff, strlen(sendBuff));
		FTP_CLOSESOCKET(user->sockData);
		FTP_CLOSESOCKET(user->sockClient);
		return FTP_ERR;
	}

	int num_read = 0;
	int total_read = 0;
	do {
		num_read = FTP_Recv(user->sockClient, recvBuff, sizeof(recvBuff));
		if (num_read <= 0) {
			break;
		}
		total_read += num_read;

		num_read = fwrite(recvBuff, 1, num_read, fd);
		if (num_read <= 0) {
			FTP_debug("FTPC_Download, error in fwrite. (num_read=%d)", num_read);
			break;
		}
	} while(num_read > 0);
	FTP_CLOSESOCKET(user->sockData);
	FTP_CLOSESOCKET(user->sockClient);

	vty_printf(user->vtyId, "Download %s complete, %d bytes received.\r\n", filename, total_read);

	/* close file handle */
	fclose(fd);

	/* 226 File sent ok */
	ret = FTPC_RecvRespond(user, recvBuff, sizeof(recvBuff));
    if (ret != 226) {
        return FTP_ERR;
    }
	return FTP_OK;
}

ULONG FTPC_Login(FTPC_USER_S *user, CHAR *ip, ULONG port, CHAR *username, CHAR *password)
{
	ULONG ret;
	socket_t sock;
	char recvBuff[FTP_DATABUF_SIZE];
	char sendBuff[FTP_DATABUF_SIZE];

	sock = FTP_SOCK_Connect(port, ip);
	if (sock == INVALID_SOCKET) {
		FTP_debug("connect server failed!\r\n");
		return FTP_ERR_CONNECT_FAILED;
	}
	user->sockCtrl = sock;

	/* 220 FTP service ready. */
	ret = FTPC_RecvRespond(user, recvBuff, sizeof(recvBuff));
	if (ret != 220) {
		FTP_debug("bad server, ret=%d!\r\n", ret);
		FTP_CLOSESOCKET(sock);
		return FTP_ERR;
	}
	
	sprintf(sendBuff, "USER %s\r\n", username);
	ret = FTPC_SendCmd(user, sendBuff);
	if (ret != FTP_OK) {
		FTP_CLOSESOCKET(sock);
		return FTP_ERR;
	}
	/* 331 Password required */
	ret = FTPC_RecvRespond(user, recvBuff, sizeof(recvBuff));
	if (ret != 331) {
		FTP_CLOSESOCKET(sock);
		return FTP_ERR;
	}
	
	sprintf(sendBuff, "PASS %s\r\n", password);
	ret = FTPC_SendCmd(user, sendBuff);
	if(ret != FTP_OK) {
		FTP_CLOSESOCKET(sock);
		return FTP_ERR;
	}
	/* 230 User xxxx logged in. */
	ret = FTPC_RecvRespond(user, recvBuff, sizeof(recvBuff));
	if (ret != 230) {
		FTP_CLOSESOCKET(sock);
		return FTP_ERR;
	}

	ret = FTPC_SendCmd(user, "TYPE I\r\n");
	if(ret != FTP_OK) {
		FTP_CLOSESOCKET(sock);
		return FTP_ERR;
	}

	/*  200 Type set to I. */
	ret = FTPC_RecvRespond(user, recvBuff, sizeof(recvBuff));
	if (ret != 200) {
		FTP_CLOSESOCKET(sock);
		return FTP_ERR;
	}

	user->sockCtrl = sock;
	return FTP_OK;
}

ULONG FTPC_LoginEx(FTPC_USER_S *user, CHAR *ip, ULONG port)
{
	ULONG ret;
	socket_t sock;
	char recvBuff[FTP_DATABUF_SIZE];
	char sendBuff[FTP_DATABUF_SIZE];

	sock = FTP_SOCK_Connect(port, ip);
	if (sock == INVALID_SOCKET) {
		FTP_debug("connect server failed!\r\n");
		return FTP_ERR_CONNECT_FAILED;
	}	
	user->sockCtrl = sock;

	int ipLocal[4];
	int ipPeer[4];
	FTP_GetLocalIpBySocket(sock, ipLocal);
	FTP_GetPeerIpBySocket(sock, ipPeer);
	FTP_debug("ipLocal:%d.%d.%d.%d, ip_peer:%d.%d.%d.%d",
			  ipLocal[0],ipLocal[1],ipLocal[2],ipLocal[3], ipPeer[0],ipPeer[1],ipPeer[2],ipPeer[3]);

	/* waiting for msg: 220 FTP service ready. */
	ret = FTPC_RecvRespond(user, recvBuff, sizeof(recvBuff));	
	if (ret != 220) {
		FTP_debug("bad server, ret=%d.", ret);
		vty_printf(user->vtyId, "FTP server connect failed.");
		FTP_CLOSESOCKET(sock);
		return FTP_ERR;
	}

	vty_printf(user->vtyId, "Username:");
	char username[32];
	scanf("%s", username);
	sprintf(sendBuff, "USER %s\r\n", username);
	ret = FTPC_SendCmd(user, sendBuff);
	if (ret != FTP_OK) {
		FTP_CLOSESOCKET(sock);
		return FTP_ERR;
	}
	/* waiting for msg: 331 Password required. */
	ret = FTPC_RecvRespond(user, recvBuff, sizeof(recvBuff));
	if (ret != 331) {
		FTP_CLOSESOCKET(sock);
		return FTP_ERR;
	}
	
	vty_printf(user->vtyId, "Password:");
	char password[32];
	scanf("%s", password);
	sprintf(sendBuff, "PASS %s\r\n", password);
	ret = FTPC_SendCmd(user, sendBuff);
	if(ret != FTP_OK) {
		closesocket(sock);
		return FTP_ERR;
	}

	/*  230 User xxx logged in.*/
	ret = FTPC_RecvRespond(user, recvBuff, sizeof(recvBuff));
	if (ret != 230) {
		closesocket(sock);
		return FTP_ERR;
	}

	ret = FTPC_SendCmd(user, "TYPE I\r\n");
	if(ret != FTP_OK) {
		closesocket(sock);
		return FTP_ERR;
	}
	/* 200 Type set to I. */
	ret = FTPC_RecvRespond(user, recvBuff, sizeof(recvBuff));
	if (ret != 200) {
		closesocket(sock);
		return FTP_ERR;
	}

	return FTP_OK;
}

VOID FTPC_Quit(FTPC_USER_S *user)
{
	ULONG ret;
	char recvBuff[FTP_DATABUF_SIZE];

	ret = FTPC_SendCmd(user, "QUIT\r\n");
	if (ret != FTP_OK) {
		return;
	}

	/* waiting for msg: 221 goodbye */
	ret = FTPC_RecvRespond(user, recvBuff, sizeof(recvBuff));

	closesocket(user->sockCtrl);
	user->sockCtrl = INVALID_SOCKET;
}

VOID FTPC_Passive(FTPC_USER_S *user, ULONG pasv)
{
	user->isPasv = pasv;
	FTP_debug("Transfer passive mode change to %d.", pasv);
}

int FTPC_RecvData(FTPC_USER_S *user)
{
	int ret = 0;	
	char recvBuff[FTP_DATABUF_SIZE];

	do {
		ret = FTP_Recv(user->sockClient, recvBuff, sizeof(recvBuff));
		if (ret > 0) {
			vty_printf(user->vtyId, "%s", recvBuff);
		} else {
			FTP_debug("FTPC_DataRecvThread recv ret=%d", ret);
		}
	} while(ret > 0);

	return FTP_OK;
}

ULONG FTPC_LIST(FTPC_USER_S *user)
{
	ULONG ret = 0;
	CHAR szCMD[FTP_CMD_MAXSIZE] = {0};
	char recvBuff[FTP_DATABUF_SIZE];
	char ipaddr[32] = "127.0.0.1";
	int  port = 21;

	if (user->isPasv) {
		ret = FTPC_EnterPasv(user, ipaddr, &port);
	} else {
		ret = FTPC_EnterPortMode(user, ipaddr, &port);		
	}
	if (ret != FTP_OK) {
		return FTP_ERR;
	}

	sprintf(szCMD, "LIST\n");
	if (OS_OK != FTPC_SendCmd(user, szCMD)) {
		FTP_CLOSESOCKET(user->sockData);
		FTP_CLOSESOCKET(user->sockClient);
		return FTP_ERR;
	}

	/* waiting for server msg: 150 Opening data connection for directory list. */
	ret = FTPC_RecvRespond(user, recvBuff, sizeof(recvBuff));
	if (ret != 150) {
		FTP_CLOSESOCKET(user->sockData);
		FTP_CLOSESOCKET(user->sockClient);		
		return FTP_ERR;
	}		
	
    FTPC_RecvData(user);
	FTP_CLOSESOCKET(user->sockData);
	FTP_CLOSESOCKET(user->sockClient);

	/* waiting for msg: 226 File sent ok */
	ret = FTPC_RecvRespond(user, recvBuff, sizeof(recvBuff));
	if (ret != 226) {
		return FTP_ERR;
	}

	return FTP_OK;
}

ULONG FTPC_Help(FTPC_USER_S *user)
{
	vty_printf(user->vtyId, " ?                     Get help.\r\n"
						    " cd                    Change dirction.\r\n"
							" dir                   Get listed files and folders in FTP server.\r\n"
							" get                   Download file from FTP server.\r\n"
							" help                  Get help.\r\n"
							" put                   Upload file to FTP server.\r\n"
							" [ undo ] passive      Change Transfer passive mode.\r\n"
							" quit                  Disconnect with FTP server.\r\n");
	return FTP_OK;
}
VOID FTPC_CWD(FTPC_USER_S *user, CHAR *path)
{
	ULONG ret;
	char recvBuff[FTP_DATABUF_SIZE];
	CHAR szCMD[FTP_CMD_MAXSIZE] = {0};

	sprintf(szCMD, "CWD %s\r\n", path);
	ret = FTPC_SendCmd(user, szCMD);
	if (ret != FTP_OK) {
		return;
	}

	/* waiting for msg: 250 CWD command successful. "xxx" is current directory. */
	ret = FTPC_RecvRespond(user, recvBuff, sizeof(recvBuff));
}

ULONG FTPC_Run(ULONG vtyId, ULONG port, CHAR * szIP)
{
	ULONG ret;
	FTPC_USER_S user = {0};
	user.vtyId = vtyId;
	user.isPasv = 0;

	if (CMD_VTY_CONSOLE_ID != vtyId) {
		vty_printf(vtyId, "Error: Only console user support FTP client.\r\n");
		return OS_ERR;
	}

	ret = FTPC_LoginEx(&user, szIP, port);
	if (ret != FTP_OK) {
		return FTP_ERR;
	}

	CHAR buff[FTP_CMD_ARGV_SIZE + FTP_CMD_CODE_SIZE + 1] = {0};
#ifdef MSVC
	while(gets_s(buff, sizeof(buff) - 1)) {	
#else
	while(gets(buff)) {
#endif
		CHAR cmd[FTP_CMD_CODE_SIZE] = {0};
		CHAR arg[FTP_CMD_ARGV_SIZE]= {0};
		char recvBuff[FTP_DATABUF_SIZE];
		FTP_debug("FTP run: vty=%u, cmd=%s.", user.vtyId, buff);
		sscanf(buff, "%s%s", cmd, arg);
		if (0 == strcmp(cmd, "help") || 0 == strcmp(cmd, "?") ) {
			FTPC_Help(&user);
		} else if (0 == strcmp(cmd, "dir")) {
			FTPC_LIST(&user);
		} else if (0 == strcmp(cmd, "get")) {
			FTPC_Download(&user, arg);
		} else if (0 == strcmp(cmd, "put")) {
			FTPC_Upload(&user, arg, arg);
		} else if (0 == strcmp(cmd, "cd")) {
			FTPC_CWD(&user, arg);
		} else if (0 == strcmp(cmd, "undo") && 0 == strcmp(arg, "passive")) {
			FTPC_Passive(&user, 0);
		} else if (0 == strcmp(cmd, "passive")) {
			FTPC_Passive(&user, 1);
		} else if (0 == strcmp(cmd, "quit")) {
			FTPC_Quit(&user);
			break;
		} 
		vty_printf(user.vtyId, "[ftp]");
		memset(buff, 0, sizeof(buff));
	}
}

ULONG FTPC_PUB_Upload(CHAR *ip, ULONG port, 
                      CHAR *username, CHAR *password,
                      CHAR *filePath, CHAR *fileName)
{
    ULONG ret = FTP_OK;
    FTPC_USER_S user = {0};

	user.vtyId = CMD_VTY_INVALID_ID;

    ret = FTPC_Login(&user, ip, port, username, password);
    if (ret != FTP_OK) {
        FTP_debug("FTPC_Login failed, ret=%d.\r\n", ret);
        return FTP_ERR;
    }

    ret = FTPC_Upload(&user, filePath, fileName);
    if (ret != FTP_OK) {
        FTP_debug("FTPC_Upload failed, ret=%d.\r\n", ret);
        return FTP_ERR;
    }

    return FTP_OK;
}

void ftpc_test()
{
	ULONG ret;
	FTPC_USER_S user = {0};

    ret = FTPC_Login(&user, "127.0.0.1", 21, "ftp", "root@123");
    if (ret != FTP_OK) {
        FTP_debug("FTPC_Login failed, ret=%d.\r\n", ret);
        return;
    }

    FTP_debug("FTPC_Upload=%d\n", FTPC_Upload(&user, "libjudger.dll.a", "libjudger.dll.a"));
	closesocket(user.sockCtrl);
}   