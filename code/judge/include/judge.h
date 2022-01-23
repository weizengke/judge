#ifndef _JUDGE_H_
#define _JUDGE_H_

int judge_special_judge(JUDGE_SUBMISSION_S *pstJudgeSubmission);
int judge_get_testcases(JUDGE_SUBMISSION_S *submission, char szTestcases[JUDGE_MAX_CASE][UTIL_MAX_FNAME]);
void judge_solution_time_check(JUDGE_SUBMISSION_S *submission);
void judge_solution_memory_check(JUDGE_SUBMISSION_S *submission);
void judge_solution_exit_check(JUDGE_SUBMISSION_S *submission);
int judge_solution_run(JUDGE_SUBMISSION_S *submission);
int judge_compile_run(JUDGE_SUBMISSION_S *submission);
int judge_is_vjudge_enable();

extern char g_judge_work_path[MAX_PATH];
extern char g_judge_log_path[MAX_PATH];
extern char g_judge_apihook_path[MAX_PATH];
extern int g_judge_log_buffsize;
extern char g_judge_testcase_path[MAX_PATH];
extern int g_judge_mode;
extern int g_judge_timer_enable;
extern int g_judge_auto_detect_num;
extern int g_judge_auto_detect_interval;
extern time_t g_judge_last_judgetime;
extern int g_judge_ignore_extra_space_enable;
extern int g_judge_contest_colect_interval;
extern int g_judge_api_hook_enable;
extern queue <JUDGE_REQ_MSG_S> g_judge_queue;

extern char g_judge_proxy_url[128];
extern char g_judge_proxy_uname_passwd[128] ;

#endif