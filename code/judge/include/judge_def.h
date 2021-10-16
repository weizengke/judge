#ifndef _JUDGE_DEF_H_
#define _JUDGE_DEF_H_


#define JUDGE_VIRTUAL 		VOS_YES   /* VJUDGE switch */
#define VJUDGE_CURL   		VOS_YES   /* VJUDGE switch */
#define FEATURE_JUDGE_OI    VOS_YES   /* OI switch */

#define STD_KB 1024
#define STD_MB 1048576
#define STD_T_LIM 2
#define STD_F_LIM (STD_MB<<5)
#define STD_M_LIM (STD_MB<<7)
#define BUFFER_SIZE 512


#define V_ALL 0
#define V_Q 1
#define V_C 2
#define V_CE 3
#define V_RUN 4
#define V_AC 5
#define V_WA 6
#define V_RE 7
#define V_TLE 8
#define V_MLE 9
#define V_PE 10
#define V_OLE 11
#define V_RF 12
#define V_OOC 13
#define V_SE 14
#define V_SK 15

#define JUDGE_ISJUDGE_STOP(v) (v == V_CE || v == V_RE || v == V_TLE || v == V_MLE || v == V_SE)

static const char* VERDICT_NAME[] = {"ALL", "Queuing", "Compiling", "Compilation Error", "Running","Accepted",
									"Wrong Answer","Runtime Error","Time Limit Exceeded",
									"Memory Limit Exceeded","Presentation Error","Output Limit Exceeded",
									"Restricted Function","Out Of Comtest","System Error", "Skipped"};


#define VOS_YES 1
#define VOS_NO  0


#define JUDGE_DEBUG_OFF 0
#define JUDGE_DEBUG_ON 1

#define JSONBUFSIZE 65535

const int MAX_NAME = 32;
const int MAX_CODE = 100000;
const int MAX_TITLE = 200;
const int MAX_CONTENT = 100000;
const int MAX_WAITTIME = 10000 ;

#define JUDGE_MAX_CASE 128

#define VJUDGE_MAX_SIZE_BUF 10000000
#define VJUDGE_OVECCOUNT 30    /* should be a multiple of 3 */
#define VJUDGE_MAX_LANG_SIZE 255

#define JUDGE_MODE_ACM  0
#define JUDGE_MODE_OI   1

#define JUDGE_AUTO_NUM_DEFAULT 		5
#define JUDGE_AUTO_INTERVAL_DEFAULT 10

#define JUDGE_SUBMIT_MODE 0
#define JUDGE_TEST_MODE   1

#define Judge_Debug(x, format, ...) debugcenter_print(MID_JUDGE, x, format, ##__VA_ARGS__)

#define JUDGE_FREE(p) \
{\
	if (p != NULL) {\
		free(p);\
		p = NULL;\
	}\
}

#endif
