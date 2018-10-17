#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
	
#ifdef _LINUX_
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <assert.h>
#endif
	
#ifdef _WIN32_
#include <conio.h>
#include <io.h>
#include<winsock2.h>
#endif

#include "kernel.h"

#include "osp/common/include/osp_common_def.h"
#include "product/include/pdt_common_inc.h"
#include "osp/command/include/icli.h"
#include "product/main/root.h"
#include "osp/util/util.h"
#include "product/judge/include/judge_inc.h"
#include "product/main/sysmng.h"




using namespace std;

#define SOCKET_PORT 5000

char g_sysname[APP_MAX_SYSNAME_SIZE] = "judger";
char g_startup_config[256] = "config.cfg";
int  g_cfm_recovered = OS_NO;
//HWND g_hWnd = NULL;

socket_t g_sListen;
int g_sock_port = SOCKET_PORT;
char judgePath[MAX_PATH];

#define SYSMNG_Debug(x, args...) debugcenter_print(MID_SYSMNG, x, args)

char *SYSMNG_GetSysname()
{
	return g_sysname;
}

VOID SYSMNG_InitWindows()
{
	//::SetConsoleTitle(g_sysname);

	//g_hWnd=::GetConsoleWindow();

#if 0
	if (g_hWnd !=NULL)
	{
		HANDLE hIcon=NULL;
		hIcon=::LoadImage(GetModuleHandle(NULL),"beyond.ico",
		IMAGE_ICON,32,32,LR_LOADFROMFILE);
		if (hIcon!=NULL)
		{
			::SendMessage(g_hWnd,WM_SETICON,ICON_SMALL,(LPARAM)hIcon);
		}

	}
	else
	{
		pdt_debug_print("Set judge kernel ico failed. ");
	}
#endif

	return ;
}

/* 判断是否是配置恢复阶段 */
ULONG SYSMNG_IsCfgRecoverOver()
{
	return g_cfm_recovered;
}

void SYSMNG_CfgRecover()
{
	ULONG ulRet = OS_OK;
	CHAR cfgpath[256] = {0};
	CHAR line[1024] = {0};

	sprintf(cfgpath, "conf\/%s",g_startup_config);
	
	FILE * fp=fopen(cfgpath,"r");
	if (NULL == fp)
	{
		printf("No startup configuration file.\r\n");
		g_cfm_recovered = OS_YES;
		return;
	}

	printf("Recover configuration begin.\r\n");

	write_log(JUDGE_INFO, "Recover configuration begin.");

	cmd_pub_run("system-view");
	
	while (fgets(line,1024,fp))
	{
		if ('#' != line[0]
			&& strlen(line) > 0)
		{
			line[strlen(line) - 1] = '\0';
			ulRet = cmd_pub_run(line);
			//printf("\r\nRecover command:%s, ulRet=%u", line, ulRet);
			//write_log(JUDGE_INFO, "Recover command(ulRet=%u):%s", ulRet, line);
		}
	}
	
	fclose(fp);

	cmd_pub_run("return");
	
	printf("Eecover configuration end.\r\n");
	write_log(JUDGE_INFO, "Eecover configuration end.");

	g_cfm_recovered = OS_YES;
	
	return;
}

void SYSMNG_ShowCfg(ULONG vtyId)
{
	ULONG ulRet = OS_OK;
	char cfgpath[256] = {0};
	char line[1024] = {0};

	sprintf(cfgpath, "conf\/%s",g_startup_config);
	
	FILE * fp=fopen(cfgpath,"r");
	if (NULL == fp)
	{
		pdt_debug_print("No startup configuration file.");
		fclose(fp);
		return;
	}

	while (fgets(line,1024,fp))
	{
		vty_printf(vtyId,"%s", line);
	}

	vty_printf(vtyId, "\r\n");

	fclose(fp);

	return;
}

