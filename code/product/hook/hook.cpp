#include <windows.h>
#include <shellapi.h>
#include <winsock2.h>

#include <stdio.h>
#include <string.h>

#pragma comment(lib,"ws2_32")

#define BOOL_OK 1
#define BOOL_ERR 1

/* Hook 注册的API个数 */
#define PROC_MAX 15

DWORD* lpAddr[PROC_MAX];
PROC OldProc[PROC_MAX];

typedef struct
{
	char szCalleeModName[MAX_PATH];
	PROC oldFunc;
	PROC newFunc;
} API_FUNC_ID;

int APIHOOK_TRACE(char *szFuncName)
{
	MessageBox(NULL, szFuncName, "API HOOK", 0);
	//puts(szFuncName);
	return 0;
}

/*
进程线程相关
*/
BOOL  __stdcall MyGetCurrentProcess( VOID )
{
	APIHOOK_TRACE("GetCurrentProcess");
	return 0;
}

BOOL  __stdcall  MyCreateProcess(LPCTSTR lApplicationName,
LPTSTR lpCommandLine,
LPSECURITY_ATTRIBUTES lpProcessAttributes,
LPSECURITY_ATTRIBUTES lpThreadAttributes,
BOOL bInheritHandles,
DWORD dwCreationFlags,
LPVOID lpEnvironment,
LPCTSTR lpCurrentDirectory,
LPSTARTUPINFO lpStartupInfo,
LPPROCESS_INFORMATION lpProcessInformation )
{
	APIHOOK_TRACE("CreateProcess");
	return 0;
}

BOOL  __stdcall MyCreateThread( LPSECURITY_ATTRIBUTES lpThreadAttributes, DWORD dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId )
{
	APIHOOK_TRACE("CreateThread");
	return 0;
}

/*
文件相关
*/
BOOL  __stdcall MyCreateFileA(
LPCWSTR lpFileName,
DWORD dwDesiredAccess,
DWORD dwShareMode,
LPSECURITY_ATTRIBUTES lpSecurityAttributes,
DWORD dwCreationDisposition,
DWORD dwFlagsAndAttributes,
HANDLE hTemplateFile)
{
	APIHOOK_TRACE("CreateFileA");
	return 0;
}

BOOL  __stdcall MyCreateFileW(
LPCWSTR lpFileName,
DWORD dwDesiredAccess,
DWORD dwShareMode,
LPSECURITY_ATTRIBUTES lpSecurityAttributes,
DWORD dwCreationDisposition,
DWORD dwFlagsAndAttributes,
HANDLE hTemplateFile)
{
	APIHOOK_TRACE("CreateFileW");
	return 0;
}

BOOL  __stdcall MyDeleteFileA(LPCSTR lpFileName)
{
	APIHOOK_TRACE("DeleteFileA");
	return 0;
}

BOOL  __stdcall MyDeleteFileW(LPCSTR lpFileName)
{
	APIHOOK_TRACE("DeleteFileW");
	return 0;
}

/*
CMD相关
*/
BOOL  __stdcall MyShellExecuteA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd)
{
	APIHOOK_TRACE("ShellExecuteA");
	return 0;
}

BOOL  __stdcall MyShellExecuteW(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd)
{
	APIHOOK_TRACE("ShellExecuteW");
	return 0;
}

BOOL  __stdcall MyWinExec(LPCSTR lpCmdLine, UINT uCmdShow)
{
	APIHOOK_TRACE("WinExec");
	return 0;
}

/*
窗口相关
*/
BOOL  __stdcall MyMessageBoxA(HWND hWnd , LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	APIHOOK_TRACE("MessageBoxA");
	return 0;
}

BOOL  __stdcall MyMessageBoxW(HWND hWnd , LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	APIHOOK_TRACE("MessageBoxW");
	return 0;
}

/*
Socket相关
*/
BOOL  __stdcall MyWSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData)
{
	APIHOOK_TRACE("WSAStartup");
	return 0;
}

BOOL  __stdcall Myaccept(SOCKET s, struct sockaddr FAR *addr, int FAR *addrlen)
{
	APIHOOK_TRACE("accept");
	return 0;
}

BOOL  __stdcall Mysocket(int af, int type, int protocol)
{
	APIHOOK_TRACE("socket");
	return 0;
}

