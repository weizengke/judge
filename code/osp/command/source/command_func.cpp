
/*
	 如梦令．偶然

	夜里无意心烦，
	发丝如此蓬乱。
	耳边声声颤，
	是谁嘴里轻叹。
	偶然，
	偶然，
	是我太过留恋。

*/


#include "osp\command\include\command_inc.h"


#if M_DES("cmd_debugging_enable_st",1)
DEFUN(cmd_debugging_enable_st, (char*)"debugging enable", (char*)"Debugging switch on", Debugging_enable)
{
	if (g_debug_switch == DEBUG_ENABLE)
	{
		printf("Info: debugging switch is already enable.\n");
		return 0;
	}

	g_debug_switch = DEBUG_ENABLE;

	extern ULONG Judge_DebugSwitch(ULONG st);
	Judge_DebugSwitch(DEBUG_ENABLE);

	printf("Info: debugging switch is enable.\n");

	return 0;
}
#endif

#if M_DES("cmd_undo_debugging_enable_st",1)
DEFUN(cmd_undo_debugging_enable_st, (char*)"undo debugging enable", (char*)"Debugging switch off", undo_debugging_enable)
{

	if (g_debug_switch == DEBUG_DISABLE)
	{
		printf("Info: debugging switch is already disable.\n");
		return 0;
	}

	g_debug_switch = DEBUG_DISABLE;

	extern ULONG Judge_DebugSwitch(ULONG st);
	Judge_DebugSwitch(DEBUG_DISABLE);

	printf("Info: debugging switch is disable.\n");

	return 0;
}
#endif

#if M_DES("cmd_display_clock_st",1)
DEFUN(cmd_display_clock_st, (char*)"display clock", (char*)"Display clock of device", display_clock)
{
	time_t	timep = time(NULL);
	struct tm *p;

	p = localtime(&timep);
	p->tm_year = p->tm_year + 1900;
	p->tm_mon = p->tm_mon + 1;

	printf(" Date of device:\r\n %04d-%02d-%02d %02d:%02d:%02d UTC(+8) DTS\n",p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);

	return 0;
}
#endif

#if M_DES("cmd_display_computer_st",1)
DEFUN(cmd_display_computer_st, (char*)"display computer", (char*)"Display computer information", display_computer)
{
	printf("This is Jungle Wei's computer.\n");
	return 0;
}
#endif

#if M_DES("cmd_display_version_st",1)
DEFUN(cmd_display_version_st, (char*)"display version", (char*)"Display device version", display_version)
{
	printf("Judge Kernel Version: 1.0.3 , released at 2014-03-23.\n");
	return 0;
}
#endif

#if M_DES("cmd_sysname_st",1)
DEFUN(cmd_sysname_st, (char*)"sysname STRING<1-24>", (char*)"set system name", sysname)
{
	CMD_DBGASSERT(argv[1] != 0);

	strcpy(g_sysname, argv[1]);

	::SetConsoleTitle(g_sysname);

	printf("Info: system name change to %s successful.\r\n", g_sysname);

	return 0;
}
#endif

#if M_DES("cmd_display_history_st",1)
DEFUN(cmd_display_history_st, (char*)"display history", (char*)"Display history command", display_history)
{
	int try_idx = 0;
	int i = 0;

	for (i = 0;  i < HISTORY_MAX_SIZE; i++)
	{
		if (vty->history[i] == NULL)
			break;
	}

	for (i = i-1; i >= 0; i--)
	{
		if (vty->history[i] == NULL)
			break;

		cmd_outstring("%s\r\n", vty->history[i]);
	}

	return 0;
}
#endif

#if M_DES("cmd_display_history_n_st",1)
DEFUN(cmd_display_history_n_st, (char*)"display history INTEGER<1-100>", (char*)"Display history command", display_history_n)
{
	int n = 0;
	int i = 0;

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "%d %s %s %s\n", argc, argv[0], argv[1], argv[2]);

	CMD_DBGASSERT(argv[2]);

	n = atoi(argv[2]);

	debug_print_ex(CMD_DEBUG_TYPE_FUNC,"n = %d\n", n);

	for (i = 0;  i < HISTORY_MAX_SIZE; i++)
	{
		if (vty->history[i] == NULL)
			break;
	}

	for (i = i-1; i >= 0 && n > 0; i--,n--)
	{
		if (vty->history[i] == NULL)
			break;

		cmd_outstring("%s\r\n", vty->history[i]);
	}

	debug_print_ex(CMD_DEBUG_TYPE_FUNC,"+");

	return 0;
}

#endif

#if M_DES("cmd_debugging_error_st",1)

DEFUN(cmd_debugging_error_st, (char*)"debugging error", (char*)"open debugging error switch", debugging_error)
{
	CMD_DEBUGMASK_SET(CMD_DEBUG_TYPE_ERROR);
	printf("Info: debugging error switch is on.\r\n");
	return 0;
}
#endif

#if M_DES("cmd_undo_debugging_error_st",1)

DEFUN(cmd_undo_debugging_error_st, (char*)"undo debugging error", (char*)"close debugging error switch", undo_debugging_error)
{
	CMD_DEBUGMASK_CLEAR(CMD_DEBUG_TYPE_ERROR);
	printf("Info: debugging error switch is off.\r\n");
	return 0;
}
#endif

#if M_DES("cmd_debugging_function_st",1)

DEFUN(cmd_debugging_function_st, (char*)"debugging function", (char*)"open debugging function switch", debugging_function)
{
	CMD_DEBUGMASK_SET(CMD_DEBUG_TYPE_FUNC);
	printf("Info: debugging function switch is on.\r\n");
	return 0;
}
#endif