#ifdef _WIN32_
long WINAPI SYSMNG_ExceptionFilter(EXCEPTION_POINTERS * excp)
{
	int err = GetLastError();
	extern socket_t g_TelnetSSocket;

	pdt_debug_print("Exception in thread main...[ErrorCode=%u]", err);
	
	write_log(JUDGE_ERROR,
		"!!! Exception in thread main, please mail to 269574524@qq for help. (ErrorCode:%u)", err);

	closesocket(g_sListen);

#if (OS_YES == OSP_MODULE_TELNETS)
	closesocket(g_TelnetSSocket);
#endif

	#ifdef _WIN32_
	WSACleanup();
    #endif
	
    Sleep(5000);
    ShellExecuteA(NULL,"open","judge.exe",NULL,NULL,SW_SHOWNORMAL);

	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

#ifdef _LINUX_
static void segv_error_handle(int v)
{
	extern socket_t g_TelnetSSocket;

	/* kill -s 9 1827 */
	printf("\r\n segv_error_handle...[v=%u]", v);
	
	closesocket(g_sListen);

#if (OS_YES == OSP_MODULE_TELNETS)
	closesocket(g_TelnetSSocket);
#endif

	return;
}

static void install_segv_handler()
{
	struct sigaction siga;
	
	siga.sa_handler = segv_error_handle;
	siga.sa_flags = 0;
	memset(&siga.sa_mask, 0, sizeof(sigset_t));
	
	sigaction(SIGSEGV, &siga, NULL);/* 捕获段非法错误的信号 */
	sigaction(SIGTERM, &siga, NULL);/* 捕获软件终止的信号 */
	sigaction(SIGINT,  &siga, NULL);/* 捕获进程中断的信号 */
}
#endif

int SYSMNG_ListenThread(void *pEntry)
{
	sockaddr_in remoteAddr;
	socket_t sClient;
	socklen_t nAddrLen = sizeof(remoteAddr);
	char buff[1024] = {0};
	char buff1024[1024] = {0};
	char *pBuf = NULL;
	while(TRUE)
	{
		sClient = accept(g_sListen, (SOCKADDR*)&remoteAddr, &nAddrLen);
		if(sClient == INVALID_SOCKET)
		{
			continue;
		}

		memset(buff,0,sizeof(buff));
		memset(buff1024,0,sizeof(buff1024));
		pBuf = buff1024;
		
		int ret = recv(sClient, (char*)buff, sizeof(buff), 0);
		if(ret>0)
		{
			int i = 0;
			for (i = 0; i < 64; i++)
			{
				pBuf += sprintf(pBuf,"%02x ", buff[i]);
				if ((i + 1 )  % 16 == 0)
				{
					pBuf += sprintf(pBuf,"\r\n");
				}
			}

			write_log(JUDGE_INFO,"Recieve packet from (ip:%s, port:%u):\r\n%s",inet_ntoa(remoteAddr.sin_addr), htons(remoteAddr.sin_port), buff1024);
			PDT_Debug(DEBUG_TYPE_MSG, "Recieve packet from (ip:%s, port:%u):\r\n%s",inet_ntoa(remoteAddr.sin_addr), htons(remoteAddr.sin_port), buff1024);
			PDT_Debug(DEBUG_TYPE_MSG, "Message:%s", buff);

			/* 需要检查发送源的合法性 */

			(VOID)SYSMNG_PacketParse(buff, strlen(buff));
			
		}

	
		Sleep(1);
	}

	write_log(JUDGE_ERROR,"ListenThread Crash");
	closesocket(sClient);

	return 0;
}

int SYSMNG_InitSocket()
{
	write_log(JUDGE_INFO,"Start initialization of Socket...");

#ifdef _WIN32_
	WSADATA wsaData;
    WORD sockVersion = MAKEWORD(2, 2);
	if(WSAStartup(sockVersion, &wsaData) != 0)
		return 0;
#endif

	g_sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(g_sListen == INVALID_SOCKET)
	{
		write_log(JUDGE_SYSTEM_ERROR,"create socket error");
		return 0;
	}

	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(g_sock_port);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);


	int trybind=50;
	int ret=0;

	ret = bind(g_sListen,(const sockaddr*)&sin,sizeof(sin));

	while(ret == SOCKET_ERROR && trybind > 0)
	{
		ret = bind(g_sListen,(const sockaddr*)&sin,sizeof(sin));
		//write_log(JUDGE_SYSTEM_ERROR,"bind failed:%d , it will try later...",WSAGetLastError());
		trybind--;
		Sleep(100);
	}

	if(ret<0)
	{
		while (ret == SOCKET_ERROR)
		{
			g_sock_port++;
			sin.sin_port = htons(g_sock_port);
			ret =  bind(g_sListen,(const sockaddr*)&sin,sizeof(sin));
			if (ret != SOCKET_ERROR)
			{
				char szPort[10] = {0};
				sprintf(szPort, "%u", g_sock_port);
			}
			Sleep(10);
		}
	}

	printf("Socket bind port %u ok...\r\n", g_sock_port);

	write_log(JUDGE_INFO,"Socket bind port %u ok...", g_sock_port);

	//进入监听状态
	int trylisten=50; //重试listen次数
	while((ret=listen(g_sListen,20))==SOCKET_ERROR&&trylisten)
	{
		write_log(JUDGE_SYSTEM_ERROR,"listen failed:%d , it will try later..", geterror());
		trylisten--;
		Sleep(100);
	}

	if(ret<0)
	{
		write_log(JUDGE_SYSTEM_ERROR,"Listen failed...");
		printf("Error: Listen port(%u) failed......[code:%u]\r\n", g_sock_port, geterror());

		closesocket(g_sListen);

#ifdef _WIN32_		
		WSACleanup();
#endif
		return 0;
	}

	printf("Socket listen ok...\r\n");
	write_log(JUDGE_INFO,"Socket listen ok...");

	return 1;
}

