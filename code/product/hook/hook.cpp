
/*
	Online judge 最重要的安全防护技术: API-HOOK
*/

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>  

#pragma comment(lib,"ws2_32")

#define BOOL_OK 1
#define BOOL_ERR 1


typedef struct
{
	LPCSTR szCalleeModName;
	LPCSTR oldFuncName;
	PROC newFuncAddr;
	PROC oldFuncAddr;
} API_FUNC_ID;

int APIHOOK_TRACE(char *szFuncName)
{
	//printf("API Hook:%s.\r\n",szFuncName);
	return 0;
}

/*
进程线程相关
*/
HANDLE WINAPI MyGetCurrentProcess( VOID )
{
	APIHOOK_TRACE("GetCurrentProcess");
	return 0;
}

WINBOOL WINAPI MyCreateProcess(LPCTSTR lApplicationName,
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

HANDLE WINAPI MyCreateThread( LPSECURITY_ATTRIBUTES lpThreadAttributes, DWORD dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId )
{
	APIHOOK_TRACE("CreateThread");
	return 0;
}


/*
文件相关
*/
HANDLE WINAPI MyCreateFileA(
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

HANDLE WINAPI MyCreateFileW(
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

WINBOOL WINAPI MyDeleteFileA(LPCSTR lpFileName)
{
	APIHOOK_TRACE("DeleteFileA");
	return 0;
}

WINBOOL WINAPI MyDeleteFileW(LPCSTR lpFileName)
{
	APIHOOK_TRACE("DeleteFileW");
	return 0;
}

/*
CMD相关
*/
UINT WINAPI MyShellExecuteA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd)
{
	APIHOOK_TRACE("ShellExecuteA");
	return 0;
}

UINT WINAPI MyShellExecuteW(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd)
{
	APIHOOK_TRACE("ShellExecuteW");
	return 0;
}

UINT WINAPI MyWinExec(LPCSTR lpCmdLine, UINT uCmdShow)
{
	APIHOOK_TRACE("WinExec");
	return 0;
}

/*
窗口相关
*/
int WINAPI MyMessageBoxA(HWND hWnd , LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	APIHOOK_TRACE("MessageBoxA");
	return 0;
}

int WINAPI MyMessageBoxW(HWND hWnd , LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	APIHOOK_TRACE("MessageBoxW");
	return 0;
}

/*
Socket相关
*/
int WSAAPI MyWSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData)
{
	APIHOOK_TRACE("WSAStartup");
	return WSAStartup(wVersionRequired, lpWSAData);
}

WINBOOL WINAPI Myaccept(SOCKET s, struct sockaddr FAR *addr, int FAR *addrlen)
{
	APIHOOK_TRACE("accept");
	return 0;
}

WINBOOL WINAPI Mysocket(int af, int type, int protocol)
{
	APIHOOK_TRACE("socket");
	return 0;
}

DWORD WINAPI MyGetCurrentProcessId(void)
{
	APIHOOK_TRACE("GetCurrentProcessId");
	return 0;
}


WINBOOL WINAPI MyTerminateProcess (HANDLE hProcess, UINT uExitCode)
{
	APIHOOK_TRACE("TerminateProcess");
	return 0;
}

HANDLE WINAPI MyOpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId)
{  
	APIHOOK_TRACE("OpenProcess");
    return NULL;  
}  

#define MakePtr(cast, ptr, AddValue) (cast)((size_t)(ptr)+(size_t)(AddValue))  
  
static PIMAGE_IMPORT_DESCRIPTOR GetNamedImportDescriptor(HMODULE hModule, LPCSTR szImportModule)  
{  
    PIMAGE_DOS_HEADER pDOSHeader;  
    PIMAGE_NT_HEADERS pNTHeader;  
    PIMAGE_IMPORT_DESCRIPTOR pImportDesc;  
  
    if ((szImportModule == NULL) || (hModule == NULL))  
        return NULL;  
    pDOSHeader = (PIMAGE_DOS_HEADER) hModule;  
    if (IsBadReadPtr(pDOSHeader, sizeof(IMAGE_DOS_HEADER)) || (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE)) {  
        return NULL;  
    }  
    pNTHeader = MakePtr(PIMAGE_NT_HEADERS, pDOSHeader, pDOSHeader->e_lfanew);  
    if (IsBadReadPtr(pNTHeader, sizeof(IMAGE_NT_HEADERS)) || (pNTHeader->Signature != IMAGE_NT_SIGNATURE))  
        return NULL;  
    if (pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress == 0)  
        return NULL;  
    pImportDesc = MakePtr(PIMAGE_IMPORT_DESCRIPTOR, pDOSHeader, pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);  
    while (pImportDesc->Name) {  
        PSTR szCurrMod = MakePtr(PSTR, pDOSHeader, pImportDesc->Name);  
        if (_stricmp(szCurrMod, szImportModule) == 0)  
            break;  
        pImportDesc++;  
    }  
    if (pImportDesc->Name == (DWORD)0)  
        return NULL;  
    return pImportDesc;  
}  
  