#if M_DES("cmd_undo_debugging_function_st",1)

DEFUN(cmd_undo_debugging_function_st, (char*)"undo debugging function", (char*)"Close debugging function switch", undo_debugging_function)
{
	CMD_DEBUGMASK_CLEAR(CMD_DEBUG_TYPE_FUNC);
	printf("Info: debugging function switch is off.\r\n");
	return 0;
}
#endif

#if M_DES("cmd_debugging_info_st",1)

DEFUN(cmd_debugging_info_st, (char*)"debugging info", (char*)"Open debugging info switch", debugging_info)
{
	CMD_DEBUGMASK_SET(CMD_DEBUG_TYPE_INFO);
	printf("Info: debugging info switch is on.\r\n");
	return 0;
}
#endif

#if M_DES("cmd_undo_debugging_info_st",1)

DEFUN(cmd_undo_debugging_info_st, (char*)"undo debugging info", (char*)"close debugging info switch", undo_debugging_info)
{
	CMD_DEBUGMASK_CLEAR(CMD_DEBUG_TYPE_INFO);
	printf("Info: debug info switch is off.\r\n");
	return 0;
}
#endif

#if M_DES("cmd_debugging_message_st",1)

DEFUN(cmd_debugging_message_st, (char*)"debugging message", (char*)"Open debugging message switch", debugging_message)
{
	CMD_DEBUGMASK_SET(CMD_DEBUG_TYPE_MSG);
	printf("Info: debug message switch is on.\r\n");
	return 0;
}
#endif

#if M_DES("cmd_undo_debugging_message_st",1)

DEFUN(cmd_undo_debugging_message_st, (char*)"undo debugging message", (char*)"close debugging message switch", undo_debugging_message)
{
	CMD_DEBUGMASK_CLEAR(CMD_DEBUG_TYPE_MSG);
	printf("Info: debugging message switch is off.\r\n");
	return 0;
}
#endif

#if M_DES("cmd_debugging_fsm_st",1)

DEFUN(cmd_debugging_fsm_st, (char*)"debugging fsm", (char*)"open debugging fsm switch", debugging_fsm)
{
	CMD_DEBUGMASK_SET(CMD_DEBUG_TYPE_FSM);
	printf("Info: debugging fsm switch is on.\r\n");
	return 0;
}
#endif

#if M_DES("cmd_undo_debugging_fsm_st",1)

DEFUN(cmd_undo_debugging_fsm_st, (char*)"undo debugging fsm", (char*)"close debugging fsm switch", undo_debugging_fsm)
{
	CMD_DEBUGMASK_CLEAR(CMD_DEBUG_TYPE_FSM);
	printf("Info: debugging message switch is off.\r\n");
	return 0;
}
#endif

#if M_DES("cmd_debugging_all_st",1)

DEFUN(cmd_debugging_all_st, (char*)"debugging all", (char*)"open debugging all switch", debugging_all)
{
	int i;
	for (i = CMD_DEBUG_TYPE_NONE + 1; i < CMD_DEBUG_TYPE_MAX; i++ )
	{
		CMD_DEBUGMASK_SET(i);
	}

	printf("Info: debugging all switch is on.\r\n");
	return 0;
}
#endif

#if M_DES("cmd_undo_debugging_all_st",1)

DEFUN(cmd_undo_debugging_all_st, (char*)"undo debugging all", (char*)"close debugging all switch", undo_debugging_all)
{
	int i;
	for (i = CMD_DEBUG_TYPE_NONE + 1; i < CMD_DEBUG_TYPE_MAX; i++ )
	{
		CMD_DEBUGMASK_CLEAR(i);
	}

	printf("Info: debugging all switch is off.\r\n");
	return 0;

}
#endif

#if M_DES("cmd_display_debugging_st",1)

DEFUN(cmd_display_debugging_st, (char*)"display debugging", (char*)"display debugging switch", display_debugging)
{
	int i = 0;
	if (g_debug_switch == DEBUG_ENABLE)
	{
		printf("Global debugging is enable.\r\n");
	}
	else
	{
		printf("Global debugging is disable.\r\n");
	}

	printf(" DebugMask(0x%x", g_aulDebugMask[0]);
	for (i = 1; i < CMD_DEBUG_TYPE_MAX/CMD_MASKLENTG + 1 ; i++)
	{
		printf("	,0x%x", g_aulDebugMask[i]);
	}
	printf(").\r\n");

	for (i = CMD_DEBUG_TYPE_NONE + 1; i < CMD_DEBUG_TYPE_MAX; i++ )
	{
		if (CMD_DEBUGMASK_GET(i))
		{
			printf(" Debugging %s switch is on.\r\n", szDebugName[i]);
		}
	}

	return 0;
}
#endif

#if M_DES("cmd_hdujudge_login_st",1)
/*
version    : r90
description: 带参数命令，错误在参数时，无法定位错误位置
brief      :

hdu-judge login STRING<1-24> STRING<1-24>

<Jungle>hdu-judge  login  sa
        ^
Error: Unrecognized command at '^' position.
<Jungle>

*/
DEFUN(cmd_hdujudge_login_st, (char*)"hdu-judge login STRING<1-24> STRING<1-24>", (char*)"hdujudge login", hdujudge_login)
{
	char username_[25] = {0};
	char pwd[25] = {0};

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "%d %s %s %s\n", argc, argv[0], argv[1], argv[2], argv[3]);

	strcpy(username_, argv[2]);
	strcpy(pwd, argv[3]);

	extern int HDU_loginEx(char *uname, char *pdw);
	HDU_loginEx(username_, pwd);

	return 0;
}
#endif

