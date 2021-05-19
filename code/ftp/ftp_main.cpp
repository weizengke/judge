
#include "ftp_server.h"

#include "ic/include/debug_center_inc.h"
#include "pdt_common_inc.h"
#include "event/include/event_pub.h"
#include "aaa/aaa.h"
#include "icli.h"
#include "root.h"


#if (OS_YES == OSP_MODULE_FTPS)

enum FTPS_CMO_TBLID_EM {
	FTPS_CMO_TBL_CFG = 0,		
};

enum FTPS_CMO_CFG_ID_EM {
	FTPS_CMO_CFG_UNDO = CMD_ELEMID_DEF(MID_FTP, FTPS_CMO_TBL_CFG, 0),
	FTPS_CMO_CFG_ENABLE ,
	FTPS_CMO_CFG_PORT ,
	FTPS_CMO_CFG_PORT_NUM ,
	FTPS_CMO_CFG_SERVER_IP ,
	FTPS_CMO_CFG_SERVER_SRC,
	FTPS_CMO_CFG_SERVER_SRC_IP ,
};

extern BOOL g_ulFtpEnable;
extern ULONG FTPS_SH_Enable();
extern ULONG FTPS_SH_Disable();

ULONG FTPS_CFG_Callback(VOID *pRcvMsg)
{
	ULONG ulRet = OS_OK;
	ULONG iLoop = 0;
	ULONG iElemNum = 0;
	ULONG iElemID = 0;
	VOID *pElem = NULL;
	ULONG vtyId = 0;
	ULONG isUndo = 0;
	ULONG isEnable = 0;
	ULONG isPort = 0;
	ULONG portNum = 0;
	ULONG isSrc = 0;
	ULONG srcIP = 0;

	vtyId = cmd_get_vty_id(pRcvMsg);
	iElemNum = cmd_get_elem_num(pRcvMsg);

	for (iLoop = 0; iLoop < iElemNum; iLoop++)
	{	
		pElem = cmd_get_elem_by_index(pRcvMsg, iLoop);
		iElemID = cmd_get_elemid(pElem);

		switch(iElemID)
		{
			case FTPS_CMO_CFG_UNDO:
				isUndo = 1;
				break;
				
			case FTPS_CMO_CFG_ENABLE:		
				isEnable = OS_YES;
				break;

			case FTPS_CMO_CFG_PORT:		
				isPort = OS_YES;
				break;

			case FTPS_CMO_CFG_PORT_NUM:		
				portNum = cmd_get_ulong_param(pElem);
				break;
			case FTPS_CMO_CFG_SERVER_IP:
				{
					ULONG ulIp = 0;
					char ip[25] = {0};
					extern VOID cmd_ip_ulong_to_string(ULONG ip, CHAR *buf);
					extern ULONG FTPC_Run(ULONG vtyId,ULONG ulPort,CHAR * szIP);
					
					ulIp = cmd_get_ip_ulong_param(pElem);
					cmd_ip_ulong_to_string(ulIp, ip);
					FTPC_Run(vtyId, 21, ip);

				}break;
			case FTPS_CMO_CFG_SERVER_SRC:		
				isSrc = OS_YES;
				break;

			case FTPS_CMO_CFG_SERVER_SRC_IP:
				{
					char ip[25] = {0};
					extern VOID cmd_ip_ulong_to_string(ULONG ip, CHAR *buf);
					srcIP = cmd_get_ip_ulong_param(pElem);
					break;

				}
			default:
				break;
		}
	}

	if (OS_YES == isEnable)
	{
		if (0 == isUndo)
		{
			ulRet = FTPS_SH_Enable();
			if (OS_OK != ulRet)
			{
				vty_printf(vtyId, "Error: FTP server enable failed.\r\n");
			}			
		}
		else
		{
			ulRet = FTPS_SH_Disable();
			if (OS_OK != ulRet)
			{
				vty_printf(vtyId, "Error: FTP server disable failed.\r\n");
			}
		}
		
		return 0;
	}

	if (OS_YES == isPort) {
		if (OS_YES == isUndo) {
			portNum = 21;
		}

		ULONG FTPS_SH_SetPort(ULONG port);
		ulRet = FTPS_SH_SetPort(portNum);
		if (OS_OK != ulRet) {
			vty_printf(vtyId, "Error: Set FTP server port failed.\r\n");
		}
	}

	if (OS_YES == isSrc) {
		if (OS_YES == isUndo) {
			srcIP = 0;
		}

		ULONG FTPS_SH_SetSource(ULONG ip);
		ulRet = FTPS_SH_SetSource(srcIP);
		if (OS_OK != ulRet) {
			vty_printf(vtyId, "Error: Set FTP server source failed.\r\n");
		}
	}	
	

	return ulRet;
}


ULONG FTPS_CMD_Callback(VOID *pRcvMsg)
{
	ULONG iRet = 0;
	ULONG iTBLID = 0;
	
	iTBLID = cmd_get_first_elem_tblid(pRcvMsg);
		
	switch(iTBLID)
	{
		case FTPS_CMO_TBL_CFG:
			iRet = FTPS_CFG_Callback(pRcvMsg);
			break;

		default:
			break;
	}

	return iRet;
}


