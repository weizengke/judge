#ifndef _JUDGE_VAR_H_
#define _JUDGE_VAR_H_

#include "..\..\thirdpart\mysql\include\mysql.h"


extern int g_oj_debug_switch;

extern int g_judge_enable;
extern int g_vjudge_enable;
extern char g_Vjudgetfilename[MAX_PATH];

extern SOCKET g_sListen;

extern int g_judge_mode ;

#endif
