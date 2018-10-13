
#include "product/judge/include/judge_inc.h"

#if (OS_YES == OSP_MODULE_JUDGE)

int g_oj_debug_switch = JUDGE_DEBUG_OFF;

int g_judge_enable = OS_YES;  /* 全局使能judge */
int g_vjudge_enable = OS_NO;  /* 全局使能vjudge */


char g_Vjudgetfilename[MAX_PATH]="tmpfile.txt";

#endif
