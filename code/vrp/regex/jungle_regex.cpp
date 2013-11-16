#include <stdio.h>
#include <string.h>
#include "pcre.h"

int getInfo(char *src, char *pattern, char *res)
{
    pcre  *re;
    const char *error;
    int  erroffset;
    int  ovector[30];
    int  rc, i;

	re = pcre_compile(pattern, 0, &error, &erroffset, NULL);
    if (re == NULL) {                 //如果编译失败，返回错误信息
        return 1;
    }

    rc = pcre_exec(re,NULL, src, strlen(src), 0, 0, ovector, 30);
	// 返回值：匹配成功返回非负数，没有匹配返回负数
    if (rc < 0) {                     //如果没有匹配，返回错误信息
		if (rc == PCRE_ERROR_NOMATCH) printf("Sorry, no match ...\n");
		else {
			printf("Matching error %d\n", rc);
		}
		pcre_free(re);
		return 1;
	}

	i = (rc==0)?(0):(rc-1);

//	for (i = 0; i < rc; i++) //分别取出捕获分组 $0整个正则公式 $1第一个()
	{
        char *substring_start =  src + ovector[2*i];
        int substring_length = ovector[2*i+1] - ovector[2*i];

		char *str_tmp = (char*)malloc(sizeof(char)*substring_length+100);

		sprintf(str_tmp, "%.*s\n", substring_length, substring_start);
		printf("In getInfoByTag 4...(length = %d)", strlen(str_tmp));
		printf("End getInfoByTag success...[%s]",str_tmp);

		free(str_tmp);
    }


	pcre_free(re);

	return 0;
}

int test_main()
{
	int ulRet = 0;

	char  patternTime [] = "(\\d*) MS";
	char *pattern="jungle  C++  123 MS 2012-3-2";
	ulRet = getInfo(pattern, patternTime, 0);
	return 0;
}

