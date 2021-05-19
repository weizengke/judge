#include <gtest/gtest.h>

#if 1
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

extern int compare(const char *file1, const char *file2);

#define MY_ASSERT_EQ(expect,input)\
	if (expect==input){ASSERT_TRUE(expect==input);} else {printf("expect %d, but input %d\r\n",expect,input);ASSERT_TRUE(expect==input);}

TEST(judger_checker_suite, case1) {
	int ret = compare("test/data_1.in","test/data_ans.in");
	MY_ASSERT_EQ(V_AC,ret);
}

TEST(judger_checker_suite, case2) {
	int ret = compare("test/data_2.in","test/data_ans.in");
	MY_ASSERT_EQ(V_AC,ret);
}

TEST(judger_checker_suite, case3) {
	int ret = compare("test/data_3.in","test/data_ans.in");
	MY_ASSERT_EQ(V_WA,ret);
}

TEST(judger_checker_suite, case4) {
	int ret = compare("test/data_4.in","test/data_ans.in");
	MY_ASSERT_EQ(V_PE,ret);
}

TEST(judger_checker_suite, case5) {
	int ret = compare("test/data_5.in","test/data_ans.in");
	MY_ASSERT_EQ(V_WA,ret);
}

TEST(judger_checker_suite, case6) {
	int ret = compare("test/data_6.in","test/data_ans.in");
	MY_ASSERT_EQ(V_PE,ret);
}

TEST(judger_checker_suite, mul_line_no_cr_return_wa) {
	int ret = compare("test/1_1.in","test/1_ans.in");
	MY_ASSERT_EQ(V_WA,ret);
}
TEST(judger_checker_suite, ignore_space_of_line_tailer) {
	int ret = compare("test/1_2.in","test/1_ans.in");
	MY_ASSERT_EQ(V_AC,ret);
}

TEST(judger_checker_suite, NULL_equal_CR) {
	int ret = compare("test/2_1.out","test/2_ans.out");
	MY_ASSERT_EQ(V_AC,ret);
}

#include "util.h"
void test_judge_json_mode()
{
	char buf[1024] = {0};
	util_fread("1.json", buf, 1024);
	extern void judge_request_enqueue_test(char *jsonMsg);
	judge_request_enqueue_test(buf);
}

#endif