#if M_DES("cmd_display_hdujudge_status_st",1)

DEFUN(cmd_display_hdujudge_status_st, (char*)"display hdu-judge status STRING<1-24>", (char*)"display hdujudge status <username>", display_hdujudge_status)
{
	char username_[25] = {0};

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "%d %s %s %s\n", argc, argv[0], argv[1], argv[2], argv[3]);

	strcpy(username_, argv[3]);

	extern int getStatusEx(char *);
	getStatusEx(username_);

	return 0;
}
#endif

#if M_DES("cmd_judge_solution_st",1)

DEFUN(cmd_judge_solution_st, (char*)"judge solution INTEGER<1-65535>", (char*)"judge solution <ID>", judge_solution)
{
	int solutionId = 0;

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "%d %s %s %s\n", argc, argv[0], argv[1], argv[2]);

	solutionId = atoi(argv[2]);

	extern int Judge_PushQueue(int solutionId);
	Judge_PushQueue(solutionId);

	cmd_outstring("Info: Solution judge-request has been sent to Judge-Queue asynchronously.\r\n");

	return 0;
}
#endif

#if M_DES("cmd_display_guetjudge_status_st",1)

DEFUN(cmd_display_guetjudge_status_st, (char*)"display guet-judge status STRING<1-24>", (char*)"display guetjudge status <username>", display_guetjudge_status)
{
	char username_[25] = {0};

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "%d %s %s %s\n", argc, argv[0], argv[1], argv[2], argv[3]);

	strcpy(username_, argv[3]);

	extern int GUET_getStatusEx(char *);
	GUET_getStatusEx(username_);

	return 0;
}
#endif

#if M_DES("cmd_guetjudge_login_st",1)
DEFUN(cmd_guetjudge_login_st, (char*)"guet-judge login STRING<1-24> STRING<1-24>", (char*)"guet-judge login <username> <password>", guetjudge_login_st)
{
	char username_[25] = {0};
	char psw_[25] = {0};

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "%d %s %s %s\n", argc, argv[0], argv[1], argv[2], argv[3]);

	strcpy(username_, argv[2]);
	strcpy(psw_, argv[3]);

	extern int GUET_Login(char *uname, char *psw);
	GUET_Login(username_, psw_);

	return 0;
}
#endif

#if M_DES("cmd_display_command_tree_st",1)

DEFUN(cmd_display_command_tree_st, (char*)"display command-tree", (char*)"display command tree", display_command_tree)
{
	extern void cmd_show_command_tree();
	cmd_show_command_tree();

	return 0;
}
#endif

#if M_DES("cmd_display_current_configuration_st",1)

DEFUN(cmd_display_current_configuration_st, (char*)"display current-configuration", (char*)"display current-configuration", display_current_configuration)
{
	extern void Judge_ShowCfgContent();
	Judge_ShowCfgContent();

	return 0;
}
#endif

#if M_DES("cmd_display_hdu_judge_problem_by_pid_st",1)

DEFUN(cmd_display_hdu_judge_problem_by_pid_st, (char*)"display hdu-judge problem INTEGER<1-65535>", (char*)"display hdu-judge problem by problem ID", display_hdu_judge_problem_by_pid)
{
	int problemId = 0;

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "%d %s %s %s %s\n", argc, argv[0], argv[1], argv[2], argv[2]);

	problemId = atoi(argv[3]);


	extern int DLL_GetProblemInfoFromHDU(int pid);

	int ret = DLL_GetProblemInfoFromHDU(problemId);
	if (OS_FALSE == ret)
	{
		cmd_outstring("Info: No such problam %d on hdu-judge.\r\n", problemId);
	}

	return 0;
}

#include <windows.h>
#include <conio.h>
#include <stdlib.h>
/*
COORD和CONSOLE_SCREEN_BUFFER_ INFO是wincon.h定义的控制台结构体类型，其原型如下：

// 坐标结构体
typedef struct _COORD {
SHORT X;
SHORT Y;
} COORD;

// 控制台窗口信息结构体
typedef struct _CONSOLE_SCREEN_BUFFER_INFO {
COORD dwSize; // 缓冲区大小
COORD dwCursorPosition; // 当前光标位置
WORD wAttributes; // 字符属性
SMALL_RECT srWindow; // 当前窗口显示的大小和位置
COORD dwMaximumWindowSize; // 最大的窗口缓冲区大小
} CONSOLE_SCREEN_BUFFER_INFO ;

*/

void gotoxy(int Wide,int High)
{
	HANDLE hOut;
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	COORD loc;
	loc.X=Wide;
	loc.Y=High;
	SetConsoleCursorPosition(hOut, loc);
}

/*
#define FOREGROUND_BLUE      0x0001 // text color contains blue.
#define FOREGROUND_GREEN     0x0002 // text color contains green.
#define FOREGROUND_RED       0x0004 // text color contains red.
#define FOREGROUND_INTENSITY 0x0008 // text color is intensified.
#define BACKGROUND_BLUE      0x0010 // background color contains blue.
#define BACKGROUND_GREEN     0x0020 // background color contains green.
#define BACKGROUND_RED       0x0040 // background color contains red.
#define BACKGROUND_INTENSITY 0x0080 // background color is intensified.
*/
void TestColor()
{
	HANDLE consolehwnd;
	consolehwnd = GetStdHandle(STD_OUTPUT_HANDLE);

	int i=0;
	SetConsoleTextAttribute(consolehwnd,i);
	printf("The color-value is %d\n", i);

	for(i=1;i<=0xFC;i++)
	{
		SetConsoleTextAttribute(consolehwnd,i);
		printf("The color-value is %d\n", i);

		/* 改变console text & backround */
		char clor[128] = {0};
		sprintf(clor, "color %x", i);
		system(clor);
	}

	//SetConsoleTextAttribute(consolehwnd,255);

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),23);
	printf("The color-value is %d..\n", 23);

}

