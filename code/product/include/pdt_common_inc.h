#ifndef _PDT_COMMON_INC_
#define _PDT_COMMON_INC_

#define UCHAR unsigned char
#define ULONG unsigned long
#define CHAR char

#define OS_OK   0
#define OS_ERR  1

#define OS_TRUE 1
#define OS_FALSE 0

#define OS_YES 1
#define OS_NO  0

#define APP_NAME_SIZE 64

#define SOLFWARE_VERSION "V100R001C00B120"

#define STARTUP_CFG "conf\\config.ini"

/* 模块id定义 */
enum MID_ID_EM
{
	MID_NULL = 0,
	MID_OS = 1,
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

typedef struct tagAPP_INFO_S
{
	unsigned long taskMID;
	char taskName[APP_NAME_SIZE];
	int (*pfInitFunction)();
	unsigned _stdcall  (*pfTaskMain)(void *);

}APP_INFO_S;

extern int RegistAppInfo(APP_INFO_S *pstAppInfo);

extern void RunDelay(int t);
extern void MSGQueueMain();
extern void pdt_debug_print(const char *format, ...);

extern void MSG_StartDot();
extern void MSG_StopDot();


extern char INI_filename[];
extern HWND g_hWnd;

#define PDT_Debug(x, args...) debugcenter_print(MID_OS, x, args)


#endif _PDT_COMMON_INC_