void SYSMNG_DestroySocket()
{
	closesocket(g_sListen);

#ifdef _WIN32_	
	WSACleanup();
#endif

	printf("Socket close ok...\r\n");
}

void SYSMNG_InitConfigData()
{
	extern int g_judge_mode;
	extern int isDeleteTemp;
	extern DWORD OutputLimit;
	extern int JUDGE_LOG_BUF_SIZE;
	extern int g_vjudge_enable;
	extern int isRestrictedFunction;
	extern char workPath[MAX_PATH];
	extern char dataPath[MAX_PATH];
	extern char judgeLogPath[MAX_PATH];
	extern char Mysql_url[255];
	extern char Mysql_username[255];
	extern char Mysql_password[255];
	extern char Mysql_table[255];
	extern int  Mysql_port;
	extern char hdu_domain[256];
	extern char hdu_username[1000];
	extern char hdu_password[1000];
	extern char hdu_judgerIP[20];
	extern int hdu_sockport;
	extern int hdu_remote_enable;
	extern int hdu_vjudge_enable;

#if (OS_YES == OSP_MODULE_JUDGE)
	
	if( (file_access(INI_filename, 0 )) == -1 )
	{
		printf("\r\nWarnning: config.ini path '%s' is not exist, please check.", INI_filename);
		write_log(JUDGE_ERROR,"config.ini path '%s' is not exist, please check.", dataPath);
	}

	util_ini_get_string("System","startup_config","config.cfg",g_startup_config,sizeof(g_startup_config),INI_filename);
	util_ini_get_string("System","sysname","Judge-Kernel",g_sysname,sizeof(g_sysname),INI_filename);
	g_sock_port=util_ini_get_int("System","sock_port",SOCKET_PORT,INI_filename);
	util_ini_get_string("System","JudgePath","",judgePath,sizeof(judgePath),INI_filename);

	g_judge_mode=util_ini_get_int("Judge","judge_mode",0,INI_filename);
	isDeleteTemp=util_ini_get_int("Judge","DeleteTemp",0,INI_filename);
	OutputLimit=util_ini_get_int("Judge","OutputLimit",10000,INI_filename);
	JUDGE_LOG_BUF_SIZE=util_ini_get_int("Judge","judge_logbuf_size",500,INI_filename);
	g_vjudge_enable=util_ini_get_int("Judge","vjudge_enable",OS_NO,INI_filename);
	isRestrictedFunction=util_ini_get_int("Judge","isRestrictedFunction",0,INI_filename);
	util_ini_get_string("Judge","WorkingPath","",workPath,sizeof(workPath),INI_filename);
	util_ini_get_string("Judge","DataPath","data\/",dataPath,sizeof(dataPath),INI_filename);

    if( (file_access(dataPath, 0 )) == -1 )
    {
    	printf("\r\nWarnning: Data path '%s' is not exist, please check.", dataPath);
    	write_log(JUDGE_ERROR,"Judge data path '%s' is not exist, please check.", dataPath);
    }
    
	util_ini_get_string("Judge","JudgeLogPath","",judgeLogPath,sizeof(judgeLogPath),INI_filename);

	util_ini_get_string("MySQL","url","",Mysql_url,sizeof(Mysql_url),INI_filename);
	util_ini_get_string("MySQL","username","NULL",Mysql_username,sizeof(Mysql_username),INI_filename);
	util_ini_get_string("MySQL","password","NULL",Mysql_password,sizeof(Mysql_password),INI_filename);
	util_ini_get_string("MySQL","table","",Mysql_table,sizeof(Mysql_table),INI_filename);
	Mysql_port=util_ini_get_int("MySQL","port",0,INI_filename);

	/* BEGIN: Added by weizengke,for hdu-vjudge*/
    util_ini_get_string("HDU","domain","http://acm.hdu.edu.cn",hdu_domain,sizeof(hdu_domain),INI_filename);
	util_ini_get_string("HDU","username","",hdu_username,sizeof(hdu_username),INI_filename);
	util_ini_get_string("HDU","password","",hdu_password,sizeof(hdu_password),INI_filename);
	util_ini_get_string("HDU","judgerIP","127.0.0.1",hdu_judgerIP,sizeof(hdu_judgerIP),INI_filename);
	hdu_sockport=util_ini_get_int("HDU","sock_port",SOCKET_PORT,INI_filename);
	hdu_remote_enable=util_ini_get_int("HDU","remote_enable",OS_NO,INI_filename);
	hdu_vjudge_enable=util_ini_get_int("HDU","vjudge_enable",OS_NO,INI_filename);

#if 0
	/* BEGIN: Added by weizengke, for guet-dept3-vjudge */
	GetPrivateProfileString("GUET_DEPT3","username","NULL",guet_username,sizeof(guet_username),INI_filename);
	GetPrivateProfileString("GUET_DEPT3","password","NULL",guet_password,sizeof(guet_password),INI_filename);
	GetPrivateProfileString("GUET_DEPT3","judgerIP","127.0.0.1",guet_judgerIP,sizeof(guet_judgerIP),INI_filename);
	guet_sockport=GetPrivateProfileInt("GUET_DEPT3","sock_port",7706,INI_filename);
	guet_remote_enable=GetPrivateProfileInt("GUET_DEPT3","remote_enable",OS_NO,INI_filename);
	guet_vjudge_enable=GetPrivateProfileInt("GUET_DEPT3","vjudge_enable",OS_NO,INI_filename);
#endif

	write_log(JUDGE_INFO,"DataPath:%s, Workpath:%s",dataPath,workPath);
	write_log(JUDGE_INFO,"MySQL:%s %s %s %s %d",Mysql_url,Mysql_username,Mysql_password,Mysql_table,Mysql_port);
#endif

}