void Test()
{
	HANDLE hOut;
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO bInfo;
	GetConsoleScreenBufferInfo(hOut, &bInfo );

	printf("Buf:X*Y=%u, Cur:X=%u, Y=%u\r\n"
			"WindowsSize:Bottom=%u, Left=%u, Right=%u, Top=%u\r\n",
			bInfo.dwSize.X * bInfo.dwSize.Y,
			bInfo.dwCursorPosition.X, bInfo.dwCursorPosition.Y,
			bInfo.srWindow.Bottom, bInfo.srWindow.Left, bInfo.srWindow.Right, bInfo.srWindow.Top);

	GetConsoleScreenBufferInfo(hOut, &bInfo );
	gotoxy(bInfo.dwCursorPosition.X+1, bInfo.dwCursorPosition.Y+1);

	TestColor();

}
#endif

#if M_DES("cmd_display_st",1)

DEFUN(cmd_display_st, (char*)"display", (char*)"display", display)
{
	printf("Info: display thread info.\n");

	extern int GetProcessThreadList();
	GetProcessThreadList();

	char current_path[MAX_PATH] = {0};
	GetCurrentDirectory(sizeof(current_path),current_path);

	//Test();

	return 0;
}
#endif

#if M_DES("cmd_set_config_section_name_value_st",1)

DEFUN(cmd_set_config_section_name_value_st, (char*)"set config STRING<1-24> STRING<1-24> STRING<1-65535>",
		(char*)"Set Config section name value", set_config_section_name_value_st)
{
	char section[25] = {0};
	char name[25] = {0};
	char value[65536]={0};

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "%d %s %s %s %s %s.\n", argc, argv[0], argv[1], argv[2], argv[3], argv[4]);

	strcpy(section, argv[2]);
	strcpy(name, argv[3]);
	strcpy(value, argv[4]);


	int ret = WritePrivateProfileString(section,name,value,INI_filename);

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

}
#endif

#if M_DES("cmd_virtual_judge_enable_st",1)

/* BEGIN: Added by weizengke, 2014/3/3 for 全局使能vjudge */
DEFUN(cmd_virtual_judge_enable_st, (char*)"virtual-judge enable", (char*)"virtual-judge enable", virtual_judge_enable_st)
{
	extern int GL_vjudge_enable;

	if (OS_YES == GL_vjudge_enable)
	{
		printf("Info: virtual-judge is already enable.\r\n");
		return OS_ERR;
	}

	int ret = WritePrivateProfileString("Tool","vjudge_enable","1",INI_filename);
	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		printf("Error: virtual-judge enable failed.\r\n");
		return OS_ERR;
	}

	GL_vjudge_enable = OS_YES;

}
#endif

#if M_DES("cmd_undo_virtual_judge_enable_st",1)

/* BEGIN: Added by weizengke, 2014/3/3 for 全局去使能vjudge */
DEFUN(cmd_undo_virtual_judge_enable_st, (char*)"undo virtual-judge enable", (char*)"undo virtual-judge enable", undo_virtual_judge_enable_st)
{
	extern int GL_vjudge_enable;

	if (OS_NO == GL_vjudge_enable)
	{
		printf("Info: virtual-judge is already disable.\r\n");
		return OS_ERR;
	}

	int ret = WritePrivateProfileString("Tool","vjudge_enable","0",INI_filename);
	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		printf("Error: virtual-judge disable failed.\r\n");
		return OS_ERR;
	}

	GL_vjudge_enable = OS_NO;

}
#endif

#if M_DES("cmd_hdu_judge_enable_st",1)

/* BEGIN: Added by weizengke, 2014/3/3 for 全局使能hdu-vjudge */
DEFUN(cmd_hdu_judge_enable_st, (char*)"hdu-judge enable", (char*)"hdu-judge enable", hdu_judge_enable_st)
{
	extern int hdu_vjudge_enable;

	if (OS_YES == hdu_vjudge_enable)
	{
		printf("Info: hdu-judge is already enable.\r\n");
		return OS_ERR;
	}

	int ret = WritePrivateProfileString("HDU","vjudge_enable","1",INI_filename);
	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		printf("Error: hdu-judge enable failed.\r\n");
		return OS_ERR;
	}

	hdu_vjudge_enable = OS_YES;

}
#endif

#if M_DES("cmd_undo_hdu_judge_enable_st",1)

/* BEGIN: Added by weizengke, 2014/3/3 for 全局去使能hdu-vjudge */
DEFUN(cmd_undo_hdu_judge_enable_st, (char*)"undo hdu-judge enable", (char*)"undo hdu-judge enable", undo_hdu_judge_enable_st)
{
	extern int hdu_vjudge_enable;

	if (OS_NO == hdu_vjudge_enable)
	{
		printf("Info: hdu-judge is already disable.\r\n");
		return OS_ERR;
	}

	int ret = WritePrivateProfileString("HDU","hdu_enable","0",INI_filename);
	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		printf("Error: hdu-judge disable failed.\r\n");
		return OS_ERR;
	}

	hdu_vjudge_enable = OS_NO;


}
#endif

#if M_DES("cmd_guet_judge_enable_st",1)

