#ifndef _ROOT_H_
#define _ROOT_H_

#define APP_NAME_SIZE 64

#define APP_MAX_SYSNAME_SIZE 24

typedef struct tagAPP_INFO_S
{
	unsigned long taskMID;
	char taskName[APP_NAME_SIZE];
	int (*pfInitFunction)();
	int (*pfTaskMain)(void *);

}APP_INFO_S;

extern char g_sysname[];
extern char g_startup_config[];

extern int APP_RegistInfo(APP_INFO_S *pstAppInfo);
extern int APP_Run();


#endif

