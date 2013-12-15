/*
  风继续吹


夜色如此放肆，
从不知：
风继续吹。
瑟缩街中落泪，
只有你，
可细说，
可倾诉。

*/
#include <iostream>
#include <windows.h>
#include <process.h>
#include <stdlib.h>
//#include <thread>
#include <string>
//#include <mutex>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <queue>

#include "..\include\pdt_common_inc.h"

using namespace std;

vector <APP_INFO_S*> g_vector_appInfo;

int RegistAppInfo(APP_INFO_S *pstAppInfo)
{
	int ret = OS_OK;

	g_vector_appInfo.push_back(pstAppInfo);

	if (NULL != pstAppInfo->pfInitFunction)
	{
		ret = pstAppInfo->pfInitFunction();
		if (OS_OK != ret)
		{
			pdt_debug_print("%s Task init failed...", pstAppInfo->taskName);
			pdt_debug_print("%s Task RegistAppInfo failed...", pstAppInfo->taskName);
			return OS_ERR;
		}

		pdt_debug_print("%s Task init ok...", pstAppInfo->taskName);
	}

	pdt_debug_print("%s Task RegistAppInfo ok...", pstAppInfo->taskName);

	return OS_OK;

}

int main()
{
	pdt_debug_print("OS Main-task Running...");

	extern void Cmd_RegAppInfo();
	Cmd_RegAppInfo();

	extern void Debug_RegAppInfo();
	Debug_RegAppInfo();

	extern void Judge_RegAppInfo();
	Judge_RegAppInfo();


	for (vector<int>::size_type ix = 0; ix < g_vector_appInfo.size(); ++ix)
	{
		if (NULL != g_vector_appInfo[ix]->pfTaskMain)
		{
			_beginthread(g_vector_appInfo[ix]->pfTaskMain,0,NULL);

			pdt_debug_print("%s Task running ok...", g_vector_appInfo[ix]->taskName);
		}
	}

	pdt_debug_print("OS Main-task init ok...");

	/* 循环读取消息队列 */
	for ( ; ; )
	{

		RunDelay(1);
	}


	return 0;
}