/* BEGIN: Added by weizengke, 2014/3/3 for 全局使能guet-vjudge */
DEFUN(cmd_guet_judge_enable_st, (char*)"guet-judge enable", (char*)"guet-judge enable", guet_judge_enable_st)
{
	extern int guet_vjudge_enable;

	if (OS_YES == guet_vjudge_enable)
	{
		printf("Info: guet-judge is already enable.\r\n");
		return OS_ERR;
	}

	int ret = WritePrivateProfileString("GUET_DEPT3","vjudge_enable","1",INI_filename);
	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		printf("Error: guet-judge enable failed.\r\n");
		return OS_ERR;
	}

	guet_vjudge_enable = OS_YES;

}
#endif

#if M_DES("cmd_undo_guet_judge_enable_st",1)

/* BEGIN: Added by weizengke, 2014/3/3 for 全局去使能guet-vjudge */
DEFUN(cmd_undo_guet_judge_enable_st, (char*)"undo guet-judge enable", (char*)"undo guet-judge enable", undo_guet_judge_enable_st)
{
	extern int guet_vjudge_enable;

	if (OS_NO == guet_vjudge_enable)
	{
		printf("Info: guet-judge is already disable.\r\n");
		return OS_ERR;
	}

	int ret = WritePrivateProfileString("GUET_DEPT3","vjudge_enable","0",INI_filename);
	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		printf("Error: guet-judge disable failed.\r\n");
		return OS_ERR;
	}

	guet_vjudge_enable = OS_NO;

}
#endif

#if M_DES("cmd_guet_judge_remote_judge_enable_st",1)

/* BEGIN: Added by weizengke, 2014/3/5 for 全局去使能guet-vjudge remote judge*/
DEFUN(cmd_guet_judge_remote_judge_enable_st, (char*)"guet-judge remote-judge enable",
		 (char*)"guet-judge remote-judge enable", guet_judge_enable_remote_judge_st)
{
	extern int guet_remote_enable;

	if (OS_YES == guet_remote_enable)
	{
		printf("Info: guet-remote-judge is already enable.\r\n");
		return OS_ERR;
	}

	int ret = WritePrivateProfileString("GUET_DEPT3","remote_enable","1",INI_filename);
	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		printf("Error: guet-remote-judge enable failed.\r\n");
		return OS_ERR;
	}

	guet_remote_enable = OS_YES;
}
#endif

#if M_DES("cmd_undo_guet_judge_remote_judge_enable_st",1)

/* BEGIN: Added by weizengke, 2014/3/5 for 全局去使能guet-vjudge remote judge*/
DEFUN(cmd_undo_guet_judge_remote_judge_enable_st, (char*)"undo guet-judge remote-judge enable",
		 (char*)"undo guet-judge remote-judge enable", undo_guet_judge_enable_remote_judge_st)
{
	extern int guet_remote_enable;

	if (OS_NO == guet_remote_enable)
	{
		printf("Info: guet-remote-judge is already disable.\r\n");
		return OS_ERR;
	}

	int ret = WritePrivateProfileString("GUET_DEPT3","remote_enable","0",INI_filename);
	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		printf("Error: guet-remote-judge disable failed.\r\n");
		return OS_ERR;
	}

	guet_remote_enable = OS_NO;
}
#endif

#if M_DES("cmd_hdu_judge_remote_judge_enable_st",1)

/* BEGIN: Added by weizengke, 2014/3/5 for 全局去使能hdu-vjudge remote judge*/
DEFUN(cmd_hdu_judge_remote_judge_enable_st, (char*)"hdu-judge remote-judge enable",
		 (char*)"hdu-judge remote-judge enable", hdu_judge_enable_remote_judge_st)
{
	extern int hdu_remote_enable;

	if (OS_YES == hdu_remote_enable)
	{
		printf("Info: hdu-remote-judge is already enable.\r\n");
		return OS_ERR;
	}

	int ret = WritePrivateProfileString("HDU","remote_enable","1",INI_filename);
	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		printf("Error: hdu-remote-judge enable failed.\r\n");
		return OS_ERR;
	}

	hdu_remote_enable = OS_YES;
}
#endif

#if M_DES("cmd_undo_hdu_judge_remote_judge_enable_st",1)

/* BEGIN: Added by weizengke, 2014/3/5 for 全局去使能guet-vjudge remote judge*/
DEFUN(cmd_undo_hdu_judge_remote_judge_enable_st, (char*)"undo hdu-judge remote-judge enable",
		 (char*)"undo hdu-judge remote-judge enable", undo_hdu_judge_enable_remote_judge_st)
{
	extern int hdu_remote_enable;

	if (OS_NO == hdu_remote_enable)
	{
		printf("Info: hdu-remote-judge is already disable.\r\n");
		return OS_ERR;
	}

	int ret = WritePrivateProfileString("HDU","remote_enable","0",INI_filename);
	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		printf("Error: hdu-remote-judge disable failed.\r\n");
		return OS_ERR;
	}

	hdu_remote_enable = OS_NO;
}
#endif

#if M_DES("cmd_hdu_judge_username_password_st",1)

/* BEGIN: Added by weizengke, 2014/3/5 judge account config*/
DEFUN(cmd_hdu_judge_username_password_st, (char*)"hdu-judge username STRING<1-24> password STRING<1-24>",
		(char*)"hdu-judge username and password", hdu_judge_username_password_st)
{
	char name[25] = {0};
	char psw[25]={0};
	extern char hdu_username[];
	extern char hdu_password[];

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "%d %s %s %s %s %s.\n", argc, argv[0], argv[1], argv[2], argv[3], argv[4]);

	strcpy(name, argv[2]);
	strcpy(psw, argv[4]);

	int ret = WritePrivateProfileString("HDU","username",name,INI_filename);
	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		printf("Error: set hdu-judge username failed.\r\n");
		return OS_ERR;
	}

	ret = WritePrivateProfileString("HDU","password",psw,INI_filename);
	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		printf("Error: set hdu-judge password failed.\r\n");
		/* 回滚 */
		(void)WritePrivateProfileString("HDU","username",hdu_username,INI_filename);

		return OS_ERR;
	}

	strcpy(hdu_username, name);
	strcpy(hdu_password, psw);

}
#endif