static BOOL IsNT()  
{  
    OSVERSIONINFO stOSVI;  
    BOOL bRet;  
  
    memset(&stOSVI, 0, sizeof(OSVERSIONINFO));  
    stOSVI.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);  
    bRet = GetVersionEx(&stOSVI);  
    if (FALSE == bRet) return FALSE;  
    return (VER_PLATFORM_WIN32_NT == stOSVI.dwPlatformId);  
}  
  
static BOOL HookImportFunction(HMODULE hModule, LPCSTR szImportModule, LPCSTR szFunc, PROC paHookFuncs, PROC* paOrigFuncs)  
{  
    PIMAGE_IMPORT_DESCRIPTOR pImportDesc;  
    PIMAGE_THUNK_DATA pOrigThunk;  
    PIMAGE_THUNK_DATA pRealThunk;  
  
    if (!IsNT() && ((size_t)hModule >= 0x80000000))  
        return FALSE;  
    pImportDesc = GetNamedImportDescriptor(hModule, szImportModule);  
    if (pImportDesc == NULL)  
        return FALSE;  
    pOrigThunk = MakePtr(PIMAGE_THUNK_DATA, hModule, pImportDesc->OriginalFirstThunk);  
    pRealThunk = MakePtr(PIMAGE_THUNK_DATA, hModule, pImportDesc->FirstThunk);  
    while (pOrigThunk->u1.Function) {  
        if (IMAGE_ORDINAL_FLAG != (pOrigThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)) {  
            PIMAGE_IMPORT_BY_NAME pByName = MakePtr(PIMAGE_IMPORT_BY_NAME, hModule, pOrigThunk->u1.AddressOfData);  
            BOOL bDoHook;  
            // When hook EditPlus, read pByName->Name[0] will case this dll terminate, so call IsBadReadPtr() here.  
            if (IsBadReadPtr(pByName, sizeof(IMAGE_IMPORT_BY_NAME))) {  
                pOrigThunk++;  
                pRealThunk++;  
                continue;                 
            }  
            if ('\0' == pByName->Name[0]) {  
                pOrigThunk++;  
                pRealThunk++;  
                continue;  
            }  
            bDoHook = FALSE;  
            if ((szFunc[0] == pByName->Name[0]) && (_strcmpi(szFunc, (char*)pByName->Name) == 0)) {  
                if (paHookFuncs)  
                    bDoHook = TRUE;  
            }  
            if (bDoHook) {  
                MEMORY_BASIC_INFORMATION mbi_thunk;  
                DWORD dwOldProtect;  
  
                VirtualQuery(pRealThunk, &mbi_thunk, sizeof(MEMORY_BASIC_INFORMATION));  
                VirtualProtect(mbi_thunk.BaseAddress, mbi_thunk.RegionSize, PAGE_READWRITE, &mbi_thunk.Protect);  
                if (paOrigFuncs)  
                    *paOrigFuncs = (PROC)pRealThunk->u1.Function;  
                pRealThunk->u1.Function = (DWORD)paHookFuncs;  
  
                VirtualProtect(mbi_thunk.BaseAddress, mbi_thunk.RegionSize, mbi_thunk.Protect, &dwOldProtect);  
                return TRUE;  
            }  
        }  
        pOrigThunk++;  
        pRealThunk++;  
    }  
    return FALSE;  
}  
  
BOOL HookAPI(LPCSTR szImportModule, LPCSTR szFunc, PROC paHookFuncs, PROC* paOrigFuncs)  
{  
    HANDLE hSnapshot;  
    MODULEENTRY32 me = {sizeof(MODULEENTRY32)};  
    BOOL bOk;  
  
    if ((szImportModule == NULL) || (szFunc == NULL) || (paHookFuncs == NULL)) {  
        return FALSE;  
    }  

	//printf("szImportModule=%s, szFunc=%s, paHookFuncs=0x%x\r\n", szImportModule, szFunc, paHookFuncs);
	
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,0);  
      
    bOk = Module32First(hSnapshot,&me);  
    while (bOk) {  
        HookImportFunction(me.hModule, szImportModule, szFunc, paHookFuncs, paOrigFuncs);  
        bOk = Module32Next(hSnapshot,&me);  
    }  
    return TRUE;  
} 