ULONG FTPS_RegCmdEnable()
{
	CMD_VECTOR_S * vec = NULL;

	CMD_VECTOR_NEW(vec);

	// 1
	cmd_regelement_new(FTPS_CMO_CFG_UNDO,	CMD_ELEM_TYPE_KEY,	  "undo",		   "Undo Operation", vec);
	// 2
	cmd_regelement_new(CMD_ELEMID_NULL,		CMD_ELEM_TYPE_KEY,	  "ftp",		   "FTP protocol", vec);
	// 3
	cmd_regelement_new(CMD_ELEMID_NULL,		CMD_ELEM_TYPE_KEY,	  "server",		   "FTP Server",   vec);
	// 4
	cmd_regelement_new(FTPS_CMO_CFG_ENABLE,	CMD_ELEM_TYPE_KEY,	  "enable",		   "Enable FTP service", vec);
	// 5
	cmd_regelement_new(FTPS_CMO_CFG_PORT,	CMD_ELEM_TYPE_KEY,	  "port",		   "Socket port of FTP server", vec);
	// 6
	cmd_regelement_new(FTPS_CMO_CFG_PORT_NUM,	CMD_ELEM_TYPE_INTEGER,"INTEGER<1-65535>", "Socket port of FTP server, default is 21.", vec);
	// 7
	cmd_regelement_new(FTPS_CMO_CFG_SERVER_IP,	CMD_ELEM_TYPE_IP, CMD_ELMT_IP,	"IP Address", vec);
	// 8 
	cmd_regelement_new(FTPS_CMO_CFG_SERVER_SRC,		CMD_ELEM_TYPE_KEY,	  "server-source",	"FTP Server source address",   vec);
	// 9
	cmd_regelement_new(FTPS_CMO_CFG_SERVER_SRC_IP,	CMD_ELEM_TYPE_IP, 	  CMD_ELMT_IP,	"Source IP Address", vec);
	
	cmd_install_command(MID_FTP, VIEW_SYSTEM,  " 1 2 3 4 ", vec);
	cmd_install_command(MID_FTP, VIEW_SYSTEM,  " 2 3 4 ", vec);
	
	cmd_install_command(MID_FTP, VIEW_SYSTEM,  " 1 2 3 5 ", vec);
	cmd_install_command(MID_FTP, VIEW_SYSTEM,  " 2 3 5 6 ", vec);

	cmd_install_command(MID_FTP, VIEW_USER,  " 2 7 ", vec);

	cmd_install_command(MID_FTP, VIEW_SYSTEM,  " 1 2 8 ", vec);
	cmd_install_command(MID_FTP, VIEW_SYSTEM,  " 2 8 9 ", vec);

	CMD_VECTOR_FREE(vec);

	return 0;
}

ULONG FTPS_RegCmd()
{
	(VOID)cmd_regcallback(MID_FTP, FTPS_CMD_Callback);	

	(VOID)FTPS_RegCmdEnable();

	(VOID)DEBUG_PUB_RegModuleDebugs(MID_FTP, "ftp", "FTP protocol");
	
	return OS_OK;
}

ULONG FTPS_BuildRun(CHAR **ppBuildrun, ULONG ulIncludeDefault)
{
	CHAR *pBuildrun = NULL;

	*ppBuildrun = (CHAR*)malloc(BDN_MAX_BUILDRUN_SIZE);
	if (NULL == *ppBuildrun)
	{
		return OS_ERR;
	}
	memset(*ppBuildrun, 0, BDN_MAX_BUILDRUN_SIZE);
	
	pBuildrun = *ppBuildrun;
	
	extern ULONG g_Ftpsockport;
	if (21 != g_Ftpsockport || OS_YES == ulIncludeDefault) {
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"ftp server port %u", g_Ftpsockport);
	}

	extern ULONG g_FtpsockSource;
	if (0 != g_FtpsockSource || OS_YES == ulIncludeDefault) {
		struct in_addr addr;
		addr.s_addr = htonl(g_FtpsockSource);
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"ftp server-source %s", inet_ntoa(addr));
	}
	
	
	if (TRUE == g_ulFtpEnable)
	{
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"ftp server enable");
	}
	else
	{
		if (OS_YES == ulIncludeDefault)
		{
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"undo ftp server enable");
		}
	}

	return OS_OK;
}

ULONG FTPS_EVT_UserDel(ULONG keyId, ULONG cmdId, VOID *pData, VOID **ppInfo)
{

	return OS_OK;
}

int FTPS_Init()
{
	(VOID)AAA_EvtRegistFunc("FTPS", AAA_EVT_USER_DEL, 0, FTPS_EVT_UserDel);
	
	return FTP_OK;
}

int FTPS_TaskEntry(void *pEntry)
{	
	(VOID)FTPS_RegCmd();

	(VOID)cli_bdn_regist(MID_FTP, VIEW_SYSTEM, BDN_PRIORITY_HIGH + 100, FTPS_BuildRun);
	
	/* ѭ����ȡ��Ϣ���� */
	for(;;)
	{
		/* ��Ȩ */
		Sleep(1000);
	}

	return FTP_OK;
}

APP_INFO_S g_FTPSAppInfo =
{
	MID_FTP,
	"FTPS",
	FTPS_Init,
	FTPS_TaskEntry
};

void FTPS_RegAppInfo()
{
	APP_RegistInfo(&g_FTPSAppInfo);
}

#endif