#if M_DES("cmd_guet_judge_username_password_st",1)

/* BEGIN: Added by weizengke, 2014/3/5 judge account config*/
DEFUN(cmd_guet_judge_username_password_st, (char*)"guet-judge username STRING<1-24> password STRING<1-24>",
		(char*)"guet-judge username and password", guet_judge_username_password_st)
{
	char name[25] = {0};
	char psw[25]={0};

	extern char guet_username[];
	extern char guet_password[];

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "%d %s %s %s %s %s.\n", argc, argv[0], argv[1], argv[2], argv[3], argv[4]);

	strcpy(name, argv[2]);
	strcpy(psw, argv[4]);

	int ret = WritePrivateProfileString("GUET_DEPT3","username",name,INI_filename);
	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		printf("Error: set guet-judge username failed.\r\n");
		return OS_ERR;
	}

	ret = WritePrivateProfileString("GUET_DEPT3","password",psw,INI_filename);
	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		printf("Error: set guet-judge password failed.\r\n");
		/* 回滚 */
		(void)WritePrivateProfileString("GUET_DEPT3","username",guet_username,INI_filename);

		return OS_ERR;
	}

	strcpy(guet_username, name);
	strcpy(guet_password, psw);

}
#endif

#if M_DES("cmd_hdu_judge_ip_port_st",1)

/* BEGIN: Added by weizengke, 2014/3/5 judge judger config*/
DEFUN(cmd_hdu_judge_ip_port_st, (char*)"hdu-judge ip STRING<1-24> port INTEGER<1-65535>",
		(char*)"hdu-judge ip and port", hdu_judge_ip_port_st)
{
	char ip[25] = {0};
	char port[20] = {0};

	extern char hdu_judgerIP[];
	extern int hdu_sockport;

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "%d %s %s %s %s %s.\n", argc, argv[0], argv[1], argv[2], argv[3], argv[4]);

	strcpy(ip, argv[2]);
	strcpy(port, argv[4]);

	int ret = WritePrivateProfileString("HDU","judgerIP",ip,INI_filename);
	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		printf("Error: set hdu-judge judgerIP failed.\r\n");
		return OS_ERR;
	}

	ret = WritePrivateProfileString("HDU","socketport",port,INI_filename);
	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		printf("Error: set hdu-judge socketport failed.\r\n");
		/* 回滚 */
		(void)WritePrivateProfileString("HDU","judgerIP",hdu_judgerIP,INI_filename);

		return OS_ERR;
	}

	strcpy(hdu_judgerIP, ip);
	hdu_sockport = atoi(port);

}
#endif

#if M_DES("cmd_guet_judge_ip_port_st",1)

/* BEGIN: Added by weizengke, 2014/3/5 judge judger config*/
DEFUN(cmd_guet_judge_ip_port_st, (char*)"guet-judge ip STRING<1-24> port INTEGER<1-65535>",
		(char*)"guet-judge ip and port", guet_judge_ip_port_st)
{
	char ip[25] = {0};
	char port[20] = {0};

	extern char guet_judgerIP[];
	extern int guet_sockport;

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "%d %s %s %s %s %s.\n", argc, argv[0], argv[1], argv[2], argv[3], argv[4]);

	strcpy(ip, argv[2]);
	strcpy(port, argv[4]);

	int ret = WritePrivateProfileString("GUET_DEPT3","judgerIP",ip,INI_filename);
	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		printf("Error: set guet-judge judgerIP failed.\r\n");
		return OS_ERR;
	}

	ret = WritePrivateProfileString("GUET_DEPT3","socketport",port,INI_filename);
	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		printf("Error: set guet-judge socketport failed.\r\n");
		/* 回滚 */
		(void)WritePrivateProfileString("GUET_DEPT3","judgerIP",guet_judgerIP,INI_filename);

		return OS_ERR;
	}

	strcpy(guet_judgerIP, ip);
	guet_sockport = atoi(port);

}
#endif

#if M_DES("cmd_display_judge_brief_st",1)

