#ifndef FTP_CLIENT_H
#define FTP_CLIENT_H

#include "ftp_common.h"

typedef struct ftpc_user_st
{
    ULONG vtyId;
	ULONG access;
	ULONG isPasv;
	ULONG dataPort;
	socket_t sockCtrl;
	socket_t sockData;	
	socket_t sockClient;
}FTPC_USER_S;

ULONG FTPC_PUB_Upload(CHAR *ip, ULONG port, 
                      CHAR *username, CHAR *password,
                      CHAR *filePath, CHAR *fileName);

#endif