int SYSMNG_TLVMsgProcess(USHORT usType, USHORT usLen, CHAR *pszBuf)
{
	switch(usType)
	{
		case SYSMNG_MSG_TYPE_CMD:
			{
				/* 执行命令 */
				(void)cmd_pub_run("system-view");
				(void)cmd_pub_run(pszBuf);
			}
			break;
		default:
			break;
	}

	return OS_OK;
}

int SYSMNG_PacketParse(char *pszCmd, int len)
{
	CHAR szMagic[9] = {0};
	CHAR szType[5] = {0};
	CHAR szLen[5] = {0};
	ULONG ulMagic = 0;
	USHORT usType = 0;
	USHORT usLen = 0;
	CHAR *pszBuf = NULL;

	SYSMNG_Debug(DEBUG_TYPE_FUNC, "SYSMNG_PacketParse: len=%u, pszCmd=%s", len, pszCmd);
	
	/* 检查长度合法性 */
	if (len < 8 + 4 + 4)
	{
		return OS_ERR;
	}
	
	/* magic Num: 0xabcddcba
	   TLV:
	   T: 0x0001, cmd
	   
	*/

	/* 检查魔术字 0xabcddcba */	
	(void)strncpy(szMagic, pszCmd, 8);
	ulMagic = util_strtol(szMagic, 16);
	if (SYSMNG_MSG_MAGIC_NUM != ulMagic)
	{
		SYSMNG_Debug(DEBUG_TYPE_ERROR, "SYSMNG_PacketParse: magic num is invalid. (ulMagic=%x)", ulMagic);
		return OS_ERR;
	}

	/* 获取报文类型 */	
	(void)strncpy(szType, pszCmd + 8, 4);
	usType = util_strtol(szType, 16);
	
	/* 获取报文数据区长度 */	
	(void)strncpy(szLen, pszCmd + 8 + 4, 4);
	usLen = util_strtol(szLen, 16);
	
	/* 检查长度合法性 */
	if (usLen != len - 8 - 4 - 4)
	{
		SYSMNG_Debug(DEBUG_TYPE_ERROR, "SYSMNG_PacketParse: usLen is invalid. (usLen=%u)", usLen);
		return OS_ERR;
	}

	/* 获取报文数据内容 */
	pszBuf = (char*)malloc(usLen + 1);
	if (NULL == pszBuf)
	{
		return OS_ERR;
	}
	(void)memset(pszBuf, 0, usLen + 1);
	(void)strncpy(pszBuf, pszCmd + 8 + 4 + 4, usLen);

	SYSMNG_Debug(DEBUG_TYPE_FUNC, "SYSMNG_PacketParse: usType=0x%x, usLen=0x%x, pszBuf=%s", usType, usLen, pszBuf);
	
	(VOID)SYSMNG_TLVMsgProcess(usType, usLen, pszBuf);

	free(pszBuf);
	pszBuf = NULL;

	return OS_OK;
}


