
#include "judge/include/judge_inc.h"

#if (OS_YES == OSP_MODULE_JUDGE)

int g_oj_debug_switch = JUDGE_DEBUG_OFF;

int g_judge_enable = OS_YES;  /* ȫ��ʹ��judge */
int g_vjudge_enable = OS_NO;  /* ȫ��ʹ��vjudge */
char g_Vjudgetfilename[MAX_PATH]="tmpfile.txt";

int g_judge_upload_log_ftp_enable = OS_NO;
int g_judge_upload_log_ftp_port = 0;
char g_judge_upload_log_ftp_ip[128] = {0};
char g_judge_upload_log_ftp_user[128] = {0};
char g_judge_upload_log_ftp_pwd[128] = {0};

#endif
