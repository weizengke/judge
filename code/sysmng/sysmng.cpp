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
	
#ifdef WIN32
#include <io.h>
#include <winsock2.h>
#include <TlHelp32.h>
#endif

#include "kernel.h"

#include "osp_common_def.h"
#include "pdt_common_inc.h"
#include "icli.h"
#include "root.h"
#include "util/util.h"
#include "judge/include/judge_inc.h"
#include "sysmng/sysmng.h"
#include "stack_walker/stackwalker.h"

using namespace std;

#define SOCKET_PORT 5000

char g_sysname[APP_MAX_SYSNAME_SIZE] = "judger";
char g_startup_config[256] = "config.cfg";
int  g_cfm_recovered = OS_NO;
int g_cli_north_intf_enable = OS_YES;
socket_t g_sListen = INVALID_SOCKET;
int g_sock_port = SOCKET_PORT;
char judgePath[MAX_PATH];
int g_sysmngListenRuning = OS_NO;

#define SYSMNG_Debug(x, format, ...) debugcenter_print(MID_SYSMNG, x, format, ##__VA_ARGS__)

char *SYSMNG_GetSysname()
{
	return g_sysname;
}

VOID SYSMNG_InitWindows()
{
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


#ifdef WIN32

#ifdef _MSC_VER
#if 0
void dumpStack(void)
{
	const UINT max_name_length = 256;	// Max length of symbols' name.
 
	CONTEXT context;			// Store register addresses.
	STACKFRAME64 stackframe;		// Call stack.
	HANDLE process, thread;			// Handle to current process & thread.
						// Generally it can be subsitituted with 0xFFFFFFFF & 0xFFFFFFFE.
	PSYMBOL_INFO symbol;			// Debugging symbol's information.
	IMAGEHLP_LINE64 source_info;		// Source information (file name & line number)
	DWORD displacement;			// Source line displacement.
 
	// Initialize PSYMBOL_INFO structure.
	// Allocate a properly-sized block.
	symbol = (PSYMBOL_INFO)malloc(sizeof(SYMBOL_INFO) + (max_name_length - 1) * sizeof(TCHAR));	
	memset(symbol, 0, sizeof(SYMBOL_INFO) + (max_name_length - 1) * sizeof(TCHAR));
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);	// SizeOfStruct *MUST BE* set to sizeof(SYMBOL_INFO).
	symbol->MaxNameLen = max_name_length;
 
	// Initialize IMAGEHLP_LINE64 structure.
	memset(&source_info, 0, sizeof(IMAGEHLP_LINE64));
	source_info.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
	
	// Initialize STACKFRAME64 structure.
	RtlCaptureContext(&context);			// Get context.
	memset(&stackframe, 0, sizeof(STACKFRAME64));
	stackframe.AddrPC.Offset = context.Eip;		// Fill in register addresses (EIP, ESP, EBP).
	stackframe.AddrPC.Mode = AddrModeFlat;
	stackframe.AddrStack.Offset = context.Esp;
	stackframe.AddrStack.Mode = AddrModeFlat;
	stackframe.AddrFrame.Offset = context.Ebp;
	stackframe.AddrFrame.Mode = AddrModeFlat;
 
	process = GetCurrentProcess();	// Get current process & thread.
	thread = GetCurrentThread();
 
	// Initialize dbghelp library.
	if(!SymInitialize(process, NULL, TRUE))
		return ;
 
	puts("Call stack: \r\n");
 
	// Enumerate call stack frame.
	while(StackWalk64(IMAGE_FILE_MACHINE_I386, process, thread, &stackframe, 
		&context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
	{
		if(stackframe.AddrFrame.Offset == 0)	// End reaches.
			break;
		
		if(SymFromAddr(process, stackframe.AddrPC.Offset, NULL, symbol))// Get symbol.
			printf(" > %s\n", symbol->Name);
 
		if(SymGetLineFromAddr64(process, stackframe.AddrPC.Offset, 
			&displacement, &source_info)) {				// Get source information.
				printf("\t[%s:%d] at addr 0x%08LX\n", 
					source_info.FileName, 
					source_info.LineNumber,
					stackframe.AddrPC.Offset);
		} else {
			if(GetLastError() == 0x1E7) {		// If err_code == 0x1e7, no symbol was found.
				printf("\tNo debug symbol loaded for this function.\n");
			}
		}
	}
 
	SymCleanup(process);	// Clean up and exit.
	free(symbol);
}
#endif
HANDLE g_hHandle;
CONTEXT g_context;
HANDLE g_hThread;
	
void InitTrack()
{
    g_hHandle = GetCurrentProcess();
    SymInitialize(g_hHandle, NULL, TRUE);
}

void StackTrack()
{ 
	int len = 0;
	char buff[4096] = {0};
	
	g_hThread = GetCurrentThread();
	STACKFRAME sf = { 0 };
	sf.AddrPC.Offset = g_context.Eip;
	sf.AddrPC.Mode = AddrModeFlat;
	sf.AddrFrame.Offset = g_context.Ebp;
	sf.AddrFrame.Mode = AddrModeFlat;    
	sf.AddrStack.Offset = g_context.Esp;
	sf.AddrStack.Mode = AddrModeFlat;  
	
	typedef struct tag_SYMBOL_INFO 
	{        
		IMAGEHLP_SYMBOL symInfo;        
		TCHAR szBuffer[MAX_PATH];   
	} SYMBOL_INFO, *LPSYMBOL_INFO;
	
	DWORD dwDisplament = 0;   
	SYMBOL_INFO stack_info = { 0 };
	PIMAGEHLP_SYMBOL pSym = (PIMAGEHLP_SYMBOL)&stack_info; 
	pSym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);   
	pSym->MaxNameLength = sizeof(SYMBOL_INFO) - offsetof(SYMBOL_INFO, symInfo.Name);
	IMAGEHLP_LINE ImageLine = { 0 };    
	ImageLine.SizeOfStruct = sizeof(IMAGEHLP_LINE);

	len += sprintf(buff + len, "\r\ncallstack:");
		
	while (StackWalk(IMAGE_FILE_MACHINE_I386, g_hHandle, g_hThread, &sf, &g_context, NULL, SymFunctionTableAccess, SymGetModuleBase, NULL))
	{       
			SymGetSymFromAddr(g_hHandle, sf.AddrPC.Offset, &dwDisplament, pSym); 
			SymGetLineFromAddr(g_hHandle, sf.AddrPC.Offset, &dwDisplament, &ImageLine);  

			//printf("> %08x+%s(FILE[%s]LINE[%d])\n", pSym->Address, pSym->Name, ImageLine.FileName, ImageLine.LineNumber);

			len += sprintf(buff + len, "\r\n> %08x+%s(FILE[%s]LINE[%d])",
							pSym->Address, pSym->Name, ImageLine.FileName, ImageLine.LineNumber);
	}

	write_log(JUDGE_INFO,"%s", buff);

	return;
}

void UninitTrack()
{
    SymCleanup(g_hHandle);
}

#if 0
#define OPEN_STACK_TRACK\
	HANDLE hThread = GetCurrentThread();\
	GetThreadContext(hThread, &g_context);\
	__asm{call $ + 5}\
	__asm{pop eax}\
	__asm{mov g_context.Eip, eax}\
	__asm{mov g_context.Ebp, ebp}\
	__asm{mov g_context.Esp, esp}\
	InitTrack();\
	StackTrack();\
	UninitTrack();
#else
#define OPEN_STACK_TRACK
#endif

#endif

long WINAPI SYSMNG_ExceptionFilter(EXCEPTION_POINTERS * excp)
{
	int err = GetLastError();

	printf("\r\nWarning: An exceptions happened, please check the system logfile. (ErrorCode=%u)", err);
	
	write_log(JUDGE_ERROR,"Warning: An exceptions happened, please check the system logfile. (ErrorCode=%u)", err);

#ifdef _MSC_VER

	OPEN_STACK_TRACK

	//dumpStack();

#endif	

	Sleep(5000);

	closesocket(g_sListen);
	g_sListen = INVALID_SOCKET;

#if (OS_YES == OSP_MODULE_TELNETS)
	extern socket_t g_TelnetSSocket;
	closesocket(g_TelnetSSocket);
#endif

#if (OS_YES == OSP_MODULE_TELNETS)
	extern socket_t g_sFtpListenSocket;
	closesocket(g_sFtpListenSocket);
#endif

	#ifdef WIN32
	WSACleanup();
    #endif
	
    ShellExecuteA(NULL,"open","judger.exe",NULL,NULL,SW_SHOWNORMAL);

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
int SYSMNG_AcceptThread(void *pEntry)
{
	int len = 102400;
	char *buff = (char*)malloc(len + 1);
	if (buff == NULL) {
		write_log(JUDGE_ERROR, "SYSMNG_AcceptThread malloc error");
		return 0;
	}
	memset(buff, 0, len + 1);

	socket_t sClient = *(socket_t*)pEntry;

	int recvTimeout = 10 * 1000;  //30s
	setsockopt(sClient, SOL_SOCKET, SO_RCVTIMEO, (char *)&recvTimeout, sizeof(int));
	//setsockopt(connSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&sendTimeout, sizeof(int));

	int ret = recv(sClient, (char*)buff, len, 0);
	if (ret > 0) {
		/* 需要检查发送源的合法性 */
		(VOID)SYSMNG_PacketParse(buff, strlen(buff));
	} else {
		SYSMNG_Debug(DEBUG_TYPE_ERROR, "recv len %d invalid.", ret);
	}

	free(buff);

	return 0;
}

int SYSMNG_ListenThread(void *pEntry)
{
	sockaddr_in remoteAddr;
	socklen_t nAddrLen = sizeof(remoteAddr);

	g_sysmngListenRuning = OS_YES;
	while(g_sysmngListenRuning) {
		socket_t sClient = accept(g_sListen, (SOCKADDR*)&remoteAddr, &nAddrLen);
		if (sClient == INVALID_SOCKET) {
			write_log(JUDGE_ERROR, "SYSMNG_ListenThread accept error. ");
			break;
		}

		SYSMNG_Debug(DEBUG_TYPE_MSG, "Accept connect from (ip:%s, port:%u)", 
					 inet_ntoa(remoteAddr.sin_addr), htons(remoteAddr.sin_port));
		
		thread_create(SYSMNG_AcceptThread, (void*)&sClient);

		Sleep(1);
	}

	Sleep(2000);

	return 0;
}

int SYSMNG_IsSocketActive()
{
	socket_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock == INVALID_SOCKET) {
		write_log(JUDGE_ERROR, "SYSMNG_IsSocketActive socket error");
		return OS_ERR;
	}

	sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(g_sock_port);
	servAddr.sin_addr.s_addr =inet_addr("127.0.0.1");
	if(connect(sock,(sockaddr*)&servAddr,sizeof(servAddr)) == SOCKET_ERROR) {
		write_log(JUDGE_ERROR, "Sysmng socket port %d is not active.", g_sock_port);
		closesocket(sock);
		return OS_ERR;
	}

	closesocket(sock);
	SYSMNG_Debug(DEBUG_TYPE_FUNC, "Sysmng socket port %d is active", g_sock_port);

	return OS_OK;
}

int SYSMNG_TimerThread(void *pEntry)
{
	while (TRUE) {
		if (SYSMNG_IsSocketActive() != OS_OK) {
			SYSMNG_PacketRecvRun();
		}

		Sleep(10000);
	}

	return 0;
}

int SYSMNG_InitSocket()
{
	write_log(JUDGE_INFO,"Start initialization of Socket...");

#ifdef WIN32
	WSADATA wsaData;
    WORD sockVersion = MAKEWORD(2, 2);
	if(WSAStartup(sockVersion, &wsaData) != 0) {
		return 0;
	}
#endif

	g_sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(g_sListen == INVALID_SOCKET) {
		write_log(JUDGE_SYSTEM_ERROR,"create socket error");
		return 0;
	}

    int useAddr = 1;
    if(setsockopt(g_sListen, SOL_SOCKET, SO_REUSEADDR, (const char*)&useAddr, sizeof(int)) < 0) {
		write_log(JUDGE_SYSTEM_ERROR,"setsockopt socket error");
    }

	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(g_sock_port);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	int trybind=50;
	int ret=0;

	ret = bind(g_sListen,(const sockaddr*)&sin,sizeof(sin));
	while(ret == SOCKET_ERROR && trybind > 0) {
		ret = bind(g_sListen,(const sockaddr*)&sin,sizeof(sin));
		//write_log(JUDGE_SYSTEM_ERROR,"bind failed:%d , it will try later...",WSAGetLastError());
		trybind--;
		Sleep(100);
	}

#if 0
	if(ret<0) {
		while (ret == SOCKET_ERROR) {
			g_sock_port++;
			sin.sin_port = htons(g_sock_port);
			ret =  bind(g_sListen,(const sockaddr*)&sin,sizeof(sin));
			if (ret != SOCKET_ERROR) {
				char szPort[10] = {0};
				sprintf(szPort, "%u", g_sock_port);
			}
			Sleep(10);
		}
	}
#endif

	printf("Socket bind port %u ok...\r\n", g_sock_port);
	write_log(JUDGE_INFO,"Socket bind port %u ok...", g_sock_port);

	int trylisten=50;
	while((ret=listen(g_sListen, 20))==SOCKET_ERROR&&trylisten) {
		write_log(JUDGE_SYSTEM_ERROR,"listen failed:%d , it will try later..", geterror());
		trylisten--;
		Sleep(100);
	}

	if(ret<0) {
		write_log(JUDGE_SYSTEM_ERROR,"Listen failed...");
		printf("Error: Listen port(%u) failed......[code:%u]\r\n", g_sock_port, geterror());

		closesocket(g_sListen);
		g_sListen = INVALID_SOCKET;

#ifdef WIN32		
		WSACleanup();
#endif
		return 0;
	}

	printf("Socket port %u listen ok...\r\n", g_sock_port);
	write_log(JUDGE_INFO,"Socket port %u listen ok...", g_sock_port);

	return 1;
}

void SYSMNG_DestroySocket()
{
	if (g_sListen != INVALID_SOCKET) {
		closesocket(g_sListen);
		g_sListen = INVALID_SOCKET;

		printf("Socket close ok...\r\n");
		write_log(JUDGE_INFO, "Socket close ok...");		
	}

#ifdef WIN32	
	WSACleanup();
#endif
}

void SYSMNG_CloseLisentThread()
{
	if (g_sysmngListenRuning == OS_YES) {
		g_sysmngListenRuning = OS_NO;

		printf("Lisent thread close ok...\r\n");
		write_log(JUDGE_INFO, "Lisent thread close ok...\r\n");		
	}
}

int SYSMNG_PacketRecvRun()
{
	SYSMNG_CloseLisentThread();
	SYSMNG_DestroySocket();

	(VOID)SYSMNG_InitSocket();
	
	thread_create(SYSMNG_ListenThread, NULL);
	
	return OS_OK;
}

void SYSMNG_InitConfigData()
{
	extern int g_judge_mode;
	extern int g_judge_log_buffsize;
	extern int g_vjudge_enable;
	extern char g_judge_work_path[MAX_PATH];
	extern char g_judge_testcase_path[MAX_PATH];
	extern char g_judge_log_path[MAX_PATH];
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
	
	if( (file_access(g_judge_ini_cfg_path, 0 )) == -1 )
	{
		printf("\r\nWarnning: config.ini path '%s' is not exist, please check.", g_judge_ini_cfg_path);
		write_log(JUDGE_ERROR,"config.ini path '%s' is not exist, please check.", g_judge_testcase_path);
	}

	util_ini_get_string("System","startup_config","config.cfg",g_startup_config,sizeof(g_startup_config),g_judge_ini_cfg_path);
	util_ini_get_string("System","sysname","Judge-Kernel",g_sysname,sizeof(g_sysname),g_judge_ini_cfg_path);
	g_sock_port=util_ini_get_int("System","sock_port",SOCKET_PORT,g_judge_ini_cfg_path);
	util_ini_get_string("System","JudgePath","",judgePath,sizeof(judgePath),g_judge_ini_cfg_path);

	g_judge_mode=util_ini_get_int("Judge","judge_mode",0,g_judge_ini_cfg_path);
	g_judge_log_buffsize=util_ini_get_int("Judge","judge_logbuf_size",500,g_judge_ini_cfg_path);
	g_vjudge_enable=util_ini_get_int("Judge","vjudge_enable",OS_NO,g_judge_ini_cfg_path);

	util_ini_get_string("Judge","WorkingPath","", g_judge_work_path, sizeof(g_judge_work_path),g_judge_ini_cfg_path);
	util_ini_get_string("Judge","DataPath","data\/",g_judge_testcase_path,sizeof(g_judge_testcase_path),g_judge_ini_cfg_path);

    if( (file_access(g_judge_testcase_path, 0 )) == -1 ) {
    	printf("\r\nWarnning: Data path '%s' is not exist, please check.", g_judge_testcase_path);
    	write_log(JUDGE_ERROR,"Judge data path '%s' is not exist, please check.", g_judge_testcase_path);
    }
    
	util_ini_get_string("Judge","JudgeLogPath","",g_judge_log_path,sizeof(g_judge_log_path),g_judge_ini_cfg_path);

	util_ini_get_string("MySQL","url","",Mysql_url,sizeof(Mysql_url),g_judge_ini_cfg_path);
	util_ini_get_string("MySQL","username","NULL",Mysql_username,sizeof(Mysql_username),g_judge_ini_cfg_path);
	util_ini_get_string("MySQL","password","NULL",Mysql_password,sizeof(Mysql_password),g_judge_ini_cfg_path);
	util_ini_get_string("MySQL","table","",Mysql_table,sizeof(Mysql_table),g_judge_ini_cfg_path);
	Mysql_port=util_ini_get_int("MySQL","port",0,g_judge_ini_cfg_path);

	/* BEGIN: Added by weizengke,for hdu-vjudge*/
    util_ini_get_string("HDU","domain","http://acm.hdu.edu.cn",hdu_domain,sizeof(hdu_domain),g_judge_ini_cfg_path);
	util_ini_get_string("HDU","username","",hdu_username,sizeof(hdu_username),g_judge_ini_cfg_path);
	util_ini_get_string("HDU","password","",hdu_password,sizeof(hdu_password),g_judge_ini_cfg_path);
	util_ini_get_string("HDU","judgerIP","127.0.0.1",hdu_judgerIP,sizeof(hdu_judgerIP),g_judge_ini_cfg_path);
	hdu_sockport=util_ini_get_int("HDU","sock_port",SOCKET_PORT,g_judge_ini_cfg_path);
	hdu_remote_enable=util_ini_get_int("HDU","remote_enable",OS_NO,g_judge_ini_cfg_path);
	hdu_vjudge_enable=util_ini_get_int("HDU","vjudge_enable",OS_NO,g_judge_ini_cfg_path);

#if 0
	/* BEGIN: Added by weizengke, for guet-dept3-vjudge */
	GetPrivateProfileString("GUET_DEPT3","username","NULL",guet_username,sizeof(guet_username),g_judge_ini_cfg_path);
	GetPrivateProfileString("GUET_DEPT3","password","NULL",guet_password,sizeof(guet_password),g_judge_ini_cfg_path);
	GetPrivateProfileString("GUET_DEPT3","judgerIP","127.0.0.1",guet_judgerIP,sizeof(guet_judgerIP),g_judge_ini_cfg_path);
	guet_sockport=GetPrivateProfileInt("GUET_DEPT3","sock_port",7706,g_judge_ini_cfg_path);
	guet_remote_enable=GetPrivateProfileInt("GUET_DEPT3","remote_enable",OS_NO,g_judge_ini_cfg_path);
	guet_vjudge_enable=GetPrivateProfileInt("GUET_DEPT3","vjudge_enable",OS_NO,g_judge_ini_cfg_path);
#endif

	write_log(JUDGE_INFO,"DataPath:%s, Workpath:%s",g_judge_testcase_path,g_judge_work_path);
	write_log(JUDGE_INFO,"MySQL:%s %s %s %s %d",Mysql_url,Mysql_username,Mysql_password,Mysql_table,Mysql_port);
#endif

}

int SYSMNG_TLVMsgProcess(USHORT usType, USHORT usLen, CHAR *pszBuf)
{
	switch (usType) {
		case SYSMNG_MSG_TYPE_CMD: {
			if (g_cli_north_intf_enable == OS_YES) {
				/* 执行命令 */
				(void)cmd_pub_run("system-view");
				(void)cmd_pub_run(pszBuf);
			}
			break;
		}
		
		case SYSMNG_MSG_TYPE_JUDGE_REQ: {
			extern void judge_request_enqueue_test(char *jsonMsg);
			judge_request_enqueue_test(pszBuf);
			break;
		}

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

	SYSMNG_Debug(DEBUG_TYPE_MSG, "SYSMNG_PacketParse: len=%u, pszCmd=%s", len, pszCmd);
	
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
	
	/* 获取报文数据内容 */
	pszBuf = (char*)malloc(usLen * 2 + 1);
	if (NULL == pszBuf) {
		return OS_ERR;
	}
	(void)memset(pszBuf, 0, usLen * 2 + 1);
	(void)strncpy(pszBuf, pszCmd + 8 + 4 + 4, usLen * 2);

	SYSMNG_Debug(DEBUG_TYPE_MSG, "SYSMNG_PacketParse: usType=0x%x, usLen=0x%x, pszBuf=%s", usType, usLen, pszBuf);
	
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
	ULONG isUndo = OS_NO;

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

			case SYSMNG_CMO_UNDO:
				isUndo = OS_YES;
				break;	

			case SYSMNG_CMO_CLI_NORTH_INTF_ENABLE:
				g_cli_north_intf_enable = (isUndo == OS_YES)?OS_NO:OS_YES;
				break;	

			case SYSMNG_CMO_SHUTDOWN:
				extern int g_system_runing;
				if (vtyId != CMD_VTY_CONSOLE_ID) {
					vty_printf(vtyId, "Error: Only console user support shutdown command.\r\n");
					break;	
				}

				vty_printf(vtyId, "Warning: System will shutdown after 1 seconds.\r\n");
				Sleep(1000);
				g_system_runing = OS_NO;
				break;	
			default:
				break;
		}
	}
	
	if (OS_YES == isVersion) {
		vty_printf(vtyId, 
			" Kernel version %u.%u.%u, released at %s %s. \r\n Copyright @ 2011-2021 Jungle.WEI. All Rights Reserved. \r\n",
			OS_VERSION_MAJOR, OS_VERSION_MINOR, OS_VERSION_PATCH,
			__TIME__,
			__DATE__);
		return OS_OK;
	}

#if 0
	if (OS_YES == isSysname) {
		::SetConsoleTitle(g_sysname);
		return OS_OK;
	}	
#endif	

	if (OS_YES == isDir) {
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
	cmd_regelement_new(SYSMNG_CMO_SYSNAME_STRING,		CMD_ELEM_TYPE_STRING, "STRING<1-24>",	   "System Name",   vec);
	cmd_regelement_new(SYSMNG_CMO_DIR,					CMD_ELEM_TYPE_KEY,	  "directory",		   "Directory", vec);
	cmd_regelement_new(SYSMNG_CMO_UNDO,					CMD_ELEM_TYPE_KEY,	  "undo",		   	   "Undo Operation", vec);	
	cmd_regelement_new(CMD_ELEMID_NULL,					CMD_ELEM_TYPE_KEY,	  "cli",		   	   "CLI northbound interface", vec);
	cmd_regelement_new(CMD_ELEMID_NULL,					CMD_ELEM_TYPE_KEY,	  "northbound-interface","Northbound interface", vec);
	cmd_regelement_new(SYSMNG_CMO_CLI_NORTH_INTF_ENABLE,	CMD_ELEM_TYPE_KEY,	  "enable",		   	"Enable", vec);
	cmd_regelement_new(SYSMNG_CMO_SHUTDOWN,				CMD_ELEM_TYPE_KEY,	  "shutdown",		    "Shutdown system", vec);
	
	/* 命令行注册四部曲3: 注册命令行 */
	cmd_install_command(MID_SYSMNG, VIEW_DIAGNOSE,  " 1 ", vec);
	cmd_install_command(MID_SYSMNG, VIEW_SYSTEM,    " 2 3 ", vec);
	cmd_install_command(MID_SYSMNG, VIEW_USER,  	" 4 ", vec);
	cmd_install_command(MID_SYSMNG, VIEW_SYSTEM,    " 6 7 8 ", vec);
	cmd_install_command(MID_SYSMNG, VIEW_SYSTEM,    " 5 6 7 8 ", vec);
	cmd_install_command(MID_SYSMNG, VIEW_SYSTEM, 	" 9 ", vec);

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

	if (g_cli_north_intf_enable == OS_YES) {
		if (VOS_YES == ulIncludeDefault) {
			pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"cli northbound-interface enable");
		}
	} else {
		pBuildrun += sprintf(pBuildrun, BDN_BUILDRUN"undo cli northbound-interface enable");
	}

	return OS_OK;
}

int SYSMNG_Init()
{
	#ifdef WIN32 
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

	CMD_HOOK_RegCallback_GetSysname(SYSMNG_GetSysname);
	
	(VOID)SYSMNG_InitConfigData();

	(VOID)SYSMNG_InitWindows();

	return OS_OK;
}

int SYSMNG_TaskEntry(void *pEntry)
{
	ULONG ulRet = OS_OK;

	(VOID)DEBUG_PUB_RegModuleDebugs(MID_SYSMNG, "sysmng", "System management");
	
	(VOID)SYSMNG_RegCmd();
	
	(VOID)cmd_regcallback(MID_SYSMNG, SYSMNG_CmdCallback);	

	(VOID)cli_bdn_regist(MID_SYSMNG, VIEW_SYSTEM, BDN_PRIORITY_HIGH + 4094, SYSMNG_BuildRun);
	
	(VOID)thread_create(SYSMNG_TimerThread, NULL);

	/* 循环读取消息队列 */
	for(;;) {
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