ULONG SYSMNG_CfgCallback(VOID *pRcvMsg)
{
	ULONG ulRet = OS_OK;
	ULONG iLoop = 0;
	ULONG iElemNum = 0;
	ULONG iElemID = 0;
	VOID *pElem = NULL;
	ULONG vtyId = 0;
	ULONG isVersion = 0;
	ULONG isSysname = 0;
	ULONG isDir = 0;
	
	vtyId = cmd_get_vty_id(pRcvMsg);
	iElemNum = cmd_get_elem_num(pRcvMsg);

	for (iLoop = 0; iLoop < iElemNum; iLoop++)
	{	
		pElem = cmd_get_elem_by_index(pRcvMsg, iLoop);
		iElemID = cmd_get_elemid(pElem);

		switch(iElemID)
		{
			case SYSMNG_CMO_VERSION:
				isVersion = OS_YES;
				break;
				
			case SYSMNG_CMO_SYSNAME_STRING:
				cmd_copy_string_param(pElem, g_sysname);
				isSysname = OS_YES;
				break;

			case SYSMNG_CMO_DIR:
				isDir = OS_YES;
				break;	
				
			default:
				break;
		}
	}
	
	if (OS_YES == isVersion)
	{
		vty_printf(vtyId, 
			" Kernel version %u.%u.%u, released at %s %s. \r\n Copyright @ 2011-2018 debugforces.com. All Rights Reserved. \r\n",
			OS_VERSION_MAJOR, OS_VERSION_MINOR, OS_VERSION_PATCH,
			__TIME__,
			__DATE__);
		
		return OS_OK;
	}

#if 0
	if (OS_YES == isSysname)
	{
		::SetConsoleTitle(g_sysname);
		return OS_OK;
	}	
#endif	

	if (OS_YES == isDir)
	{
		string strinfo;		
		(VOID)util_get_directory_info(".\/", strinfo);
	
		vty_printf(vtyId, " Directory of %s:\r\n%s", ".\/", strinfo.c_str());
		
		return OS_OK;
	}

	return ulRet;

}

