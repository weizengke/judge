#ifndef _PDT_COMMON_INC_
#define _PDT_COMMON_INC_

#define UCHAR unsigned char
#define ULONG unsigned long
#define CHAR char

#define OS_OK   0
#define OS_ERR  1

#define OS_TRUE 0
#define OS_FALSE 1

#define OS_YES 1
#define OS_NO  0

#define APP_NAME_SIZE 64

#define SOLFWARE_VERSION "V100R001C00B010"

#define STARTUP_CFG "conf\\config.ini"

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
extern void pdt_debug_print_ex(int level, const char *format, ...);

extern void MSG_StartDot();
extern void MSG_StopDot();


extern char INI_filename[];
extern HWND g_hWnd;


#endif _PDT_COMMON_INC_

