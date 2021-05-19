#ifndef _JUDGE_LEETCODE_H_
#define _JUDGE_LEETCODE_H_

extern char leetcode_username[128];
extern char leetcode_password[128];
extern char leetcode_judgerIP[20];
extern int leetcode_sockport;
extern int leetcode_remote_enable;
extern int leetcode_vjudge_enable;

extern int leetcode_vjudge(JUDGE_SUBMISSION_S *pstJudgeSubmission);


#endif