ULONG SYSMNG_CmdCallback(VOID *pRcvMsg)
{
	ULONG iRet = 0;
	ULONG iTBLID = 0;
	
	iTBLID = cmd_get_first_elem_tblid(pRcvMsg);
		
	switch(iTBLID)
	{
		case SYSMNG_CMO_TBL:
			iRet = SYSMNG_CfgCallback(pRcvMsg);
			break;

		default:
			break;
	}

	return iRet;
}

ULONG SYSMNG_RegCmd()
{
	CMD_VECTOR_S * vec = NULL;

	/* 命令行注册四部曲1: 申请命令行向量 */
	CMD_VECTOR_NEW(vec);

	/* 命令行注册四部曲2: 定义命令字 */
	cmd_regelement_new(SYSMNG_CMO_VERSION,				CMD_ELEM_TYPE_KEY,	  "version",		   "Version", vec);
	cmd_regelement_new(CMD_ELEMID_NULL,					CMD_ELEM_TYPE_KEY,	  "sysname",		   "Change System Name", vec);
	cmd_regelement_new(SYSMNG_CMO_SYSNAME_STRING,			CMD_ELEM_TYPE_STRING, "STRING<1-24>",	   "System Name",   vec);
	cmd_regelement_new(SYSMNG_CMO_DIR,					CMD_ELEM_TYPE_KEY,	  "directory",		   "Directory", vec);
	
	/* 命令行注册四部曲3: 注册命令行 */
	cmd_install_command(MID_SYSMNG, VIEW_DIAGNOSE,  " 1 ", vec);
	cmd_install_command(MID_SYSMNG, VIEW_SYSTEM,    " 2 3 ", vec);
	cmd_install_command(MID_SYSMNG, VIEW_USER,  	  " 4 ", vec);
	
	/* 命令行注册四部曲4: 释放命令行向量 */
	CMD_VECTOR_FREE(vec);

	return OS_OK;
}

ULONG SYSMNG_BuildRun(CHAR **ppBuildrun, ULONG ulIncludeDefault)
{
	CHAR *pBuildrun = NULL;

	*ppBuildrun = (CHAR*)malloc(BDN_MAX_BUILDRUN_SIZE);
	if (NULL == *ppBuildrun)
	{
		return OS_ERR;
	}
	memset(*ppBuildrun, 0, BDN_MAX_BUILDRUN_SIZE);
	
	pBuildrun = *ppBuildrun;

	pBuildrun += sprintf(pBuildrun, "#version %u.%u.%u", OS_VERSION_MAJOR, OS_VERSION_MINOR, OS_VERSION_PATCH);
	pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"sysname %s", g_sysname);

	return OS_OK;
}

int SYSMNG_Init()
{
	#ifdef _WIN32_ 
	SetUnhandledExceptionFilter(SYSMNG_ExceptionFilter);
 	SetErrorMode(SEM_NOGPFAULTERRORBOX );
	#endif

	#ifdef _LINUX_
	//install_segv_handler();
	#endif
	
	extern char logPath[MAX_PATH];
	if( (file_access(logPath, 0 )) == -1 )
	{
		create_directory(logPath);
	}

	(VOID)SYSMNG_InitConfigData();

	(VOID)SYSMNG_InitWindows();

	(VOID)SYSMNG_InitSocket();

	return OS_OK;
}

int SYSMNG_TaskEntry(void *pEntry)
{
	ULONG ulRet = OS_OK;

	(VOID)SYSMNG_RegCmd();
	
	(VOID)cmd_regcallback(MID_SYSMNG, SYSMNG_CmdCallback);	

	(VOID)BDN_RegistBuildRun(MID_SYSMNG, VIEW_SYSTEM, BDN_PRIORITY_HIGH + 4094, SYSMNG_BuildRun);

	(VOID)thread_create(SYSMNG_ListenThread, NULL);
	
	/* 循环读取消息队列 */
	for(;;)
	{
		/* 放权 */
		Sleep(1000);
	}
	
}

APP_INFO_S g_SysmngAppInfo =
{
	MID_SYSMNG,
	"Sysmng",
	SYSMNG_Init,
	SYSMNG_TaskEntry
};

void SYSMNG_RegAppInfo()
{
	APP_RegistInfo(&g_SysmngAppInfo);
}