API_FUNC_ID MANDATORY_API_FUNCS[] =
{
	{"Kernel32.dll", (LPCSTR)"OpenProcess", (PROC)MyOpenProcess, NULL},
	{"Kernel32.dll", (LPCSTR)"GetCurrentProcess", (PROC)MyGetCurrentProcess, NULL},	
	{"Kernel32.dll", (LPCSTR)"CreateProcess", (PROC)MyCreateProcess, NULL},			
    //{"Kernel32.dll", (LPCSTR)"CreateThread", (PROC)MyCreateThread, NULL},    Ruby
	//{"Kernel32.dll", (LPCSTR)"GetCurrentProcessId", (PROC)MyGetCurrentProcessId, NULL}, java
	//{"Kernel32.dll", (LPCSTR)"CreateFileA", (PROC)MyCreateFileA, NULL}, java
	{"Kernel32.dll", (LPCSTR)"CreateFileW", (PROC)MyCreateFileW, NULL},
	{"Kernel32.dll", (LPCSTR)"DeleteFileA", (PROC)MyDeleteFileA, NULL},
	{"Kernel32.dll", (LPCSTR)"DeleteFileW", (PROC)MyDeleteFileW, NULL},	
	{"Kernel32.dll", (LPCSTR)"TerminateProcess",        (PROC)MyTerminateProcess, NULL},
	{"Kernel32.dll", (LPCSTR)"WinExec",        (PROC)MyWinExec, NULL},	
	{"shell32.dll", (LPCSTR)"ShellExecuteA", (PROC)MyShellExecuteA, NULL},
	{"shell32.dll", (LPCSTR)"ShellExecuteW", (PROC)MyShellExecuteW, NULL},
	{"User32.dll", (LPCSTR)"MessageBoxA", (PROC)MyMessageBoxA, NULL},
	{"User32.dll", (LPCSTR)"MessageBoxW", (PROC)MyMessageBoxW, NULL},
	//{"Ws2_32.dll", (LPCSTR)"WSAStartup", (PROC)MyWSAStartup, NULL},   Ruby
	{"Ws2_32.dll", (LPCSTR)"accept", (PROC)Myaccept, NULL},
	{"Ws2_32.dll", (LPCSTR)"socket", (PROC)Mysocket, NULL},
	
};

VOID DLL_InstallHook()
{
	int iLoop = 0;
	int num = sizeof(MANDATORY_API_FUNCS)/sizeof(API_FUNC_ID);
	
	for (iLoop = 0; iLoop < num; ++iLoop)
	{
		(VOID)HookAPI(MANDATORY_API_FUNCS[iLoop].szCalleeModName,
			MANDATORY_API_FUNCS[iLoop].oldFuncName,
			(PROC)MANDATORY_API_FUNCS[iLoop].newFuncAddr,
			&MANDATORY_API_FUNCS[iLoop].oldFuncAddr);
	}	
}
VOID DLL_UnstallHook()
{
	int iLoop = 0;
	int num = sizeof(MANDATORY_API_FUNCS)/sizeof(API_FUNC_ID);
	
	for (iLoop = 0; iLoop < num; ++iLoop)
	{
		(VOID)HookAPI(MANDATORY_API_FUNCS[iLoop].szCalleeModName,
			MANDATORY_API_FUNCS[iLoop].oldFuncName,
			MANDATORY_API_FUNCS[iLoop].oldFuncAddr,
			NULL);
	}	
}

extern "C" bool WINAPI DllMain(HMODULE hModule,  
    DWORD  ul_reason_for_call,  
    LPVOID lpReserved  
    )  
{  
    switch (ul_reason_for_call)  
    {  
    case DLL_PROCESS_ATTACH:  
		DLL_InstallHook();				
		break;
    case DLL_THREAD_ATTACH:  
		break; 
    case DLL_THREAD_DETACH:  
		break; 
    case DLL_PROCESS_DETACH:  
		DLL_UnstallHook();
		break;  
    }  
	
    return TRUE;  
} 

