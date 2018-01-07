
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

#include "product\judge\include\judge_inc.h"

extern char hdu_domain[];
extern char hdu_username[];
extern char hdu_password[];
extern char guet_username[];
extern char guet_password[];
extern char hdu_judgerIP[];
extern int hdu_sockport;
extern char guet_judgerIP[];
extern int guet_sockport;
extern int g_vjudge_enable;
extern int hdu_vjudge_enable;
extern int hdu_remote_enable;
extern int guet_vjudge_enable;
extern int guet_remote_enable;
extern int g_sock_port;
extern char dataPath[];
extern int g_judge_mode;
extern char Mysql_url[];
extern char Mysql_username[];
extern char Mysql_password[];
extern char Mysql_table[];
extern int	Mysql_port;

/* debug 注册相关 */
#define DEBUG_CMD_CAT(x,y,z)      	   #x" "#y" "#z
#define DEBUG_UNDO_CMD_CAT(u, x,y,z)   #u" "#x" "#y" "#z
#define Debug_RegModule(MID, moudle_name) \
CMD_DEFINE(cmd_debugging_##moudle_name##_message_st, DEBUG_CMD_CAT(debugging,moudle_name,message), DEBUG_CMD_CAT(debugging,moudle_name,message), fun_debugging_##moudle_name##_message)\
{\
	DEBUG_MASK_SET(MID, DEBUG_TYPE_MSG);\
	return 0;\
}\
CMD_DEFINE(undo_cmd_debugging_##moudle_name##_message_st, DEBUG_UNDO_CMD_CAT(undo,debugging,moudle_name,message), DEBUG_UNDO_CMD_CAT(undo,debugging,moudle_name,message), fun_undo_debugging_##moudle_name##_message)\
{\
	DEBUG_MASK_CLEAR(MID, DEBUG_TYPE_MSG);\
	return 0;\
}\	
CMD_DEFINE(cmd_debugging_##moudle_name##_info_st, DEBUG_CMD_CAT(debugging,moudle_name,info), DEBUG_CMD_CAT(debugging,moudle_name,info), fun_debugging_##moudle_name##_info)\
{\
	DEBUG_MASK_SET(MID, DEBUG_TYPE_INFO);\
	return 0;\
}\
CMD_DEFINE(undo_cmd_debugging_##moudle_name##_info_st, DEBUG_UNDO_CMD_CAT(undo,debugging,moudle_name,info), DEBUG_UNDO_CMD_CAT(undo,debugging,moudle_name,info), fun_undo_debugging_##moudle_name##_info)\
{\
	DEBUG_MASK_CLEAR(MID, DEBUG_TYPE_INFO);\
	return 0;\
}\
CMD_DEFINE(cmd_debugging_##moudle_name##_error_st, DEBUG_CMD_CAT(debugging,moudle_name,error), DEBUG_CMD_CAT(debugging,moudle_name,error), fun_debugging_##moudle_name##_error)\
{\
	DEBUG_MASK_SET(MID, DEBUG_TYPE_ERROR);\
	return 0;\
}\
CMD_DEFINE(undo_cmd_debugging_##moudle_name##_error_st, DEBUG_UNDO_CMD_CAT(undo,debugging,moudle_name,error), DEBUG_UNDO_CMD_CAT(undo,debugging,moudle_name,error), fun_undo_debugging_##moudle_name##_error)\
{\
	DEBUG_MASK_CLEAR(MID, DEBUG_TYPE_ERROR);\
	return 0;\
}\
CMD_DEFINE(cmd_debugging_##moudle_name##_function_st, DEBUG_CMD_CAT(debugging,moudle_name,function), DEBUG_CMD_CAT(debugging,moudle_name,function), fun_debugging_##moudle_name##_function)\
{\
	DEBUG_MASK_SET(MID, DEBUG_TYPE_FUNC);\
	return 0;\
}\
CMD_DEFINE(undo_cmd_debugging_##moudle_name##_function_st, DEBUG_UNDO_CMD_CAT(undo,debugging,moudle_name,function), DEBUG_UNDO_CMD_CAT(undo,debugging,moudle_name,function), fun_undo_debugging_##moudle_name##_function)\
{\
	DEBUG_MASK_CLEAR(MID, DEBUG_TYPE_FUNC);\
	return 0;\
}\
CMD_DEFINE(cmd_debugging_##moudle_name##_all_st, DEBUG_CMD_CAT(debugging,moudle_name,all), DEBUG_CMD_CAT(debugging,moudle_name,all), fun_debugging_##moudle_name##_all)\
{\
	int i;\
	int m= 0;\
	for (i = DEBUG_TYPE_NONE + 1; i < DEBUG_TYPE_MAX; i++ ){\
		DEBUG_MASK_SET(MID, i);\
	}\
	return 0;\
}\
CMD_DEFINE(undo_cmd_debugging_##moudle_name##_all_st, DEBUG_UNDO_CMD_CAT(undo,debugging,moudle_name,all), DEBUG_UNDO_CMD_CAT(undo,debugging,moudle_name,all), fun_undo_debugging_##moudle_name##_all)\
{\
	int i;\
	int m= 0;\
	for (i = DEBUG_TYPE_NONE + 1; i < DEBUG_TYPE_MAX; i++ ){\
			DEBUG_MASK_CLEAR(MID, i);\
	}\
	return 0;\
}\
void _____##moudle_name ();\
void _____##moudle_name ()\
{\	
	install_command(VIEW_USER, &cmd_debugging_##moudle_name##_all_st);\
	install_command(VIEW_USER, &undo_cmd_debugging_##moudle_name##_all_st);\	
	install_command(VIEW_USER, &cmd_debugging_##moudle_name##_info_st);\
	install_command(VIEW_USER, &undo_cmd_debugging_##moudle_name##_info_st);\
	install_command(VIEW_USER, &cmd_debugging_##moudle_name##_error_st);\
	install_command(VIEW_USER, &undo_cmd_debugging_##moudle_name##_error_st);\
	install_command(VIEW_USER, &cmd_debugging_##moudle_name##_function_st);\
	install_command(VIEW_USER, &undo_cmd_debugging_##moudle_name##_function_st);\	
	install_command(VIEW_USER, &cmd_debugging_##moudle_name##_message_st);\
	install_command(VIEW_USER, &undo_cmd_debugging_##moudle_name##_message_st);\
}\

#define Debug_ModuleCmdInstall(moudle_name) _____##moudle_name();

#if M_DES("cmd_terminal_debugging_st",1)
CMD_DEFINE(cmd_terminal_debugging_st, (char*)"terminal debugging", (char*)"Ternimal debugging switch on", fun_cmd_terminal_debugging)
{
	vty->user.terminal_debugging = OS_YES;
	return 0;
}
#endif
#if M_DES("cmd_undo_terminal_debugging_st",1)
CMD_DEFINE(cmd_undo_terminal_debugging_st, (char*)"undo terminal debugging", (char*)"Ternimal debugging switch off", fun_cmd_undo_terminal_debugging)
{
	vty->user.terminal_debugging = OS_NO;
	return 0;
}
#endif

#if M_DES("cmd_debugging_enable_st",1)
CMD_DEFINE(cmd_debugging_enable_st, (char*)"debugging enable", (char*)"Debugging switch on", Debugging_enable)
{
	extern int g_oj_debug_switch;
	if (g_debug_switch == DEBUG_ENABLE)
	{
		vty_printf(vty, "Info: debugging switch is already enable.\r\n");
		return 0;
	}

	g_debug_switch = DEBUG_ENABLE;
	g_oj_debug_switch = DEBUG_ENABLE;

	extern ULONG Judge_DebugSwitch(ULONG st);
	Judge_DebugSwitch(DEBUG_ENABLE);
	vty_printf(vty, "Info: debugging switch is enable.\r\n");

	return 0;
}
#endif

#if M_DES("cmd_undo_debugging_enable_st",1)
CMD_DEFINE(cmd_undo_debugging_enable_st, (char*)"undo debugging enable", (char*)"Debugging switch off", undo_debugging_enable)
{
	extern int g_oj_debug_switch;

	if (g_debug_switch == DEBUG_DISABLE)
	{
		vty_printf(vty, "Info: debugging switch is already disable.\r\n");
		return 0;
	}

	g_debug_switch = DEBUG_DISABLE;
	g_oj_debug_switch = DEBUG_DISABLE;

	extern ULONG Judge_DebugSwitch(ULONG st);
	Judge_DebugSwitch(DEBUG_DISABLE);

	vty_printf(vty, "Info: debugging switch is disable.\r\n");

	return 0;
}
#endif

#if M_DES("cmd_debugging_all_st",1)
CMD_DEFINE(cmd_debugging_all_st, (char*)"debugging all", (char*)"open debugging all switch", debugging_all)
{
	int i;
	int m= 0;
	for (m = MID_NULL+1; m < MID_ID_END; m++)
	{
		for (i = DEBUG_TYPE_NONE + 1; i < DEBUG_TYPE_MAX; i++ )
		{
			DEBUG_MASK_SET(m, i);
		}
	}
	
	return 0;
}
#endif

#if M_DES("cmd_undo_debugging_all_st",1)
CMD_DEFINE(cmd_undo_debugging_all_st, (char*)"undo debugging all", (char*)"close debugging all switch", undo_debugging_all)
{
	int i;
	int m= 0;
	for (m = MID_NULL+1; m < MID_ID_END; m++)
	{
		for (i = DEBUG_TYPE_NONE + 1; i < DEBUG_TYPE_MAX; i++ )
		{
			DEBUG_MASK_CLEAR(m, i);
		}
	}
	
	return 0;

}
#endif

#if M_DES("cmd_display_debugging_st",1)
CMD_DEFINE(cmd_display_debugging_st, (char*)"display debugging", (char*)"display debugging switch", display_debugging)
{
	int m = 0;
	int i = 0;
	if (g_debug_switch == DEBUG_ENABLE)
	{
		vty_printf(vty, "Global debugging is enable.\r\n");
	}
	else
	{
		vty_printf(vty, "Global debugging is disable.\r\n");
	}

#if 0
	vty_printf(vty, " DebugMask(0x%x", g_aulDebugMask[0]);
	for (i = 1; i < DEBUG_TYPE_MAX/DEBUG_MASKLENTG + 1 ; i++)
	{
		vty_printf(vty, "	,0x%x", g_aulDebugMask[i]);
	}
	
	vty_printf(vty, ").\r\n");
#endif
	for (m = MID_NULL+1; m < MID_ID_END; m++)
	{
		for (i = DEBUG_TYPE_NONE + 1; i < DEBUG_TYPE_MAX; i++ )
		{
			if (DEBUG_MASK_GET(m, i))
			{
				vty_printf(vty, " Debugging %s %s switch is on.\r\n", szModuleName[m], szDebugName[i]);
			}
		}
	}


	return 0;
}
#endif

#if M_DES("cmd_display_clock_st",1)
CMD_DEFINE(cmd_display_clock_st, (char*)"display clock", (char*)"Display clock of device", display_clock)
{
	time_t	timep = time(NULL);
	struct tm *p;

	p = localtime(&timep);
	p->tm_year = p->tm_year + 1900;
	p->tm_mon = p->tm_mon + 1;

	vty_printf(vty, " Date of device:\r\n %04d-%02d-%02d %02d:%02d:%02d UTC(+8) DTS\n",p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);

	return 0;
}
#endif

#if M_DES("cmd_display_computer_st",1)
CMD_DEFINE(cmd_display_computer_st, (char*)"display computer", (char*)"Display computer information", display_computer)
{
	vty_printf(vty, "This is Jungle Wei's computer.\r\n");
	return 0;
}
#endif

#if M_DES("cmd_version_st",1)
CMD_DEFINE(cmd_version_st, (char*)"version", (char*)"Display device version", version)
{
	/* support oi mode*/
	vty_printf(vty, " Kernel version: %s, released at %s %s. \r\n Copyright @ 2011-2017 debugforces.com. All Rights Reserved. \r\n",
			SOLFWARE_VERSION, __TIME__, __DATE__);
	
	return 0;
}
#endif

#if M_DES("cmd_sysname_st",1)
CMD_DEFINE(cmd_sysname_st, (char*)"sysname STRING<1-24>", (char*)"set system name", sysname)
{
	CMD_DBGASSERT(argv[1] != 0, "sysname");

	strcpy(g_sysname, argv[1]);

#if 0
	WritePrivateProfileString("System","sysname",g_sysname,INI_filename);
#endif
	::SetConsoleTitle(g_sysname);

	vty_printf(vty, "Info: system name change to %s successful.\r\n", g_sysname);

	return 0;
}
#endif

#if M_DES("cmd_display_history_st",1)
CMD_DEFINE(cmd_display_history_st, (char*)"display history", (char*)"Display history command", display_history)
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

		vty_printf(vty, " %s\r\n", vty->history[i]);
	}

	return 0;
}
#endif

#if M_DES("cmd_display_history_n_st",1)
CMD_DEFINE(cmd_display_history_n_st, (char*)"display history INTEGER<1-100>", (char*)"Display history command", display_history_n)
{
	int n = 0;
	int i = 0;

	CMD_debug(DEBUG_TYPE_FUNC, "%d %s %s %s", argc, argv[0], argv[1], argv[2]);

	CMD_DBGASSERT(argv[2], "display_history_n");

	n = atoi(argv[2]);

	for (i = 0;  i < HISTORY_MAX_SIZE; i++)
	{
		if (vty->history[i] == NULL)
			break;
	}

	for (i = i-1; i >= 0 && n > 0; i--,n--)
	{
		if (vty->history[i] == NULL)
			break;

		vty_printf(vty, "%s \r\n", vty->history[i]);
	}

	return 0;
}

#endif

#if M_DES("cmd_judge_solution_st",1)

CMD_DEFINE(cmd_judge_solution_st, (char*)"judge solution INTEGER<1-65535>", (char*)"judge solution <ID>", judge_solution)
{
	int solutionId = 0;

	CMD_debug(DEBUG_TYPE_FUNC, "%d %s %s %s", argc, argv[0], argv[1], argv[2]);

	solutionId = atoi(argv[2]);

	extern int Judge_PushQueue(int solutionId);
	Judge_PushQueue(solutionId);

	//vty_printf(vty, "Info: Solution judge-request has been sent to Judge-Queue asynchronously.\r\n");

	return 0;
}
#endif


#if(JUDGE_VIRTUAL == VOS_YES)
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
CMD_DEFINE(cmd_hdujudge_login_st, (char*)"hdu-judge login STRING<1-24> STRING<1-24>", (char*)"hdujudge login", hdujudge_login)
{
	char username_[25] = {0};
	char pwd[25] = {0};

	CMD_debug(DEBUG_TYPE_FUNC, "%d %s %s %s\r\n", argc, argv[0], argv[1], argv[2], argv[3]);

	strcpy(username_, argv[2]);
	strcpy(pwd, argv[3]);

	extern int HDU_loginEx(char *uname, char *pdw);
	HDU_loginEx(username_, pwd);

	return 0;
}
#endif

#if M_DES("cmd_display_hdujudge_status_st",1)

CMD_DEFINE(cmd_display_hdujudge_status_st, (char*)"display hdu-judge status STRING<1-24>", (char*)"display hdujudge status <username>", display_hdujudge_status)
{
	char username_[25] = {0};

	CMD_debug(DEBUG_TYPE_FUNC, "%d %s %s %s\r\n", argc, argv[0], argv[1], argv[2], argv[3]);

	strcpy(username_, argv[3]);

	extern int getStatusEx(char *);
	getStatusEx(username_);

	return 0;
}
#endif


#if M_DES("cmd_display_guetjudge_status_st",1)

CMD_DEFINE(cmd_display_guetjudge_status_st, (char*)"display guet-judge status STRING<1-24>", (char*)"display guetjudge status <username>", display_guetjudge_status)
{
	char username_[25] = {0};

	CMD_debug(DEBUG_TYPE_FUNC, "%d %s %s %s\n", argc, argv[0], argv[1], argv[2], argv[3]);

	strcpy(username_, argv[3]);

	extern int GUET_getStatusEx(char *);
	GUET_getStatusEx(username_);

	return 0;
}
#endif

#if M_DES("cmd_guetjudge_login_st",1)
CMD_DEFINE(cmd_guetjudge_login_st, (char*)"guet-judge login STRING<1-24> STRING<1-24>", (char*)"guet-judge login <username> <password>", guetjudge_login_st)
{
	char username_[25] = {0};
	char psw_[25] = {0};

	CMD_debug(DEBUG_TYPE_FUNC, "%d %s %s %s\r\n", argc, argv[0], argv[1], argv[2], argv[3]);

	strcpy(username_, argv[2]);
	strcpy(psw_, argv[3]);

	extern int GUET_Login(char *uname, char *psw);
	GUET_Login(username_, psw_);

	return 0;
}
#endif
#if M_DES("cmd_display_hdu_judge_problem_by_pid_st",1)

CMD_DEFINE(cmd_display_hdu_judge_problem_by_pid_st, (char*)"display hdu-judge problem INTEGER<1-65535>", (char*)"display hdu-judge problem by problem ID", display_hdu_judge_problem_by_pid)
{
	int problemId = 0;

	CMD_debug(DEBUG_TYPE_FUNC, "%d %s %s %s %s\r\n", argc, argv[0], argv[1], argv[2], argv[2]);

	problemId = atoi(argv[3]);


	extern int DLL_GetProblemInfoFromHDU(int pid);

	int ret = DLL_GetProblemInfoFromHDU(problemId);
	if (OS_FALSE == ret)
	{
		vty_printf(vty, "Info: No such problam %d on hdu-judge.\r\n", problemId);
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
	judge_outstring("The color-value is %d\n", i);

	for(i=1;i<=0xFC;i++)
	{
		SetConsoleTextAttribute(consolehwnd,i);
		judge_outstring("The color-value is %d\n", i);

		/* 改变console text & backround */
		char clor[128] = {0};
		judge_outstring(clor, "color %x", i);
		system(clor);
	}

	//SetConsoleTextAttribute(consolehwnd,255);

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),23);
	judge_outstring("The color-value is %d..\n", 23);

}

void Test()
{
	HANDLE hOut;
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO bInfo;
	GetConsoleScreenBufferInfo(hOut, &bInfo );

	judge_outstring("Buf:X*Y=%u, Cur:X=%u, Y=%u\r\n"
			"WindowsSize:Bottom=%u, Left=%u, Right=%u, Top=%u\r\n",
			bInfo.dwSize.X * bInfo.dwSize.Y,
			bInfo.dwCursorPosition.X, bInfo.dwCursorPosition.Y,
			bInfo.srWindow.Bottom, bInfo.srWindow.Left, bInfo.srWindow.Right, bInfo.srWindow.Top);

	GetConsoleScreenBufferInfo(hOut, &bInfo );
	gotoxy(bInfo.dwCursorPosition.X+1, bInfo.dwCursorPosition.Y+1);

	TestColor();

}
#endif

#endif

#if M_DES("cmd_display_command_tree_st",1)

CMD_DEFINE(cmd_display_command_tree_st, (char*)"display command-tree", (char*)"display command tree", display_command_tree)
{
	extern void cmd_show_command_tree(struct cmd_vty *vty);
	cmd_show_command_tree(vty);

	return 0;
}
#endif


#if M_DES("cmd_set_config_section_name_value_st",1)

CMD_DEFINE(cmd_set_config_section_name_value_st, (char*)"set config STRING<1-24> STRING<1-24> STRING<1-65535>",
		(char*)"Set Config section name value", set_config_section_name_value_st)
{
	char section[25] = {0};
	char name[25] = {0};
	char value[65536]={0};

	CMD_debug(DEBUG_TYPE_FUNC, "%d %s %s %s %s %s.\r\n", argc, argv[0], argv[1], argv[2], argv[3], argv[4]);

	strcpy(section, argv[2]);
	strcpy(name, argv[3]);
	strcpy(value, argv[4]);

	int ret = WritePrivateProfileString(section,name,value,INI_filename);

	CMD_debug(DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

}
#endif

#if M_DES("cmd_judge_enable_st",1)
CMD_DEFINE(cmd_judge_enable_st, (char*)"judge enable", (char*)"judge enable", judge_enable_st)
{
	extern int g_judge_enable;

	if (OS_YES == g_judge_enable)
	{
		vty_printf(vty, "Info: judge is already enable.\r\n");
		return OS_ERR;
	}

	g_judge_enable = OS_YES;

}
#endif

#if M_DES("cmd_undo_judge_enable_st",1)
CMD_DEFINE(cmd_undo_judge_enable_st, (char*)"undo judge enable", (char*)"undo judge enable", undo_judge_enable_st)
{
	extern int g_judge_enable;

	if (OS_NO == g_judge_enable)
	{
		vty_printf(vty, "Info: judge is already disable.\r\n");
		return OS_ERR;
	}

	g_judge_enable = OS_NO;

}
#endif
#if M_DES("cmd_judge_datapath_st",1)

CMD_DEFINE(cmd_judge_datapath_st, (char*)"judge data-path STRING<1-256>", 
       (char*)"judge data-path", cmd_judge_datapath)
{
	char path[258]={0};
    int len = 0;
    extern char dataPath[];

    CMD_debug(DEBUG_TYPE_FUNC, "%d %s %s %s .\n", argc, argv[0], argv[1], argv[2]);

    strcpy(path, argv[2]);

    if( (_access(path, 0 )) == -1 )
    {
    	pdt_debug_print("Error: Path '%s' is not exist.", path);
        return OS_ERR;
    }
     
    #if 0
	int ret = WritePrivateProfileString("Judge","DataPath",path,INI_filename);
	if (ret != 1)
	{
		judge_outstring("Error: set judge data-path failed.\r\n");
		return OS_ERR;
	}
    #endif
    strcpy(dataPath, path);
	return 0;
}
#endif


#if M_DES("cmd_virtual_judge_enable_st",1)

/* BEGIN: Added by weizengke, 2014/3/3 for 全局使能vjudge */
CMD_DEFINE(cmd_virtual_judge_enable_st, (char*)"virtual-judge enable", (char*)"virtual-judge enable", virtual_judge_enable_st)
{
	extern int g_vjudge_enable;

	if (OS_YES == g_vjudge_enable)
	{
		vty_printf(vty, "Info: virtual-judge is already enable.\r\n");
		return OS_ERR;
	}
#if 0
	int ret = WritePrivateProfileString("Judge","vjudge_enable","1",INI_filename);
	CMD_debug(DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		judge_outstring("Error: virtual-judge enable failed.\r\n");
		return OS_ERR;
	}
#endif
	g_vjudge_enable = OS_YES;

}
#endif

#if M_DES("cmd_undo_virtual_judge_enable_st",1)

/* BEGIN: Added by weizengke, 2014/3/3 for 全局去使能vjudge */
CMD_DEFINE(cmd_undo_virtual_judge_enable_st, (char*)"undo virtual-judge enable", (char*)"undo virtual-judge enable", undo_virtual_judge_enable_st)
{
	extern int g_vjudge_enable;

	if (OS_NO == g_vjudge_enable)
	{
		vty_printf(vty, "Info: virtual-judge is already disable.\r\n");
		return OS_ERR;
	}
#if 0
	int ret = WritePrivateProfileString("Judge","vjudge_enable","0",INI_filename);
	CMD_debug(DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		judge_outstring("Error: virtual-judge disable failed.\r\n");
		return OS_ERR;
	}
#endif
	g_vjudge_enable = OS_NO;

}
#endif

#if M_DES("cmd_hdu_judge_enable_st",1)

/* BEGIN: Added by weizengke, 2014/3/3 for 全局使能hdu-vjudge */
CMD_DEFINE(cmd_hdu_judge_enable_st, (char*)"hdu-judge enable", (char*)"hdu-judge enable", hdu_judge_enable_st)
{
	extern int hdu_vjudge_enable;

	if (OS_YES == hdu_vjudge_enable)
	{
		vty_printf(vty, "Info: hdu-judge is already enable.\r\n");
		return OS_ERR;
	}
#if 0
	int ret = WritePrivateProfileString("HDU","vjudge_enable","1",INI_filename);
	CMD_debug(DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		judge_outstring("Error: hdu-judge enable failed.\r\n");
		return OS_ERR;
	}
#endif
	hdu_vjudge_enable = OS_YES;

}
#endif

#if M_DES("cmd_undo_hdu_judge_enable_st",1)

/* BEGIN: Added by weizengke, 2014/3/3 for 全局去使能hdu-vjudge */
CMD_DEFINE(cmd_undo_hdu_judge_enable_st, (char*)"undo hdu-judge enable", (char*)"undo hdu-judge enable", undo_hdu_judge_enable_st)
{
	extern int hdu_vjudge_enable;

	if (OS_NO == hdu_vjudge_enable)
	{
		vty_printf(vty, "Info: hdu-judge is already disable.\r\n");
		return OS_ERR;
	}
#if 0
	int ret = WritePrivateProfileString("HDU","vjudge_enable","0",INI_filename);
	CMD_debug(DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		judge_outstring("Error: hdu-judge disable failed.\r\n");
		return OS_ERR;
	}
#endif
	hdu_vjudge_enable = OS_NO;


}
#endif

#if M_DES("cmd_guet_judge_enable_st",1)

/* BEGIN: Added by weizengke, 2014/3/3 for 全局使能guet-vjudge */
CMD_DEFINE(cmd_guet_judge_enable_st, (char*)"guet-judge enable", (char*)"guet-judge enable", guet_judge_enable_st)
{
	extern int guet_vjudge_enable;

	if (OS_YES == guet_vjudge_enable)
	{
		vty_printf(vty, "Info: guet-judge is already enable.\r\n");
		return OS_ERR;
	}
#if 0
	int ret = WritePrivateProfileString("GUET_DEPT3","vjudge_enable","1",INI_filename);
	CMD_debug(DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		judge_outstring("Error: guet-judge enable failed.\r\n");
		return OS_ERR;
	}
#endif
	guet_vjudge_enable = OS_YES;

}
#endif

#if M_DES("cmd_undo_guet_judge_enable_st",1)

/* BEGIN: Added by weizengke, 2014/3/3 for 全局去使能guet-vjudge */
CMD_DEFINE(cmd_undo_guet_judge_enable_st, (char*)"undo guet-judge enable", (char*)"undo guet-judge enable", undo_guet_judge_enable_st)
{
	extern int guet_vjudge_enable;

	if (OS_NO == guet_vjudge_enable)
	{
		vty_printf(vty, "Info: guet-judge is already disable.\r\n");
		return OS_ERR;
	}
#if 0
	int ret = WritePrivateProfileString("GUET_DEPT3","vjudge_enable","0",INI_filename);
	CMD_debug(DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		judge_outstring("Error: guet-judge disable failed.\r\n");
		return OS_ERR;
	}
#endif
	guet_vjudge_enable = OS_NO;

}
#endif

#if M_DES("cmd_guet_judge_remote_judge_enable_st",1)

/* BEGIN: Added by weizengke, 2014/3/5 for 全局去使能guet-vjudge remote judge*/
CMD_DEFINE(cmd_guet_judge_remote_judge_enable_st, (char*)"guet-judge remote-judge enable",
		 (char*)"guet-judge remote-judge enable", guet_judge_enable_remote_judge_st)
{
	extern int guet_remote_enable;

	if (OS_YES == guet_remote_enable)
	{
		vty_printf(vty, "Info: guet-remote-judge is already enable.\r\n");
		return OS_ERR;
	}
#if 0
	int ret = WritePrivateProfileString("GUET_DEPT3","remote_enable","1",INI_filename);
	CMD_debug(DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		judge_outstring("Error: guet-remote-judge enable failed.\r\n");
		return OS_ERR;
	}
#endif
	guet_remote_enable = OS_YES;
}
#endif

#if M_DES("cmd_undo_guet_judge_remote_judge_enable_st",1)

/* BEGIN: Added by weizengke, 2014/3/5 for 全局去使能guet-vjudge remote judge*/
CMD_DEFINE(cmd_undo_guet_judge_remote_judge_enable_st, (char*)"undo guet-judge remote-judge enable",
		 (char*)"undo guet-judge remote-judge enable", undo_guet_judge_enable_remote_judge_st)
{
	extern int guet_remote_enable;

	if (OS_NO == guet_remote_enable)
	{
		vty_printf(vty, "Info: guet-remote-judge is already disable.\r\n");
		return OS_ERR;
	}
#if 0
	int ret = WritePrivateProfileString("GUET_DEPT3","remote_enable","0",INI_filename);
	CMD_debug(DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		judge_outstring("Error: guet-remote-judge disable failed.\r\n");
		return OS_ERR;
	}
#endif
	guet_remote_enable = OS_NO;
}
#endif

#if M_DES("cmd_hdu_judge_remote_judge_enable_st",1)

/* BEGIN: Added by weizengke, 2014/3/5 for 全局去使能hdu-vjudge remote judge*/
CMD_DEFINE(cmd_hdu_judge_remote_judge_enable_st, (char*)"hdu-judge remote-judge enable",
		 (char*)"hdu-judge remote-judge enable", hdu_judge_enable_remote_judge_st)
{
	extern int hdu_remote_enable;

	if (OS_YES == hdu_remote_enable)
	{
		vty_printf(vty, "Info: hdu-remote-judge is already enable.\r\n");
		return OS_ERR;
	}
#if 0
	int ret = WritePrivateProfileString("HDU","remote_enable","1",INI_filename);
	CMD_debug(DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		judge_outstring("Error: hdu-remote-judge enable failed.\r\n");
		return OS_ERR;
	}
#endif
	hdu_remote_enable = OS_YES;
}
#endif

#if M_DES("cmd_undo_hdu_judge_remote_judge_enable_st",1)

/* BEGIN: Added by weizengke, 2014/3/5 for 全局去使能guet-vjudge remote judge*/
CMD_DEFINE(cmd_undo_hdu_judge_remote_judge_enable_st, (char*)"undo hdu-judge remote-judge enable",
		 (char*)"undo hdu-judge remote-judge enable", undo_hdu_judge_enable_remote_judge_st)
{
	extern int hdu_remote_enable;

	if (OS_NO == hdu_remote_enable)
	{
		vty_printf(vty, "Info: hdu-remote-judge is already disable.\r\n");
		return OS_ERR;
	}

#if 0
	int ret = WritePrivateProfileString("HDU","remote_enable","0",INI_filename);
	CMD_debug(DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		judge_outstring("Error: hdu-remote-judge disable failed.\r\n");
		return OS_ERR;
	}
#endif

	hdu_remote_enable = OS_NO;
}
#endif

#if M_DES("cmd_hdu_judge_username_password_st",1)

/* BEGIN: Added by weizengke, 2014/3/5 judge account config*/
CMD_DEFINE(cmd_hdu_judge_username_password_st, (char*)"hdu-judge username STRING<1-24> password STRING<1-24>",
		(char*)"hdu-judge username and password", hdu_judge_username_password_st)
{
	char name[25] = {0};
	char psw[25]={0};


	CMD_debug(DEBUG_TYPE_FUNC, "%d %s %s %s %s %s.\n", argc, argv[0], argv[1], argv[2], argv[3], argv[4]);

	strcpy(name, argv[2]);
	strcpy(psw, argv[4]);

#if 0

	int ret = WritePrivateProfileString("HDU","username",name,INI_filename);
	CMD_debug(DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		judge_outstring("Error: set hdu-judge username failed.\r\n");
		return OS_ERR;
	}

	ret = WritePrivateProfileString("HDU","password",psw,INI_filename);
	CMD_debug(DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		judge_outstring("Error: set hdu-judge password failed.\r\n");
		/* 回滚 */
		(void)WritePrivateProfileString("HDU","username",hdu_username,INI_filename);

		return OS_ERR;
	}
#endif

	strcpy(hdu_username, name);
	strcpy(hdu_password, psw);

}
#endif

#if M_DES("cmd_guet_judge_username_password_st",1)

/* BEGIN: Added by weizengke, 2014/3/5 judge account config*/
CMD_DEFINE(cmd_guet_judge_username_password_st, (char*)"guet-judge username STRING<1-24> password STRING<1-24>",
		(char*)"guet-judge username and password", guet_judge_username_password_st)
{
	char name[25] = {0};
	char psw[25]={0};

	CMD_debug(DEBUG_TYPE_FUNC, "%d %s %s %s %s %s.\n", argc, argv[0], argv[1], argv[2], argv[3], argv[4]);

	strcpy(name, argv[2]);
	strcpy(psw, argv[4]);

#if 0
	int ret = WritePrivateProfileString("GUET_DEPT3","username",name,INI_filename);
	CMD_debug(DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		judge_outstring("Error: set guet-judge username failed.\r\n");
		return OS_ERR;
	}

	ret = WritePrivateProfileString("GUET_DEPT3","password",psw,INI_filename);
	CMD_debug(DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		judge_outstring("Error: set guet-judge password failed.\r\n");
		/* 回滚 */
		(void)WritePrivateProfileString("GUET_DEPT3","username",guet_username,INI_filename);

		return OS_ERR;
	}
#endif

	strcpy(guet_username, name);
	strcpy(guet_password, psw);

}
#endif

#if M_DES("cmd_hdu_judge_ip_port_st",1)

/* BEGIN: Added by weizengke, 2014/3/5 judge judger config*/
CMD_DEFINE(cmd_hdu_judge_ip_port_st, (char*)"hdu-judge ip STRING<1-24> port INTEGER<1-65535>",
		(char*)"hdu-judge ip and port", hdu_judge_ip_port_st)
{
	char ip[25] = {0};
	char port[20] = {0};

	CMD_debug(DEBUG_TYPE_FUNC, "%d %s %s %s %s %s.\n", argc, argv[0], argv[1], argv[2], argv[3], argv[4]);

	strcpy(ip, argv[2]);
	strcpy(port, argv[4]);

#if 0
	int ret = WritePrivateProfileString("HDU","judgerIP",ip,INI_filename);
	CMD_debug(DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		judge_outstring("Error: set hdu-judge judgerIP failed.\r\n");
		return OS_ERR;
	}

	ret = WritePrivateProfileString("HDU","sock_port",port,INI_filename);
	CMD_debug(DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		judge_outstring("Error: set hdu-judge socketport failed.\r\n");
		/* 回滚 */
		(void)WritePrivateProfileString("HDU","judgerIP",hdu_judgerIP,INI_filename);

		return OS_ERR;
	}
#endif

	strcpy(hdu_judgerIP, ip);
	hdu_sockport = atoi(port);

}
#endif

#if M_DES("cmd_guet_judge_ip_port_st",1)

/* BEGIN: Added by weizengke, 2014/3/5 judge judger config*/
CMD_DEFINE(cmd_guet_judge_ip_port_st, (char*)"guet-judge ip STRING<1-24> port INTEGER<1-65535>",
		(char*)"guet-judge ip and port", guet_judge_ip_port_st)
{
	char ip[25] = {0};
	char port[20] = {0};

	CMD_debug(DEBUG_TYPE_FUNC, "%d %s %s %s %s %s.\n", argc, argv[0], argv[1], argv[2], argv[3], argv[4]);

	strcpy(ip, argv[2]);
	strcpy(port, argv[4]);

#if 0
	int ret = WritePrivateProfileString("GUET_DEPT3","judgerIP",ip,INI_filename);
	CMD_debug(DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		judge_outstring("Error: set guet-judge judgerIP failed.\r\n");
		return OS_ERR;
	}

	ret = WritePrivateProfileString("GUET_DEPT3","sock_port",port,INI_filename);
	CMD_debug(DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		judge_outstring("Error: set guet-judge socketport failed.\r\n");
		/* 回滚 */
		(void)WritePrivateProfileString("GUET_DEPT3","judgerIP",guet_judgerIP,INI_filename);

		return OS_ERR;
	}
#endif

	strcpy(guet_judgerIP, ip);
	guet_sockport = atoi(port);

}
#endif

#if M_DES("cmd_display_judge_brief_st",1)

/* BEGIN: Added by weizengke, 2014/3/5 judge config brief*/
CMD_DEFINE(cmd_display_judge_brief_st, (char*)"display judge brief",
		(char*)"display judge brief", display_judge_brief_st)
{
    string strDateStr;
    extern time_t g_lastjudgetime;
	char buffTmp[65535] = {0};
	char *buff = buffTmp;
	
    API_TimeToString(strDateStr, g_lastjudgetime);

	buff += sprintf(buff, "# Local Judger Info\r\n");
	buff += sprintf(buff, "  Global Judge Is %s\r\n",
		  (g_judge_enable==OS_YES)?"Enable":"Disable");
	buff += sprintf(buff, "  Sysname   : %s\r\n", g_sysname);
	buff += sprintf(buff, "  Sock Port : %d\r\n", g_sock_port);
	buff += sprintf(buff, "  Judge Mode: %s\r\n", (g_judge_mode==JUDGE_MODE_ACM)?"ACM":"OI");
	buff += sprintf(buff, "  Data Path : %s\r\n", dataPath);
	buff += sprintf(buff, "  Last Judge: %s\r\n", strDateStr.c_str());
	buff += sprintf(buff, " =================================================="
		   "========================\r\n");

	buff += sprintf(buff, "# Virtual Judger Info\r\n");
	buff += sprintf(buff, "  Global Virtual Judge Is %s\r\n",
		  (g_vjudge_enable==OS_YES)?"Enable":"Disable");
    
	buff += sprintf(buff, "  HDU domain: %s\r\n", hdu_domain);

          
	buff += sprintf(buff, "  Judger | Account | Password | Status | Remote |"
		   "   Judger-IP   | J-Port\r\n");
	buff += sprintf(buff, "  -------------------------------------------------"
		   "------------------------\r\n");

	buff += sprintf(buff, " %-8s %-10s %-10s %-8s %-8s %-15s %-8d\r\n",
		  " HDU ", hdu_username, hdu_password,
		   (hdu_vjudge_enable==OS_YES)?"Enable":"Disable",
		   (hdu_remote_enable==OS_YES)?"Enable":"Disable",
		   hdu_judgerIP, hdu_sockport);
#if 0
	buff += sprintf(buff, " %-8s %-10s %-10s %-8s %-8s %-15s %-8d\r\n",
		  " GUET3", guet_username, guet_password,
		   (guet_vjudge_enable==OS_YES)?"Enable":"Disable",
		   (guet_remote_enable==OS_YES)?"Enable":"Disable",
		   guet_judgerIP, guet_sockport);
#endif

	buff += sprintf(buff, " =================================================="
		   "========================\r\n");

	buff += sprintf(buff, "# MySQL Info\r\n"
		  "  URL       : %s\r\n"
		  "  Username  : %s\r\n"
		  "  Password  : %s\r\n"
		  "  Table-Name: %s\r\n"
		  "  Port      : %d\r\n",
		Mysql_url,Mysql_username,Mysql_password,Mysql_table,Mysql_port);

	buff += sprintf(buff, " =================================================="
		   "========================\r\n");

	vty_printf(vty, buffTmp);

}
#endif

#if M_DES("cmd_judge_mode_acm",1)
CMD_DEFINE(cmd_judge_mode_acm, (char*)"judge mode acm",
		(char*)"judge mode acm", judge_mode_acm)
{
	extern int g_judge_mode;

	if (JUDGE_MODE_ACM == g_judge_mode)
	{
		vty_printf(vty, "Info: judge mode is already acm.\r\n");
		return OS_ERR;
	}
	
#if 0
	int ret = WritePrivateProfileString("Judge","judge_mode","0",INI_filename);
	CMD_debug(DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		judge_outstring("Error: judge mode acm failed.\r\n");
		return OS_ERR;
	}
#endif

	g_judge_mode = JUDGE_MODE_ACM;

}
#endif

#if M_DES("cmd_judge_mode_oi",1)
CMD_DEFINE(cmd_judge_mode_oi, (char*)"judge mode oi",
		(char*)"judge mode oi", judge_mode_oi)
{
	extern int g_judge_mode;

	if (JUDGE_MODE_OI == g_judge_mode)
	{
		vty_printf(vty, "Info: judge mode is already oi.\r\n");
		return OS_ERR;
	}
	
#if 0
	int ret = WritePrivateProfileString("Judge","judge_mode","1",INI_filename);
	CMD_debug(DEBUG_TYPE_FUNC, "WritePrivateProfileString return %u..", ret);

	if (ret != 1)
	{
		judge_outstring("Error: judge mode acm failed.\r\n");
		return OS_ERR;
	}
#endif

	g_judge_mode = JUDGE_MODE_OI;

}
#endif

#if M_DES("cmd_judge_auto_detect_enable",1)
CMD_DEFINE(cmd_judge_auto_detect_enable, (char*)"judge auto-detect enable",
		(char*)"judge auto-detect enable", judge_auto_detect_enable)
{
	extern int g_judge_timer_enable;

	if (OS_YES == g_judge_timer_enable)
	{
		vty_printf(vty, "Info: judge auto-detect is already enale.\r\n");
		return OS_ERR;
	}

	g_judge_timer_enable = OS_YES;

}
#endif

#if M_DES("cmd_undo_judge_auto_detect_enable",1)
CMD_DEFINE(cmd_undo_judge_auto_detect_enable, (char*)"undo judge auto-detect enable",
		(char*)"undo judge auto-detect enable", undo_judge_auto_detect_enable)
{
	extern int g_judge_timer_enable;

	if (OS_NO == g_judge_timer_enable)
	{
		vty_printf(vty, "Info: judge auto-detect is already disable.\r\n");
		return OS_ERR;
	}

	g_judge_timer_enable = OS_NO;

}
#endif
#if M_DES("cmd_judge_auto_detect_interval",1)
CMD_DEFINE(cmd_judge_auto_detect_interval, (char*)"judge auto-detect interval INTEGER<1-65535>",
		(char*)"judge auto-detect interval", judge_auto_detect_interval)
{
	extern int g_judge_auto_detect_interval;

	g_judge_auto_detect_interval = atoi(argv[3]);
}
#endif

#if M_DES("cmd_ndp_server_enable",1)
CMD_DEFINE(cmd_ndp_server_enable, (char*)"ndp server enable",
		(char*)"ndp server enable", ndp_server_enable)
{
	extern ULONG NDP_SH_ServerEnable();
	NDP_SH_ServerEnable();
}
#endif
#if M_DES("cmd_undo_ndp_server_enable",1)
CMD_DEFINE(cmd_undo_ndp_server_enable, (char*)"undo ndp server enable",
		(char*)"undo ndp server enable", undo_ndp_server_enable)
{
	extern ULONG NDP_ServerDisable();
	NDP_ServerDisable();

}
#endif

#if M_DES("cmd_ndp_client_enable",1)
CMD_DEFINE(cmd_ndp_client_enable, (char*)"ndp client enable",
		(char*)"ndp client enable", ndp_client_enable)
{
	extern ULONG NDP_SH_ClientEnable();
	NDP_SH_ClientEnable();
}
#endif
#if M_DES("cmd_undo_ndp_client_enable",1)
CMD_DEFINE(cmd_undo_ndp_client_enable, (char*)"undo ndp client enable",
		(char*)"undo ndp client enable", undo_ndp_client_enable)
{
	extern ULONG NDP_ClientDisable();
	NDP_ClientDisable();

}
#endif

#if M_DES("cmd_ndp_server_ip_port",1)
CMD_DEFINE(cmd_ndp_server_ip_port, (char*)"ndp server ip STRING<1-24> port INTEGER<1-65535>",
		(char*)"ndp server ip and port", ndp_server_ip_port)
{
	char ip[25] = {0};
	char port[20] = {0};

	CMD_debug(DEBUG_TYPE_FUNC, "cmd_ndp_server_ip_port: %d %s %s %s %s %s %s.\n", argc, argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);

	strcpy(ip, argv[3]);
	strcpy(port, argv[5]);

	extern void NDP_SetServerIPPort(char *ip, int port);
	NDP_SetServerIPPort(ip, atoi(port));
}
#endif

#if M_DES("cmd_ndp_server_bind_port",1)
CMD_DEFINE(cmd_ndp_server_bind_port, (char*)"ndp server bind port INTEGER<1-65535>",
		(char*)"ndp server bind port", ndp_server_bind_port)
{
	char port[20] = {0};

	CMD_debug(DEBUG_TYPE_FUNC, "cmd_ndp_server_bind_port: %d %s %s %s %s %s.\n", argc, argv[0], argv[1], argv[2], argv[3], argv[4]);

	strcpy(port, argv[4]);

	extern void NDP_SetServerBindPort(int port);
	NDP_SetServerBindPort(atoi(port));
}
#endif

#if M_DES("cmd_display_ndp_neighbor",1)
CMD_DEFINE(cmd_display_ndp_neighbor, (char*)"display ndp neighbor",
		(char*)"display ndp neighbor", display_ndp_neighbor)
{
	extern void NDP_ShowAllNeighbors(struct cmd_vty *vty);
	NDP_ShowAllNeighbors(vty);
}
#endif

#if M_DES("cmd_telnet_server_enable",1)
CMD_DEFINE(cmd_telnet_server_enable, (char*)"telnet server enable",
		(char*)"telnet server enable", telnet_server_enable)
{
	ULONG ulRet = OS_OK;
	
	extern ULONG TELNET_ServerEnable();
	ulRet = TELNET_ServerEnable();
	if (OS_OK != ulRet)
	{
		vty_printf(vty, "Error: Telnet server enable failed.\r\n");
	}

}
#endif

#if M_DES("cmd_undo_telnet_server_enable",1)
CMD_DEFINE(cmd_undo_telnet_server_enable, (char*)"undo telnet server enable",
		(char*)"undo telnet server enable", undo_telnet_server_enable)
{
	ULONG ulRet = OS_OK;
	
	extern ULONG TELNET_ServerDisable();
	ulRet = TELNET_ServerDisable();
	if (OS_OK != ulRet)
	{
		vty_printf(vty, "Error: Telnet server disable failed.\r\n");
	}
	
}
#endif

#if M_DES("cmd_telnet_authentication_mode_none",1)
CMD_DEFINE(cmd_telnet_authentication_mode_none, (char*)"telnet authentication-mode none",
		(char*)"telnet authentication-mode none", telnet_authentication_mode_none)
{
	extern ULONG g_TelnetAuthMode ;
	extern char g_szTelnetUsername[32];
	extern char g_szTelnetPassword[32];

	g_TelnetAuthMode = 0;

	memset(g_szTelnetUsername, 0, sizeof(g_szTelnetUsername));
	memset(g_szTelnetPassword, 0, sizeof(g_szTelnetPassword));
}
#endif

#if M_DES("cmd_telnet_authentication_mode_password",1)
CMD_DEFINE(cmd_telnet_authentication_mode_password, (char*)"telnet authentication-mode password",
		(char*)"telnet authentication-mode password", telnet_authentication_mode_password)
{
	extern ULONG g_TelnetAuthMode ;
	
	g_TelnetAuthMode = 1;
	
	vty_printf(vty, "Info: Please create telnet username and password.\r\n");
}
#endif

#if M_DES("cmd_telnet_authentication_mode_aaa",1)
CMD_DEFINE(cmd_telnet_authentication_mode_aaa, (char*)"telnet authentication-mode aaa",
		(char*)"telnet authentication-mode aaa", telnet_authentication_mode_aaa)
{
	extern ULONG g_TelnetAuthMode ;
	extern char g_szTelnetUsername[32];
	extern char g_szTelnetPassword[32];

	g_TelnetAuthMode = 2;
	memset(g_szTelnetUsername, 0, sizeof(g_szTelnetUsername));
	memset(g_szTelnetPassword, 0, sizeof(g_szTelnetPassword));
	
	vty_printf(vty, "Info: Please create AAA username and password.\r\n");
}
#endif


#if M_DES("cmd_telnet_username_password",1)
CMD_DEFINE(cmd_telnet_username_password, (char*)"telnet username STRING<1-32> password STRING<1-32>",
		(char*)"telnet username password", telnet_username_password)
{
	extern ULONG g_TelnetAuthMode ;
	extern char g_szTelnetUsername[32];
	extern char g_szTelnetPassword[32];

	if (g_TelnetAuthMode == 0)
	{
		vty_printf(vty, "Error: The telnet authentication-mode is none, please change to mode password.\r\n");
		return OS_ERR;
	}

	strcpy(g_szTelnetUsername, argv[2]);
	strcpy(g_szTelnetPassword, argv[4]);
}
#endif

#if M_DES("cmd_local_user_username_password",1)
CMD_DEFINE(cmd_local_user_username_password, (char*)"local-user STRING<1-32> password STRING<1-32>",
		(char*)"local-user username password", local_user_username_password)
{
	char Username[32] = {0};
	char Password[32] = {0};
	extern ULONG AAA_AddUser(struct cmd_vty *vty, char *uname, char *psw);

	strcpy(Username, argv[1]);
	strcpy(Password, argv[3]);

	if (OS_OK != AAA_AddUser(vty, Username, Password))
	{
		return OS_ERR;
	}

	return OS_OK;
}
#endif

#if M_DES("cmd_undo_local_user_username",1)
CMD_DEFINE(cmd_undo_local_user_username, (char*)"undo local-user STRING<1-32>",
		(char*)"undo local-user username", undo_local_user_username)
{
	char Username[32] = {0};
	extern ULONG AAA_DelUser(struct cmd_vty *vty, char *uname);
	
	strcpy(Username, argv[2]);

	if (OS_OK != AAA_DelUser(vty, Username))
	{
		return OS_ERR;
	}

	return OS_OK;
}
#endif

#if M_DES("cmd_reboot_system_st",1)

/* BEGIN: Added by weizengke, 2014/3/3 reset */
CMD_DEFINE(cmd_reboot_system_st, (char*)"reboot system", (char*)"reboot system", reboot_system_st)
{
	extern void PDT_DestroySocket();
	PDT_DestroySocket();

	extern void PDT_InitConfigData();
	PDT_InitConfigData();
	
	extern int PDT_InitSocket();
	PDT_InitSocket();

	extern int OJ_Init();
	OJ_Init();
	
	vty_printf(vty, "Info: reboot system ok.\r\n");
}
#endif

#if M_DES("cmd_mysql_url_username_password_table_st",1)
CMD_DEFINE(cmd_mysql_url_username_password_table_st, (char*)"mysql url STRING<1-256> port INTEGER<1-65535> username STRING<1-24> password STRING<1-24> table STRING<1-24>",
		(char*)"mysql url port username password table", mysql_url_username_password_table_st)
{
	int ret = 0;
	char name[25] = {0};
	char psw[25]={0};
	char port[25]={0};
	char url[257]={0};
	char table[25] = {0};
	char port_tmp[25];
	
	itoa(Mysql_port,port_tmp,10);

	strcpy(url, argv[2]);
	strcpy(port, argv[4]);
	strcpy(name, argv[6]);
	strcpy(psw, argv[8]);
	strcpy(table, argv[10]);
	
	strcpy(Mysql_url, url);
	strcpy(Mysql_username, name);
	strcpy(Mysql_password, psw);
	strcpy(Mysql_table, table);
	Mysql_port = atoi(port);

	//printf("\r\n%s %s %s %s %s %s", url, name, psw,table, Mysql_table, argv[10]);
	
	vty_printf(vty, "Info: Please reboot to take effect.\r\n");

	return OS_OK;
}
#endif

#if M_DES("cmd_save_st",1)
CMD_DEFINE(cmd_save_st, (char*)"save",
		(char*)"save", cmd_save)
{
	char filename[257] = {0};
	ULONG ulRet = OS_OK;
	CHAR *pBuildrun = NULL;
	FILE * fp= NULL;
	extern ULONG BDN_SystemBuildRun(CHAR **ppBuildrun);
	
	sprintf(filename, "conf\\config.cfg");

	fp = fopen(filename,"w+");
	if (NULL == fp)
	{
		vty_printf(vty, "Error: Open file %s failed.\r\n", filename);
		return OS_ERR;
	}
	
	ulRet = BDN_SystemBuildRun(&pBuildrun);
	if (OS_OK != ulRet)
	{
		vty_printf(vty, "Error: Save configuration failed.\r\n");
		fclose(fp);
		return OS_ERR;
	}
	
	//judge_outstring("\r\n%s\r\n", pBuildrun);

	fputs(pBuildrun, fp);
	
	free(pBuildrun);
	fclose(fp);

	vty_printf(vty, "Info: Save configuration successfully.\r\n");
	
	return OS_OK;
}
#endif

#if M_DES("cmd_display_current_configuration_st",1)

CMD_DEFINE(cmd_display_current_configuration_st, (char*)"display current-configuration", (char*)"display current-configuration", display_current_configuration)
{
	void BDN_ShowBuildRun(CMD_VTY *vty);
	BDN_ShowBuildRun(vty);
	
	//extern void Judge_ShowCfgContent();
	//Judge_ShowCfgContent();

	return 0;
}
#endif

#if M_DES("cmd_display_save_configuration_st",1)
CMD_DEFINE(cmd_display_save_configuration_st, (char*)"display save-configuration",
		(char*)"display save-configuration", cmd_display_save_configuration)
{
	extern void PDT_ShowCfg(CMD_VTY *vty);
	PDT_ShowCfg(vty);
	
	return OS_OK;
}
#endif

#if M_DES("cmd_display_users_st",1)
CMD_DEFINE(cmd_display_users_st, (char*)"display users", 
		(char*)"display users", cmd_display_user)
{
	int ret = 0;
	time_t t = time(NULL);
	long diff = 0;
	long sec = 0;
	long min = 0;
	long hour = 0;
	char time_delay[10] = {0};
	char acbuff[65556] = {0};
	char *buff = NULL;

	buff = acbuff;
		
	buff += sprintf(buff, "   #   Type     Delay        Network Address   Socket  Username   \r\n"
						  " ---------------------------------------------------------------------\r\n");
	
	diff = getdiftime(t, g_con_vty->user.lastAccessTime);  
	sec=diff%60;
	diff=diff/60;
	min=diff%60; 
	hour=diff=diff/60;
	sprintf(time_delay, "%02d:%02d:%02d", hour, min, sec);
	
	buff += sprintf(buff, " %s %-3u %-7s  %-12s %-16s  %-7s %s\r\n",
					(vty->user.type == 0)?"+":" ", 0, "Console", time_delay, "127.0.0.1", "-","-");
	
	for (int i = 0; i < CMD_VTY_MAXUSER_NUM; i++)
	{
		if (g_vty[i].valid)
		{
			sockaddr_in addr;  
		    int addrlen = sizeof(addr);  
		    if(getpeername(g_vty[i].user.socket, (struct sockaddr*)&addr, &addrlen) == -1){          
		        continue;  
		    }  

			if (0 == g_vty[i].user.state)
			{
				sprintf(time_delay, "Unauthorized");
			}
			else
			{
				diff = getdiftime(t, g_vty[i].user.lastAccessTime);
				sec=diff%60;
				diff=diff/60;
				min=diff%60; 
				hour=diff=diff/60;
				sprintf(time_delay, "%02d:%02d:%02d", hour, min, sec);
			}

			buff += sprintf(buff, " %s %-3u %-7s  %-12s %-16s  %-7u %s\r\n",
				(vty->user.socket == g_vty[i].user.socket)?"+":" ", i + 1, "Telnet", time_delay, inet_ntoa(addr.sin_addr), g_vty[i].user.socket, g_vty[i].user.state?g_vty[i].user.user_name:"-");
		}
	}

	vty_printf(vty, acbuff);
	
}
#endif

#if M_DES("cmd_quit_st",1)
CMD_DEFINE(cmd_quit_st, (char*)"quit", 
		(char*)"Quit from the current view", func_cmd_quit)
{
	vty_view_quit(vty);
}
#endif

#if M_DES("cmd_system_view_st",1)
CMD_DEFINE(cmd_system_view_st, (char*)"system-view", 
		(char*)"Enter the system-view", func_cmd_system_view)
{
	vty_view_set(vty, VIEW_SYSTEM);
}
#endif
#if M_DES("cmd_diagnose_view_st",1)
CMD_DEFINE(cmd_diagnose_view_st, (char*)"diagnose-view", 
		(char*)"Enter the diagnose-view", func_cmd_diagnose_view)
{
	vty_view_set(vty, VIEW_DIAGNOSE);
}
#endif

#if M_DES("cmd_aaa_st",1)
CMD_DEFINE(cmd_aaa_st, (char*)"aaa", 
		(char*)"", func_cmd_aaa_st)
{
	vty_view_set(vty, VIEW_AAA);
}
#endif


#if M_DES("cmd_return_st",1)
CMD_DEFINE(cmd_return_st, (char*)"return", 
		(char*)"Return to the user viewAuthentication Authorization Accounting",func_cmd_return)
{
	vty_view_set(vty, VIEW_USER);
}
#endif

#if M_DES("cmd_display_st",1)
#include"product\thirdpart32\cjson\cJSON.h"
CMD_DEFINE(cmd_display_st, (char*)"display", (char*)"display", display)
{

#if 0

	judge_outstring("Info: display thread info.\n");

	extern int GetProcessThreadList();
	GetProcessThreadList();

	char current_path[MAX_PATH] = {0};
	GetCurrentDirectory(sizeof(current_path),current_path);
#endif

#if 0

	cJSON *json = cJSON_CreateObject();
	cJSON *array = NULL;
	cJSON *testcase = NULL;
	cJSON_AddNumberToObject(json,"solutionId", 1001);
	//向文档中添加一个键值对
	cJSON_AddNumberToObject(json,"problemId",1000);
	cJSON_AddStringToObject(json,"username","jungle");
	cJSON_AddStringToObject(json,"language","C++");
	cJSON_AddItemToObject(json,"testcase",array=cJSON_CreateArray());
	for (int i = 0; i<100; i++)
	{
		cJSON_AddItemToArray(array, testcase = cJSON_CreateObject());
		cJSON_AddNumberToObject(testcase, "case", 1);
		cJSON_AddStringToObject(testcase, "verdict", "Accepted");
		cJSON_AddNumberToObject(testcase, "timeused", 10);
		cJSON_AddNumberToObject(testcase, "memused", 1024);
	}

	char *buf = cJSON_Print(json);

	judge_outstring("\n%s\nlen=%d\n", buf,strlen(buf));
	free(buf);

	cJSON_Delete(json);	
#endif


	//strcpy(NULL,"adfadfg");
	
	return 0;
}
#endif

/* 	MID_OS = 1,
	MID_JUDGE,
	MID_SQL,
	MID_DEBUG,
	MID_CMD,
	MID_EVENT,
	MID_NDP,
	MID_TELNET,
*/

#if M_DES("debug模块注册",1)
Debug_RegModule(MID_OS, common)
Debug_RegModule(MID_JUDGE, judge)
Debug_RegModule(MID_SQL, mysql)
Debug_RegModule(MID_DEBUG, debug)
Debug_RegModule(MID_CMD, command)
Debug_RegModule(MID_TELNET, telnet)
Debug_RegModule(MID_TELNET, event)
Debug_RegModule(MID_NDP, ndp)
#endif

void cmd_install()
{
	/* reg cmd-element */
	cmd_reg_newcmdelement(CMD_ELEM_ID_CR, 			CMD_ELEM_TYPE_END,			CMD_END,			    ""               );
	cmd_reg_newcmdelement(CMD_ELEM_ID_STRING1TO24,  CMD_ELEM_TYPE_STRING,       "STRING<1-24>",     "String lenth range form 1 to 24");
	cmd_reg_newcmdelement(CMD_ELEM_ID_STRING1TO32,  CMD_ELEM_TYPE_STRING,       "STRING<1-32>",     "String lenth range form 1 to 32");
    cmd_reg_newcmdelement(CMD_ELEM_ID_STRING1TO256, CMD_ELEM_TYPE_STRING,       "STRING<1-256>",     "String lenth range form 1 to 256");
    cmd_reg_newcmdelement(CMD_ELEM_ID_STRING1TO65535,  CMD_ELEM_TYPE_STRING,    "STRING<1-65535>",  "String lenth range form 1 to 65535");
	cmd_reg_newcmdelement(CMD_ELEM_ID_INTEGER1TO24, CMD_ELEM_TYPE_INTEGER,      "INTEGER<1-100>",   "Integer range form 1 to 100");
	cmd_reg_newcmdelement(CMD_ELEM_ID_INTEGER1TO65535, CMD_ELEM_TYPE_INTEGER,   "INTEGER<1-65535>", "Integer range form 1 to 65535");

	cmd_reg_newcmdelement(CMD_ELEM_ID_RETURN,  		CMD_ELEM_TYPE_KEY, 			"return", 			"Return to the user view");
	cmd_reg_newcmdelement(CMD_ELEM_ID_QUIT,  		CMD_ELEM_TYPE_KEY,          "quit",      		"Quit from the current system view");
	cmd_reg_newcmdelement(CMD_ELEM_ID_SYSTEM_VIEW,  CMD_ELEM_TYPE_KEY,          "system-view",      "Enter the system view");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DAIGNOSE_VIEW,CMD_ELEM_TYPE_KEY,			"diagnose-view",	"Enter the daignose view");
	cmd_reg_newcmdelement(CMD_ELEM_ID_AAA,			CMD_ELEM_TYPE_KEY,			"aaa",				"Authentication Authorization Accounting");	

	cmd_reg_newcmdelement(CMD_ELEM_ID_COMMAND_TREE, CMD_ELEM_TYPE_KEY,          "command-tree",     "Command tree");
	cmd_reg_newcmdelement(CMD_ELEM_ID_CURRENT_CFG,  CMD_ELEM_TYPE_KEY,          "current-configuration",     "Current Configuration");
	cmd_reg_newcmdelement(CMD_ELEM_ID_EVENT, 		CMD_ELEM_TYPE_KEY,   		"event",            "Event");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DEBUG, 		CMD_ELEM_TYPE_KEY,   		"debug",            "Debug");
	cmd_reg_newcmdelement(CMD_ELEM_ID_COMMON, 		CMD_ELEM_TYPE_KEY,   		"common",           "Common");
	cmd_reg_newcmdelement(CMD_ELEM_ID_CMD, 			CMD_ELEM_TYPE_KEY,   		"command",          "Command");
	cmd_reg_newcmdelement(CMD_ELEM_ID_SYSNAME, 		CMD_ELEM_TYPE_KEY,   		"sysname",          "Set system name");
	cmd_reg_newcmdelement(CMD_ELEM_ID_UNDO, 		CMD_ELEM_TYPE_KEY,   		"undo",				"Undo operation");
	cmd_reg_newcmdelement(CMD_ELEM_ID_ENABLE, 		CMD_ELEM_TYPE_KEY,   		"enable",			"Enable operation");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DISABLE, 		CMD_ELEM_TYPE_KEY,   		"disable",			"Disable operation");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DISPLAY, 		CMD_ELEM_TYPE_KEY,   		"display",			"Display");
	cmd_reg_newcmdelement(CMD_ELEM_ID_TERMINAL,     CMD_ELEM_TYPE_KEY,   		"terminal",			"Terminal");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DEBUGING,     CMD_ELEM_TYPE_KEY,   		"debugging",		"Debugging switch");
	cmd_reg_newcmdelement(CMD_ELEM_ID_ON, 			CMD_ELEM_TYPE_KEY,   		"on",				"Debug switch open");
	cmd_reg_newcmdelement(CMD_ELEM_ID_OFF, 			CMD_ELEM_TYPE_KEY,   		"off",				"Debug switch close");
	cmd_reg_newcmdelement(CMD_ELEM_ID_VERSION, 		CMD_ELEM_TYPE_KEY,   		"version",			"Show version of solfware");

	cmd_reg_newcmdelement(CMD_ELEM_ID_CLOCK,        CMD_ELEM_TYPE_KEY,   		"clock",			"Show clock now");
	cmd_reg_newcmdelement(CMD_ELEM_ID_COMPUTER, 	CMD_ELEM_TYPE_KEY,   		"computer",			"Show computer information");

	cmd_reg_newcmdelement(CMD_ELEM_ID_HISTTORY, 	CMD_ELEM_TYPE_KEY,   		"history",			"Histrory command");
	cmd_reg_newcmdelement(CMD_ELEM_ID_VJUDGE,	    CMD_ELEM_TYPE_KEY,   		"virtual-judge", 	"Virtual judge");

	cmd_reg_newcmdelement(CMD_ELEM_ID_DEBUG_ERROR,  CMD_ELEM_TYPE_KEY,			"error",			"Error");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DEBUG_FUNC,   CMD_ELEM_TYPE_KEY,			"function",			"Function");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DEBUG_INFO,   CMD_ELEM_TYPE_KEY,			"info",				"Information");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DEBUG_MSG,    CMD_ELEM_TYPE_KEY,			"message",			"Message");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DEBUG_FSM,    CMD_ELEM_TYPE_KEY,			"fsm",				"Finite State Machine");

	cmd_reg_newcmdelement(CMD_ELEM_ID_DEBUG_ALL,    CMD_ELEM_TYPE_KEY,			"all",				"All");

	cmd_reg_newcmdelement(CMD_ELEM_ID_LOGIN,     	CMD_ELEM_TYPE_KEY,			"login",			"Login");

	cmd_reg_newcmdelement(CMD_ELEM_ID_HDUJUDGE,		CMD_ELEM_TYPE_KEY,			"hdu-judge",		"HDU-Judge");
	cmd_reg_newcmdelement(CMD_ELEM_ID_GUETJUDGE,	CMD_ELEM_TYPE_KEY,			"guet-judge",		"GUET-Judge");
	cmd_reg_newcmdelement(CMD_ELEM_ID_REMOTE_JUDGE,	CMD_ELEM_TYPE_KEY,			"remote-judge",		"Remote-Judge");
	cmd_reg_newcmdelement(CMD_ELEM_ID_USERNAME, 	CMD_ELEM_TYPE_KEY,			"username", 		"Username");
	cmd_reg_newcmdelement(CMD_ELEM_ID_PASSWORD,		CMD_ELEM_TYPE_KEY,			"password",			"Password");


	cmd_reg_newcmdelement(CMD_ELEM_ID_STATUS,		CMD_ELEM_TYPE_KEY,			"status",			"Status");

	cmd_reg_newcmdelement(CMD_ELEM_ID_JUDGE,		CMD_ELEM_TYPE_KEY,			"judge",			"Judge of OJ");
	cmd_reg_newcmdelement(CMD_ELEM_ID_SOLUTION,		CMD_ELEM_TYPE_KEY,			"solution",			"The Solution");

	cmd_reg_newcmdelement(CMD_ELEM_ID_PROBLEM,		CMD_ELEM_TYPE_KEY,			"problem",			"The Problem of OJ");

	cmd_reg_newcmdelement(CMD_ELEM_ID_SET,			CMD_ELEM_TYPE_KEY,			"set",				"Set value");
	cmd_reg_newcmdelement(CMD_ELEM_ID_CONFIG,		CMD_ELEM_TYPE_KEY,			"config",			"Set Config section name value");

	cmd_reg_newcmdelement(CMD_ELEM_ID_REBOOT,		CMD_ELEM_TYPE_KEY,			"reboot",			"Reboot Judge kernel");
	cmd_reg_newcmdelement(CMD_ELEM_ID_BRIEF,		CMD_ELEM_TYPE_KEY,			"brief",			"Brief Information");
	cmd_reg_newcmdelement(CMD_ELEM_ID_IP,			CMD_ELEM_TYPE_KEY,			"ip",			    "IP");
	cmd_reg_newcmdelement(CMD_ELEM_ID_PORT,			CMD_ELEM_TYPE_KEY,			"port",			    "Port");

	/* add for mode acm or oi */
	cmd_reg_newcmdelement(CMD_ELEM_ID_MODE,			CMD_ELEM_TYPE_KEY,			"mode",			    "Judge Mode");
	cmd_reg_newcmdelement(CMD_ELEM_ID_ACM,			CMD_ELEM_TYPE_KEY,			"acm",			    "ACM Mode, break while answer is not accepted");
	cmd_reg_newcmdelement(CMD_ELEM_ID_OI,			CMD_ELEM_TYPE_KEY,			"oi",			    "OI Mode, will run all testcases");

	cmd_reg_newcmdelement(CMD_ELEM_ID_ROLE,			CMD_ELEM_TYPE_KEY,			"role",		    	"Judge role");
	cmd_reg_newcmdelement(CMD_ELEM_ID_MASTER,		CMD_ELEM_TYPE_KEY,			"master",			"Judge master role, the default role");
	cmd_reg_newcmdelement(CMD_ELEM_ID_AGENT,		CMD_ELEM_TYPE_KEY,			"agent",		    "Judge agent role");

	cmd_reg_newcmdelement(CMD_ELEM_ID_NDP,			CMD_ELEM_TYPE_KEY,			"ndp",		    	"NDP");
	cmd_reg_newcmdelement(CMD_ELEM_ID_SERVER,		CMD_ELEM_TYPE_KEY,			"server",		    "Server");
	cmd_reg_newcmdelement(CMD_ELEM_ID_CLIENT,		CMD_ELEM_TYPE_KEY,			"client",		    "Client");
	cmd_reg_newcmdelement(CMD_ELEM_ID_BIND,			CMD_ELEM_TYPE_KEY,			"bind",		    	"Bind socket");
	cmd_reg_newcmdelement(CMD_ELEM_ID_NEIGHBOR,		CMD_ELEM_TYPE_KEY,			"neighbor",		    "Neighbor");
	cmd_reg_newcmdelement(CMD_ELEM_ID_AUTO_DETECT,	CMD_ELEM_TYPE_KEY,			"auto-detect",		"Auto Detection");
	cmd_reg_newcmdelement(CMD_ELEM_ID_INTERVAL,	    CMD_ELEM_TYPE_KEY,			"interval",			"Interval Time");

	cmd_reg_newcmdelement(CMD_ELEM_ID_DATAPATH,		CMD_ELEM_TYPE_KEY,			"data-path",		"Judge testcase data path, end with '\\'");

	cmd_reg_newcmdelement(CMD_ELEM_ID_MYSQL,		CMD_ELEM_TYPE_KEY,			"mysql",			"Mysql");
	cmd_reg_newcmdelement(CMD_ELEM_ID_URL,			CMD_ELEM_TYPE_KEY,			"url",				"The url of mysql");
	cmd_reg_newcmdelement(CMD_ELEM_ID_TABLE,		CMD_ELEM_TYPE_KEY,			"table",			"Table of Mysql");

	cmd_reg_newcmdelement(CMD_ELEM_ID_SAVE,			CMD_ELEM_TYPE_KEY,			"save",				"Save configuration");
	cmd_reg_newcmdelement(CMD_ELEM_ID_SAVE_CFG,		CMD_ELEM_TYPE_KEY,			"save-configuration","Save-configuration");

	cmd_reg_newcmdelement(CMD_ELEM_ID_TELNET,		CMD_ELEM_TYPE_KEY,			"telnet",			"Telnet Protocol");
	cmd_reg_newcmdelement(CMD_ELEM_ID_SYSTEM,		CMD_ELEM_TYPE_KEY,			"system",			"The judger system");
	cmd_reg_newcmdelement(CMD_ELEM_ID_USERS,		CMD_ELEM_TYPE_KEY,			"users",			"Users");

	cmd_reg_newcmdelement(CMD_ELEM_ID_AUTHENTICATION_MODE,	CMD_ELEM_TYPE_KEY,	"authentication-mode",	"Authentication-mode");
	cmd_reg_newcmdelement(CMD_ELEM_ID_MODE_NONE,	CMD_ELEM_TYPE_KEY,			"none",				"None");

	cmd_reg_newcmdelement(CMD_ELEM_ID_LOCAL_USER,	CMD_ELEM_TYPE_KEY,			"local-user",		"AAA Local User");

	// install command
	// ---------------------------------------------------

	/* 各模块debug命令注册 */
	Debug_ModuleCmdInstall(common);
	Debug_ModuleCmdInstall(judge);
	Debug_ModuleCmdInstall(mysql);
	Debug_ModuleCmdInstall(debug);
	Debug_ModuleCmdInstall(command);
	Debug_ModuleCmdInstall(telnet);
	Debug_ModuleCmdInstall(event);
	Debug_ModuleCmdInstall(ndp);

	install_command(VIEW_GLOBAL, &cmd_return_st);
	install_command(VIEW_GLOBAL, &cmd_quit_st);
	install_command(VIEW_USER, &cmd_system_view_st);
	install_command(VIEW_SYSTEM, &cmd_diagnose_view_st);
	install_command(VIEW_SYSTEM, &cmd_aaa_st);
	
	install_command(VIEW_SYSTEM, &cmd_sysname_st);
	//install_command(VIEW_DIAGNOSE, &cmd_sysname_st);
	install_command(VIEW_USER, &cmd_terminal_debugging_st);
	install_command(VIEW_USER, &cmd_undo_terminal_debugging_st);
	install_command(VIEW_USER, &cmd_debugging_enable_st);
 	install_command(VIEW_USER, &cmd_undo_debugging_enable_st);
	install_command(VIEW_USER, &cmd_debugging_all_st);
	install_command(VIEW_USER, &cmd_undo_debugging_all_st);
	install_command(VIEW_GLOBAL, &cmd_display_debugging_st);

 	//install_command(&cmd_display_clock_st);
 	//install_command(&cmd_display_computer_st);
 	install_command(VIEW_DIAGNOSE, &cmd_version_st);
 	install_command(VIEW_GLOBAL, &cmd_display_history_st);
	install_command(VIEW_GLOBAL, &cmd_display_history_n_st);
	install_command(VIEW_GLOBAL, &cmd_display_st);

#if(JUDGE_VIRTUAL == VOS_YES)
    install_command(VIEW_SYSTEM, &cmd_virtual_judge_enable_st);
	install_command(VIEW_SYSTEM, &cmd_undo_virtual_judge_enable_st);

	install_command(VIEW_SYSTEM, &cmd_hdujudge_login_st);
	install_command(VIEW_SYSTEM, &cmd_display_hdujudge_status_st);
    install_command(VIEW_SYSTEM, &cmd_display_hdu_judge_problem_by_pid_st);
	install_command(VIEW_SYSTEM, &cmd_hdu_judge_enable_st);
	install_command(VIEW_SYSTEM, &cmd_undo_hdu_judge_enable_st);
    install_command(VIEW_SYSTEM, &cmd_hdu_judge_remote_judge_enable_st);
	install_command(VIEW_SYSTEM, &cmd_undo_hdu_judge_remote_judge_enable_st);
	install_command(VIEW_SYSTEM, &cmd_hdu_judge_username_password_st);
	install_command(VIEW_SYSTEM, &cmd_hdu_judge_ip_port_st);
    #if 0
	install_command(VIEW_SYSTEM, &cmd_guetjudge_login_st);	
    install_command(VIEW_SYSTEM, &cmd_display_guetjudge_status_st);   
    install_command(VIEW_SYSTEM, &cmd_guet_judge_enable_st);	
    install_command(VIEW_SYSTEM, &cmd_undo_guet_judge_enable_st);
	install_command(VIEW_SYSTEM, &cmd_guet_judge_remote_judge_enable_st);
	install_command(VIEW_SYSTEM, &cmd_undo_guet_judge_remote_judge_enable_st);
    install_command(VIEW_SYSTEM, &cmd_guet_judge_username_password_st);
	install_command(VIEW_SYSTEM, &cmd_guet_judge_ip_port_st);
    #endif
#endif
    install_command(VIEW_GLOBAL, &cmd_display_judge_brief_st);
	install_command(VIEW_SYSTEM, &cmd_judge_solution_st);
    
	install_command(VIEW_GLOBAL, &cmd_display_command_tree_st);
	install_command(VIEW_GLOBAL, &cmd_display_current_configuration_st);
	install_command(VIEW_DIAGNOSE, &cmd_set_config_section_name_value_st);
	install_command(VIEW_SYSTEM, &cmd_judge_datapath_st);

	/* add for mode acm or oi */
	install_command(VIEW_SYSTEM, &cmd_judge_mode_acm);
	install_command(VIEW_SYSTEM, &cmd_judge_mode_oi);
    
	install_command(VIEW_SYSTEM, &cmd_judge_auto_detect_enable);
	install_command(VIEW_SYSTEM, &cmd_undo_judge_auto_detect_enable);
	install_command(VIEW_SYSTEM, &cmd_judge_auto_detect_interval);
	
	install_command(VIEW_SYSTEM, &cmd_judge_enable_st);
	install_command(VIEW_SYSTEM, &cmd_undo_judge_enable_st);

	install_command(VIEW_SYSTEM, &cmd_ndp_server_enable);
	install_command(VIEW_SYSTEM, &cmd_undo_ndp_server_enable);

	install_command(VIEW_SYSTEM, &cmd_ndp_client_enable);
	install_command(VIEW_SYSTEM, &cmd_undo_ndp_client_enable);

	install_command(VIEW_SYSTEM, &cmd_ndp_server_ip_port);
	install_command(VIEW_SYSTEM, &cmd_ndp_server_bind_port);

	install_command(VIEW_GLOBAL, &cmd_display_ndp_neighbor);

	install_command(VIEW_SYSTEM, &cmd_telnet_server_enable);
	install_command(VIEW_SYSTEM, &cmd_undo_telnet_server_enable);	

	install_command(VIEW_SYSTEM, &cmd_telnet_authentication_mode_none);
	install_command(VIEW_SYSTEM, &cmd_telnet_authentication_mode_password);
	install_command(VIEW_SYSTEM, &cmd_telnet_authentication_mode_aaa);
	install_command(VIEW_SYSTEM, &cmd_telnet_username_password);

	install_command(VIEW_AAA, &cmd_local_user_username_password);
	install_command(VIEW_AAA, &cmd_undo_local_user_username);
		
	install_command(VIEW_SYSTEM, &cmd_reboot_system_st);
	install_command(VIEW_SYSTEM, &cmd_mysql_url_username_password_table_st);
	install_command(VIEW_SYSTEM, &cmd_save_st);
	install_command(VIEW_GLOBAL, &cmd_display_save_configuration_st);

	install_command(VIEW_GLOBAL, &cmd_display_users_st);

	// ---------------------------------------------------

}
