#ifndef _PDT_COMMON_INC_
#define _PDT_COMMON_INC_

#define OS_OK   0
#define OS_ERR  1

#define OS_TRUE 1
#define OS_FALSE 0

#define OS_YES 1
#define OS_NO  0


/* os version */
#define OS_VERSION_MAJOR 1
#define OS_VERSION_MINOR 1
#define OS_VERSION_PATCH 320

#define STARTUP_CFG "conf\/config.ini"

/* 模块id定义 */
enum MID_ID_EM
{
	MID_NULL = 0,
	MID_OS = 1,
	MID_SYSMNG,
	MID_JUDGE,
	MID_SQL,
	MID_DEBUG,
	MID_CMD,
	MID_EVENT,
	MID_NDP,
	MID_AAA,
	MID_TELNET,
	MID_FTP,

	/* 同步修改模块名数组: char *szModuleName[32] */
	MID_ID_END
};

enum CFG_SECTION_ID_EM
{
	CFG_SECTION_NULL = 0,
	CFG_SECTION_GLOBAL = 1,
	CFG_SECTION_USER,
	CFG_SECTION_SYSTEM,
	CFG_SECTION_JUDGE_MGR,
	CFG_SECTION_VJUDGE_MGR,
	CFG_SECTION_AAA,
	CFG_SECTION_USER_VTY,
	CFG_SECTION_DIAGNOSE,
};

/* feature switch */
#ifdef WIN32
#define OSP_MODULE_JUDGE 			OS_YES
#define OSP_MODULE_JUDGE_LOCAL   	OS_YES
#define OSP_MODULE_JUDGE_VJUDGE  	OS_YES
#define OSP_MODULE_JUDGE_OI 		OS_YES
#define OSP_MODULE_NDP				OS_YES
#define OSP_MODULE_FTPS 		 	OS_YES
#define OSP_MODULE_TELNETS 			OS_YES
#define OSP_MODULE_TELNETC 			OS_YES
#define OSP_MODULE_DEBUG 			OS_YES
#define OSP_MODULE_AAA				OS_YES
#define OSP_MODULE_CLI 				OS_YES
#endif

#ifdef _LINUX_
#define OSP_MODULE_JUDGE 			OS_NO
#define OSP_MODULE_JUDGE_LOCAL   	OS_NO
#define OSP_MODULE_JUDGE_VJUDGE  	OS_NO
#define OSP_MODULE_JUDGE_OI 		OS_NO
#define OSP_MODULE_NDP				OS_YES
#define OSP_MODULE_FTPS 		 	OS_YES
#define OSP_MODULE_TELNETS 			OS_YES
#define OSP_MODULE_TELNETC 			OS_YES
#define OSP_MODULE_DEBUG 			OS_YES
#define OSP_MODULE_AAA				OS_YES
#define OSP_MODULE_CLI 				OS_YES
#endif



extern void RunDelay(int t);
extern void MSGQueueMain();

extern void MSG_StartDot();
extern void MSG_StopDot();


extern char g_judge_ini_cfg_path[];

#define PDT_Debug(x, format, ...) debugcenter_print(MID_OS, x, format, ##__VA_ARGS__)


#endif _PDT_COMMON_INC_

