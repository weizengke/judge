/*
	  风继续吹

	夜色如此放肆，
	从不知：
	风继续吹。
	瑟缩街中落泪，
	只有你，
	可细说，
	可倾诉。

            By Jungle Wei.

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

#include "product\include\pdt_common_inc.h"

using namespace std;

vector <APP_INFO_S*> g_vector_appInfo;

#define APP_MAX_SYSNAME_SIZE 24

char g_sysname[APP_MAX_SYSNAME_SIZE] = "Jungle";
HWND g_hWnd = NULL;

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

void pdt_init()
{
	GetPrivateProfileString("System","sysname","Judge-Kernel",g_sysname,sizeof(g_sysname),INI_filename);

	::SetConsoleTitle(g_sysname);

	g_hWnd=::GetConsoleWindow();

	if (g_hWnd !=NULL)
	{
		HANDLE hIcon=NULL;
		hIcon=::LoadImage(GetModuleHandle(NULL),"beyond.ico",
		IMAGE_ICON,32,32,LR_LOADFROMFILE);
		if (hIcon!=NULL)
		{
			::SendMessage(g_hWnd,WM_SETICON,ICON_SMALL,(LPARAM)hIcon);
		}

	}
	else
	{
		pdt_debug_print("Set judge kernel ico failed. ");
	}

}

int main()
{
	pdt_init();

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
