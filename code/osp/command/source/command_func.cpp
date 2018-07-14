
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
	extern void cmd_show_command_tree(struct cmd_vty_st *vty);
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
	extern void NDP_ShowAllNeighbors(struct cmd_vty_st *vty);
	NDP_ShowAllNeighbors(vty);
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
	
	vty_printf(vty, "Info: Please reboot system to take effect.\r\n");

	return OS_OK;
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
	cmd_reg_newcmdelement(CMD_ELEM_ID_JUDGE_MGR,	CMD_ELEM_TYPE_KEY,			"judge-mgr",		"Judge MGR");
	cmd_reg_newcmdelement(CMD_ELEM_ID_VJUDGE_MGR,	CMD_ELEM_TYPE_KEY,			"virtual-judge-mgr","Virtual Judge MGR");
	
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
	cmd_reg_newcmdelement(CMD_ELEM_ID_DATAPATH,		CMD_ELEM_TYPE_KEY,			"testcase-path",	"Judge testcase data path, end with '\\'");
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
	cmd_reg_newcmdelement(CMD_ELEM_ID_SECURITY,	    CMD_ELEM_TYPE_KEY,			"security",			"Judger Security Function");
	cmd_reg_newcmdelement(CMD_ELEM_ID_THIS,	   		CMD_ELEM_TYPE_KEY,			"this",			    "This");
	cmd_reg_newcmdelement(CMD_ELEM_ID_INC_DEFAULT,	CMD_ELEM_TYPE_KEY,			"include-default",	"Include default configuration");
	
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

    install_command(VIEW_GLOBAL, &cmd_display_judge_brief_st);
	install_command(VIEW_SYSTEM, &cmd_judge_solution_st);
    
	install_command(VIEW_GLOBAL, &cmd_display_command_tree_st);
	install_command(VIEW_GLOBAL, &cmd_display_current_configuration_st);
	install_command(VIEW_DIAGNOSE, &cmd_set_config_section_name_value_st);
	

	install_command(VIEW_SYSTEM, &cmd_judge_enable_st);
	install_command(VIEW_SYSTEM, &cmd_undo_judge_enable_st);
	install_command(VIEW_SYSTEM, &cmd_judge_mgr_view_st);	
	install_command(VIEW_JUDGE_MGR, &cmd_judge_mode_acm);
	install_command(VIEW_JUDGE_MGR, &cmd_judge_mode_oi);
    install_command(VIEW_JUDGE_MGR, &cmd_judge_datapath_st);
	install_command(VIEW_JUDGE_MGR, &cmd_judge_auto_detect_enable);
	install_command(VIEW_JUDGE_MGR, &cmd_undo_judge_auto_detect_enable);
	install_command(VIEW_JUDGE_MGR, &cmd_judge_auto_detect_interval);
	install_command(VIEW_JUDGE_MGR, &cmd_judge_security_enable);
	install_command(VIEW_JUDGE_MGR, &cmd_undo_judge_security_enable);

#if(JUDGE_VIRTUAL == VOS_YES)

	install_command(VIEW_SYSTEM, &cmd_virtual_judge_mgr_view_st);
	install_command(VIEW_SYSTEM, &cmd_virtual_judge_enable_st);
	install_command(VIEW_SYSTEM, &cmd_undo_virtual_judge_enable_st);

	install_command(VIEW_VJUDGE_MGR, &cmd_hdujudge_login_st);
	install_command(VIEW_DIAGNOSE, &cmd_display_hdujudge_status_st);
	install_command(VIEW_DIAGNOSE, &cmd_display_hdu_judge_problem_by_pid_st);
	install_command(VIEW_VJUDGE_MGR, &cmd_hdu_judge_enable_st);
	install_command(VIEW_VJUDGE_MGR, &cmd_undo_hdu_judge_enable_st);
	install_command(VIEW_VJUDGE_MGR, &cmd_hdu_judge_remote_judge_enable_st);
	install_command(VIEW_VJUDGE_MGR, &cmd_undo_hdu_judge_remote_judge_enable_st);
	install_command(VIEW_VJUDGE_MGR, &cmd_hdu_judge_username_password_st);
	install_command(VIEW_VJUDGE_MGR, &cmd_hdu_judge_ip_port_st);
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
	install_command(VIEW_GLOBAL, &cmd_display_this_st);
	install_command(VIEW_GLOBAL, &cmd_display_this_inc_default_st);
	
	install_command(VIEW_GLOBAL, &cmd_display_users_st);

	// ---------------------------------------------------

}