/* BEGIN: Added by weizengke, 2014/3/5 judge config brief*/
DEFUN(cmd_display_judge_brief_st, (char*)"display judge brief",
		(char*)"display judge brief", display_judge_brief_st)
{
	extern int GL_vjudge_enable;

	extern char hdu_username[];
	extern char hdu_password[];
	extern int hdu_vjudge_enable;
	extern int hdu_remote_enable;
	extern int hdu_sockport;
	extern char hdu_judgerIP[];

	extern char guet_username[];
	extern char guet_password[];
	extern int guet_vjudge_enable;
	extern int guet_remote_enable;
	extern int guet_sockport;
	extern char guet_judgerIP[];

	extern int port;
	extern char Mysql_url[];
	extern char Mysql_username[];
	extern char Mysql_password[];
	extern char Mysql_table[];
	extern int	Mysql_port;

	printf("# Local Judger Info\r\n");
	printf("  Port: %d\r\n", port);

	printf(" =================================================="
		   "========================\r\n");

	printf("# Virtual Judger Info\r\n");
	printf("  Global Virtual Judge Is %s\r\n",
		  (GL_vjudge_enable==OS_YES)?"Enable":"Disable");
	printf("  Judger | Account | Password | Status | Remote |"
		   "   Judger-IP   | J-Port\r\n");
	printf("  -------------------------------------------------"
		   "------------------------\r\n");

	printf(" %-8s %-10s %-10s %-8s %-8s %-15s %-8d\r\n",
		  " HDU ", hdu_username, hdu_password,
		   (hdu_vjudge_enable==OS_YES)?"Enable":"Disable",
		   (hdu_remote_enable==OS_YES)?"Enable":"Disable",
		   hdu_judgerIP, hdu_sockport);

	printf(" %-8s %-10s %-10s %-8s %-8s %-15s %-8d\r\n",
		  " GUET3", guet_username, guet_password,
		   (guet_vjudge_enable==OS_YES)?"Enable":"Disable",
		   (guet_remote_enable==OS_YES)?"Enable":"Disable",
		   guet_judgerIP, guet_sockport);

	printf(" =================================================="
		   "========================\r\n");

	printf("# MySQL Info\r\n"
		  "  URL       : %s\r\n"
		  "  Username  : %s\r\n"
		  "  Password  : %s\r\n"
		  "  Table-Name: %s\r\n"
		  "  Port      : %d\r\n",
		Mysql_url,Mysql_username,Mysql_password,Mysql_table,Mysql_port);

	printf(" =================================================="
		   "========================\r\n");

}
#endif

#if M_DES("cmd_reboot_st",1)

/* BEGIN: Added by weizengke, 2014/3/3 reset */
DEFUN(cmd_reboot_st, (char*)"reboot", (char*)"reboot", reboot_st)
{
	extern void Judge_Destroy();
	Judge_Destroy();

	extern int OJ_InitData();
	OJ_InitData();

	printf("Info: reboot ok.\r\n");
}
#endif