const API_FUNC_ID MANDATORY_API_FUNCS[] =
{
	{"Kernel32.dll", (PROC)GetCurrentProcess, (PROC)MyGetCurrentProcess},
	{"Kernel32.dll", (PROC)CreateProcess, (PROC)MyCreateProcess},
	{"Kernel32.dll", (PROC)CreateThread, (PROC)MyCreateThread},

	//3

	{"Kernel32.dll", (PROC)CreateFileA, (PROC)MyCreateFileA},
	{"Kernel32.dll", (PROC)CreateFileW, (PROC)MyCreateFileW},
	{"Kernel32.dll", (PROC)DeleteFileA, (PROC)MyDeleteFileA},
	{"Kernel32.dll", (PROC)DeleteFileW, (PROC)MyDeleteFileW},
	{"Kernel32.dll", (PROC)WinExec,        (PROC)MyWinExec},

	{"shell32.dll", (PROC)ShellExecuteA, (PROC)MyShellExecuteA},
	{"shell32.dll", (PROC)ShellExecuteW, (PROC)MyShellExecuteW},

	//10

	{"User32.dll", (PROC)MessageBoxA, (PROC)MyMessageBoxA},
	{"User32.dll", (PROC)MessageBoxW, (PROC)MyMessageBoxW},
	//12

	{"Ws2_32.dll", (PROC)WSAStartup, (PROC)MyWSAStartup},
	{"Ws2_32.dll", (PROC)accept, (PROC)Myaccept},
	{"Ws2_32.dll", (PROC)socket, (PROC)Mysocket},
	//15
};

int  ApiHook(char *DllName,//DLL文件名
				PROC OldFunAddr,//要HOOK的函数地址
				PROC NewFunAddr,//我们够造的函数地址
				int index
)
{
	//得到函数进程模块基地址
	HMODULE lpBase = GetModuleHandle(NULL);
	IMAGE_DOS_HEADER *dosHeader;
	IMAGE_NT_HEADERS *ntHeader;
	//IMAGE_IMPORT_BY_NAME *ImportName;
	//定位到DOS头
	dosHeader=(IMAGE_DOS_HEADER*)lpBase;
	//定位到PE头
	ntHeader=(IMAGE_NT_HEADERS32*)((BYTE*)lpBase+dosHeader->e_lfanew);
	//定位到导入表
	IMAGE_IMPORT_DESCRIPTOR *pImportDesc=(IMAGE_IMPORT_DESCRIPTOR*)((BYTE*)lpBase+ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	//循环遍历IMAGE_IMPORT_DESCRIPTOR机构数组
	while(pImportDesc->FirstThunk)
	{
		//得到DLL文件名
		char* pszDllName = (char*)((BYTE*)lpBase + pImportDesc->Name);
		//比较得到的DLL文件名是否和要HOOK函数所在的DLL相同
		if(lstrcmpiA(pszDllName, DllName) == 0)
		{
			break;
		}
		pImportDesc++;
	}
	//定位到FirstThunk参数指向的IMAGE_THUNK_DATA，此时这个结构已经是函数入口点地址了
	IMAGE_THUNK_DATA* pThunk = (IMAGE_THUNK_DATA*)
	((BYTE*)lpBase + pImportDesc->FirstThunk);
	//遍历这部分IAT表
	while(pThunk->u1.Function)
	{
		lpAddr[index] = (DWORD*)&(pThunk->u1.Function);
		//比较函数地址是否相同
		if(*(lpAddr[index]) == (DWORD)OldFunAddr)
		{
			DWORD dwOldProtect;
			//修改内存包含属性
			VirtualProtect(lpAddr[index], sizeof(DWORD), PAGE_READWRITE, &dwOldProtect);
			//API函数的入口点地址改成我们构造的函数的地址
			if (0 == WriteProcessMemory(GetCurrentProcess(),lpAddr[index], &NewFunAddr, sizeof(DWORD), NULL))
			{
				return BOOL_ERR;
			}
		}
		pThunk++;
	}
return BOOL_OK;
}

BOOL APIENTRY DllMain( HANDLE hModule,
						DWORD  ul_reason_for_call,
						LPVOID lpReserved)
{
	int iLoop = 0;

	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		//得到函数地址
		for (iLoop = 0; iLoop < PROC_MAX; ++iLoop)
		{
			//定位，修改IAT表
			ApiHook((char*)MANDATORY_API_FUNCS[iLoop].szCalleeModName,
			MANDATORY_API_FUNCS[iLoop].oldFunc,
			MANDATORY_API_FUNCS[iLoop].newFunc,
			iLoop);
		}

		break;
		case DLL_PROCESS_DETACH:
		//恢复IAT表中API函数的入口点地址
		for (iLoop = 0; iLoop < PROC_MAX; ++iLoop)
		{
			//定位，修改IAT表
			WriteProcessMemory(GetCurrentProcess(), lpAddr[iLoop], (void*)&MANDATORY_API_FUNCS[iLoop].oldFunc, sizeof(DWORD), NULL);
		}

		break;
	}
	return TRUE;
}
