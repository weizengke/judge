#ifndef _JUDGE_GUET_H_
#define _JUDGE_GUET_H_


extern char guet_username[1000];
extern char guet_password[1000];
extern char guet_judgerIP[20];
extern int guet_sockport;
extern int guet_remote_enable;
extern int guet_vjudge_enable;

extern int GUET_VJudge(JUDGE_SUBMISSION_ST *pstJudgeSubmission);

#endif