void cmd_install()
{
	/* reg cmd-element */
	cmd_reg_newcmdelement(CMD_ELEM_ID_CR, 			CMD_ELEM_TYPE_END,			CMD_END,			    ""               );
	cmd_reg_newcmdelement(CMD_ELEM_ID_STRING1TO24,  CMD_ELEM_TYPE_STRING,       "STRING<1-24>",     "String lenth range form 1 to 24");
	cmd_reg_newcmdelement(CMD_ELEM_ID_STRING1TO65535,  CMD_ELEM_TYPE_STRING,    "STRING<1-65535>",  "String lenth range form 1 to 65535");
	cmd_reg_newcmdelement(CMD_ELEM_ID_INTEGER1TO24, CMD_ELEM_TYPE_INTEGER,      "INTEGER<1-100>",   "Integer range form 1 to 100");
	cmd_reg_newcmdelement(CMD_ELEM_ID_INTEGER1TO65535, CMD_ELEM_TYPE_INTEGER,   "INTEGER<1-65535>", "Integer range form 1 to 65535");
	cmd_reg_newcmdelement(CMD_ELEM_ID_COMMAND_TREE, CMD_ELEM_TYPE_KEY,          "command-tree",     "Command tree");

	cmd_reg_newcmdelement(CMD_ELEM_ID_CURRENT_CFG,  CMD_ELEM_TYPE_KEY,          "current-configuration",     "Current Configuration");

	cmd_reg_newcmdelement(CMD_ELEM_ID_SYSNAME, 		CMD_ELEM_TYPE_KEY,   		"sysname",          "Set system name");
	cmd_reg_newcmdelement(CMD_ELEM_ID_UNDO, 			CMD_ELEM_TYPE_KEY,   		"undo",				"Undo operation");
	cmd_reg_newcmdelement(CMD_ELEM_ID_ENABLE, 		CMD_ELEM_TYPE_KEY,   		"enable",			"Enable operation");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DISABLE, 		CMD_ELEM_TYPE_KEY,   		"disable",			"Disable operation");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DISPLAY, 		CMD_ELEM_TYPE_KEY,   		"display",			"Display");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DEBUG,        CMD_ELEM_TYPE_KEY,   		"debugging",		"Debugging switch");
	cmd_reg_newcmdelement(CMD_ELEM_ID_ON, 			CMD_ELEM_TYPE_KEY,   		"on",				"Debug switch open");
	cmd_reg_newcmdelement(CMD_ELEM_ID_OFF, 			CMD_ELEM_TYPE_KEY,   		"off",				"Debug switch close");
	cmd_reg_newcmdelement(CMD_ELEM_ID_VERSION, 		CMD_ELEM_TYPE_KEY,   		"version",			"Show version of solfware");

	cmd_reg_newcmdelement(CMD_ELEM_ID_CLOCK,        CMD_ELEM_TYPE_KEY,   		"clock",			"Show clock now");
	cmd_reg_newcmdelement(CMD_ELEM_ID_COMPUTER, 		CMD_ELEM_TYPE_KEY,   		"computer",			"Show computer information");

	cmd_reg_newcmdelement(CMD_ELEM_ID_HISTTORY, 		CMD_ELEM_TYPE_KEY,   		"history",			"Histrory command");
	cmd_reg_newcmdelement(CMD_ELEM_ID_VJUDGE,	    CMD_ELEM_TYPE_KEY,   		"virtual-judge", 	"Virtual judge");

	cmd_reg_newcmdelement(CMD_ELEM_ID_DEBUG_ERROR,  CMD_ELEM_TYPE_KEY,			"error",			"Error");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DEBUG_FUNC,   CMD_ELEM_TYPE_KEY,			"function",			"Function");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DEBUG_INFO,   CMD_ELEM_TYPE_KEY,			"info",				"Information");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DEBUG_MSG,    CMD_ELEM_TYPE_KEY,			"message",			"Message");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DEBUG_FSM,    CMD_ELEM_TYPE_KEY,			"fsm",				"Finite State Machine");

	cmd_reg_newcmdelement(CMD_ELEM_ID_DEBUG_ALL,    CMD_ELEM_TYPE_KEY,			"all",				"All");

	cmd_reg_newcmdelement(CMD_ELEM_ID_LOGIN,     	CMD_ELEM_TYPE_KEY,			"login",			"Login");

	cmd_reg_newcmdelement(CMD_ELEM_ID_HDUJUDGE,		CMD_ELEM_TYPE_KEY,			"hdu-judge",		"HDU-Judge");
	cmd_reg_newcmdelement(CMD_ELEM_ID_GUETJUDGE,		CMD_ELEM_TYPE_KEY,			"guet-judge",		"GUET-Judge");
	cmd_reg_newcmdelement(CMD_ELEM_ID_REMOTE_JUDGE,	CMD_ELEM_TYPE_KEY,			"remote-judge",		"Remote-Judge");
	cmd_reg_newcmdelement(CMD_ELEM_ID_USERNAME, 		CMD_ELEM_TYPE_KEY,			"username", 		"Username");
	cmd_reg_newcmdelement(CMD_ELEM_ID_PASSWORD,		CMD_ELEM_TYPE_KEY,			"password",			"Password");


	cmd_reg_newcmdelement(CMD_ELEM_ID_STATUS,		CMD_ELEM_TYPE_KEY,			"status",			"Status");

	cmd_reg_newcmdelement(CMD_ELEM_ID_JUDGE,			CMD_ELEM_TYPE_KEY,			"judge",			"Judge of OJ");
	cmd_reg_newcmdelement(CMD_ELEM_ID_SOLUTION,		CMD_ELEM_TYPE_KEY,			"solution",			"The Solution");

	cmd_reg_newcmdelement(CMD_ELEM_ID_PROBLEM,		CMD_ELEM_TYPE_KEY,			"problem",			"The Problem of OJ");

	cmd_reg_newcmdelement(CMD_ELEM_ID_SET,			CMD_ELEM_TYPE_KEY,			"set",				"Set value");
	cmd_reg_newcmdelement(CMD_ELEM_ID_CONFIG,		CMD_ELEM_TYPE_KEY,			"config",			"Set Config section name value");

	cmd_reg_newcmdelement(CMD_ELEM_ID_REBOOT,		CMD_ELEM_TYPE_KEY,			"reboot",			"Reboot Judge kernel");
	cmd_reg_newcmdelement(CMD_ELEM_ID_BRIEF,			CMD_ELEM_TYPE_KEY,			"brief",			"Brief Information");
	cmd_reg_newcmdelement(CMD_ELEM_ID_IP,			CMD_ELEM_TYPE_KEY,			"ip",			    "IP");
	cmd_reg_newcmdelement(CMD_ELEM_ID_PORT,			CMD_ELEM_TYPE_KEY,			"port",			    "Port");

	// install command
	// ---------------------------------------------------

	install_element(&cmd_sysname_st);

	install_element(&cmd_debugging_enable_st);
 	install_element(&cmd_undo_debugging_enable_st);

 	install_element(&cmd_display_clock_st);
 	install_element(&cmd_display_computer_st);
 	install_element(&cmd_display_version_st);
 	install_element(&cmd_display_history_st);
	install_element(&cmd_display_history_n_st);

	install_element(&cmd_display_st);

	install_element(&cmd_debugging_error_st);
	install_element(&cmd_undo_debugging_error_st);

	install_element(&cmd_debugging_function_st);
	install_element(&cmd_undo_debugging_function_st);

	install_element(&cmd_debugging_info_st);
	install_element(&cmd_undo_debugging_info_st);

	install_element(&cmd_debugging_message_st);
	install_element(&cmd_undo_debugging_message_st);

	install_element(&cmd_debugging_fsm_st);
	install_element(&cmd_undo_debugging_fsm_st);


	install_element(&cmd_debugging_all_st);
	install_element(&cmd_undo_debugging_all_st);

	install_element(&cmd_display_debugging_st);

	install_element(&cmd_hdujudge_login_st);
	install_element(&cmd_display_hdujudge_status_st);
	install_element(&cmd_display_guetjudge_status_st);
	install_element(&cmd_guetjudge_login_st);

	install_element(&cmd_judge_solution_st);

	install_element(&cmd_display_command_tree_st);

	install_element(&cmd_display_current_configuration_st);

	install_element(&cmd_display_hdu_judge_problem_by_pid_st);

	install_element(&cmd_set_config_section_name_value_st);

	install_element(&cmd_virtual_judge_enable_st);
	install_element(&cmd_undo_virtual_judge_enable_st);
	install_element(&cmd_guet_judge_enable_st);
	install_element(&cmd_undo_guet_judge_enable_st);
	install_element(&cmd_hdu_judge_enable_st);
	install_element(&cmd_undo_hdu_judge_enable_st);

	install_element(&cmd_guet_judge_remote_judge_enable_st);
	install_element(&cmd_undo_guet_judge_remote_judge_enable_st);
	install_element(&cmd_hdu_judge_remote_judge_enable_st);
	install_element(&cmd_undo_hdu_judge_remote_judge_enable_st);

	install_element(&cmd_guet_judge_username_password_st);
	install_element(&cmd_hdu_judge_username_password_st);

	install_element(&cmd_display_judge_brief_st);

	install_element(&cmd_guet_judge_ip_port_st);
	install_element(&cmd_hdu_judge_ip_port_st);

	install_element(&cmd_reboot_st);
	// ---------------------------------------------------

}
