#ifndef _JUDGE_VAR_H_
#define _JUDGE_VAR_H_


extern int g_oj_debug_switch;

extern int g_judge_enable;
extern int g_vjudge_enable;
extern char g_Vjudgetfilename[MAX_PATH];

extern socket_t g_sListen;

extern int g_judge_mode ;

extern int g_judge_upload_log_ftp_enable;
extern int g_judge_upload_log_ftp_port;
extern char g_judge_upload_log_ftp_ip[128];
extern char g_judge_upload_log_ftp_user[128];
extern char g_judge_upload_log_ftp_pwd[128];

#endif
