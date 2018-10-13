#ifndef FTP_SERVER_H
#define FTP_SERVER_H

#include "ftp_common.h"

/* Commands enumeration */
typedef enum emFTP_CMD_LIST_E
{ 
    FTP_CMD_ABOR, 
	FTP_CMD_CWD,
	FTP_CMD_DELE,
	FTP_CMD_LIST,
	FTP_CMD_MDTM,
	FTP_CMD_MKD,
	FTP_CMD_NLST,
	FTP_CMD_PASS,
	FTP_CMD_PASV,
    FTP_CMD_PORT,
    FTP_CMD_PWD,
    FTP_CMD_QUIT,
    FTP_CMD_RETR,
    FTP_CMD_RMD,
    FTP_CMD_RNFR,
    FTP_CMD_RNTO,
    FTP_CMD_SITE,
    FTP_CMD_SIZE,
    FTP_CMD_STOR,
    FTP_CMD_TYPE,
    FTP_CMD_USER,
    FTP_CMD_NOOP
	
} FTP_CMD_LIST_E;

typedef struct ftp_cmd_st
{
    CHAR cmd[FTP_CMD_CODE_SIZE];
    CHAR arg[FTP_CMD_ARGV_SIZE];
} FTP_CMD_S;

typedef struct ftp_user_st
{
	CHAR szUsername[FTP_USERNAME_SIZE];
	CHAR szPassword[FTP_PASSWORD_SIZE];
	CHAR szRootDir[FTP_FILEPATH_SIZE];
	CHAR szCurrentDir[FTP_FILEPATH_SIZE];
	ULONG ulAccess;
	LONG lSockCtrl;
	LONG lSockData;
	ULONG ulDataPort;
	FTP_CMD_S stCMD;
}FTP_USER_S;

typedef struct ftp_cmd_handle_st 
{
	ULONG ulCmdCode;
	CHAR  szCmdStr[8];
	ULONG (*pKeyCallbackfunc)(FTP_USER_S *);	
} FTP_CMD_HANDLE_S;

extern ULONG FTPS_Handler_LIST(FTP_USER_S *pUser);
extern ULONG FTPS_Handler_PORT(FTP_USER_S *pUser);
extern ULONG FTPS_Handler_PASS(FTP_USER_S *pUser);
extern ULONG FTPS_Handler_QUIT(FTP_USER_S *pUser);
extern ULONG FTPS_Handler_RETR(FTP_USER_S *pUser);
extern ULONG FTPS_Handler_STOR(FTP_USER_S *pUser);
extern ULONG FTPS_Handler_TYPE(FTP_USER_S *pUser);
extern ULONG FTPS_Handler_USER(FTP_USER_S *pUser);
extern ULONG FTPS_Handler_NOOP(FTP_USER_S *pUser);

extern ULONG FTPS_GetSocketPort();
extern LONG FTPS_GetSocket();
extern VOID FTPS_SetSocket(LONG lSocket);
extern FTP_CMD_HANDLE_S * FTPS_GetHandler(CHAR *szCMD);
extern VOID FTPS_Run(LONG lSockCtrl);
extern int FTPS_UserThread(void *pEntry);
extern int FTPS_ListenThread(void *pEntry);
extern ULONG FTPS_UserAuthen(char*user, char*pass);
extern ULONG FTPS_Main();

#endif
