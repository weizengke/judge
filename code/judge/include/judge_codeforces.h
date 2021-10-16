#ifndef _JUDGE_CODEFORCES_H_
#define _JUDGE_CODEFORCES_H_

extern char codeforces_username[128];
extern char codeforces_password[128];
extern char codeforces_judgerIP[20];
extern int codeforces_sockport;
extern int codeforces_remote_enable;
extern int codeforces_vjudge_enable;

extern int codeforces_vjudge(JUDGE_SUBMISSION_S *pstJudgeSubmission);


#endif
