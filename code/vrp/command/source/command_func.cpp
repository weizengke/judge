
#include "..\include\command_inc.h"


DEFUN(cmd_debugging_enable_st, (char*)"debugging enable", (char*)"Debugging switch on", Debugging_enable)
{
	if (g_debug_switch == DEBUG_ENABLE)
	{
		printf("Info: debugging switch is already enable.\n");
		return 0;
	}

	g_debug_switch = DEBUG_ENABLE;
	printf("Info: debugging switch is enable.\n");

	return 0;
}


DEFUN(cmd_undo_debugging_enable_st, (char*)"undo debugging enable", (char*)"Debugging switch off", undo_debugging_enable)
{

	if (g_debug_switch == DEBUG_DISABLE)
	{
		printf("Info: debugging switch is already disable.\n");
		return 0;
	}

	g_debug_switch = DEBUG_DISABLE;
	printf("Info: debugging switch is disable.\n");

	return 0;
}

DEFUN(date_elem_st, (char*)"date", (char*)"Display date-time now", date)
{
	if(argc == 0) {
		time_t	timep = time(NULL);
		struct tm *p;

		p = localtime(&timep);
		p->tm_year = p->tm_year + 1900;
		p->tm_mon = p->tm_mon + 1;

		printf(" Date of device:\r\n %04d-%02d-%02d %02d:%02d:%02d UTC(+8) DTS\n",p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);

		return 0;
	}

	printf("Syntax Error\n");
	return -1;
}


DEFUN(cmd_dhcp_enable_st, (char*)"dhcp enable", (char*)"Dynamic Host Configuration Protocol", dhcp_enable)
{
	printf("Info: dhcp enable successful.\n");
	return 0;
}

DEFUN(cmd_dhcp_disable_st, (char*)"dhcp enable", (char*)"Dynamic Host Configuration Protocol", dhcp_disable)
{
	printf("Info: dhcp disable successful.\n");
	return 0;
}


DEFUN(cmd_stp_enable_st, (char*)"stp enable", (char*)"Spanning tree protocol", stp_enable)
{
	//printf("argc=%d, %s %s", argc, argv[0], argv[1]);

	printf("Info: stp enable successful.\n");

	return 0;
}

DEFUN(cmd_stp_disable_st, (char*)"stp disable", (char*)"Spanning tree protocol", stp_disable)
{
	//printf("argc=%d, %s %s", argc, argv[0], argv[1]);

	printf("Info: stp disable successful.\n");

	return 0;
}

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

DEFUN(cmd_display_computer_st, (char*)"display computer", (char*)"Display computer information", display_computer)
{
	printf("This is Jungle Wei's computer.\n");
	return 0;
}

DEFUN(cmd_display_version_st, (char*)"display version", (char*)"Display device version", display_version)
{
	printf("Common command-line system Version 0.0.1 Beta.\n");
	return 0;
}

DEFUN(cmd_display_stp_st, (char*)"display stp", (char*)"Display stp information", display_stp)
{
	printf("Test command 'display stp'.\n");
	return 0;
}


DEFUN(cmd_display_stp_brief_st, (char*)"display stp brief", (char*)"Display stp brief information", display_stp_brief)
{
	printf("Test command 'display stp brief'.\n");
	return 0;
}

DEFUN(cmd_display_stp_verbose_st, (char*)"display stp verbose", (char*)"Display device version", display_stp_verbose)
{
	printf("Test command 'display stp verbose'.\n");

	return 0;
}

DEFUN(cmd_virtual_judge_enable_st, (char*)"virtual-judge enable", (char*)"Enable virtual judge", virtual_judge_enable)
{
	printf("Info: virtual judge enable successful, support hdoj virtual-judge only.\n");

	return 0;
}

DEFUN(cmd_undo_virtual_judge_enable_st, (char*)"undo virtual-judge enable", (char*)"Undo enable virtual judge", undo_virtual_judge_enable)
{
	printf("Info: virtual judge is disable successful.\n");

	return 0;
}


DEFUN(cmd_disable_st, (char*)"disable", (char*)"disable", disable)
{
	printf("Info: disable.\n");

	return 0;
}

DEFUN(cmd_display_st, (char*)"display", (char*)"display", display)
{
	printf("Info: display.\n");
	return 0;
}



DEFUN(cmd_sysname_st, (char*)"sysname STRING<1-24>", (char*)"set system name", sysname)
{
	CMD_DBGASSERT(argv[1] != 0);

	strcpy(g_sysname, argv[1]);

	printf("Info: system name change to %s successful.\r\n", argv[1]);

	return 0;
}

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

DEFUN(cmd_loopback_internal_st, (char*)"loopback internal", (char*)"loopback internal", loopback_internal)
{
	printf("Info: loopback internal.\n");
	return 0;
}

DEFUN(cmd_loopback_detect_enable_st, (char*)"loopback-detect enable", (char*)"loopback-detect enable", loopback_detect_enable)
{
	printf("Info: loopback-detect enable.\n");
	return 0;
}

DEFUN(cmd_display_loopback_st, (char*)"display loopback", (char*)"display loopback info", display_loopback)
{
	printf("Info: display loopback.\n");
	return 0;
}
DEFUN(cmd_disable_loopback_detect_st, (char*)"disable loopback-detect", (char*)"disable loopback-detect protocol", disable_loopback_detect)
{
	printf("Info: disable loopback-detect.\n");
	return 0;
}

DEFUN(cmd_display_loopback_brief_st, (char*)"display loopback brief", (char*)"display loopback brief info", display_loopback_brief)
{
	printf("Info: display loopback.\n");
	return 0;
}

DEFUN(cmd_display_loopback_detect_brief_st, (char*)"display loopback-detect brief", (char*)"display loopback-detect protocol brief Info", display_loopback_detect_brief)
{
	printf("Info: display loopback-detect brief.\n");
	return 0;
}


DEFUN(cmd_debugging_error_st, (char*)"debugging error", (char*)"open debugging error switch", debugging_error)
{
	CMD_DEBUGMASK_SET(CMD_DEBUG_TYPE_ERROR);
	printf("Info: debugging error switch is on.\r\n");
	return 0;
}

DEFUN(cmd_undo_debugging_error_st, (char*)"undo debugging error", (char*)"close debugging error switch", undo_debugging_error)
{
	CMD_DEBUGMASK_CLEAR(CMD_DEBUG_TYPE_ERROR);
	printf("Info: debugging error switch is off.\r\n");
	return 0;
}

DEFUN(cmd_debugging_function_st, (char*)"debugging function", (char*)"open debugging function switch", debugging_function)
{
	CMD_DEBUGMASK_SET(CMD_DEBUG_TYPE_FUNC);
	printf("Info: debugging function switch is on.\r\n");
	return 0;
}

DEFUN(cmd_undo_debugging_function_st, (char*)"undo debugging function", (char*)"Close debugging function switch", undo_debugging_function)
{
	CMD_DEBUGMASK_CLEAR(CMD_DEBUG_TYPE_FUNC);
	printf("Info: debugging function switch is off.\r\n");
	return 0;
}

DEFUN(cmd_debugging_info_st, (char*)"debugging info", (char*)"Open debugging info switch", debugging_info)
{
	CMD_DEBUGMASK_SET(CMD_DEBUG_TYPE_INFO);
	printf("Info: debugging info switch is on.\r\n");
	return 0;
}

DEFUN(cmd_undo_debugging_info_st, (char*)"undo debugging info", (char*)"close debugging info switch", undo_debugging_info)
{
	CMD_DEBUGMASK_CLEAR(CMD_DEBUG_TYPE_INFO);
	printf("Info: debug info switch is off.\r\n");
	return 0;
}

DEFUN(cmd_debugging_message_st, (char*)"debugging message", (char*)"Open debugging message switch", debugging_message)
{
	CMD_DEBUGMASK_SET(CMD_DEBUG_TYPE_MSG);
	printf("Info: debug message switch is on.\r\n");
	return 0;
}

DEFUN(cmd_undo_debugging_message_st, (char*)"undo debugging message", (char*)"close debugging message switch", undo_debugging_message)
{
	CMD_DEBUGMASK_CLEAR(CMD_DEBUG_TYPE_MSG);
	printf("Info: debugging message switch is off.\r\n");
	return 0;
}

DEFUN(cmd_debugging_fsm_st, (char*)"debugging fsm", (char*)"open debugging fsm switch", debugging_fsm)
{
	CMD_DEBUGMASK_SET(CMD_DEBUG_TYPE_FSM);
	printf("Info: debugging fsm switch is on.\r\n");
	return 0;
}

DEFUN(cmd_undo_debugging_fsm_st, (char*)"undo debugging fsm", (char*)"close debugging fsm switch", undo_debugging_fsm)
{
	CMD_DEBUGMASK_CLEAR(CMD_DEBUG_TYPE_FSM);
	printf("Info: debugging message switch is off.\r\n");
	return 0;
}

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

DEFUN(cmd_hdujudge_login_st, (char*)"hdu-judge login STRING<1-24> STRING<1-24>", (char*)"hdujudge login", hdujudge_login)
{
	char username_[25] = {0};
	char pwd[25] = {0};

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "%d %s %s %s\n", argc, argv[0], argv[1], argv[2], argv[3]);

	strcpy(username_, argv[2]);
	strcpy(pwd, argv[3]);

	extern int loginEx(char *uname, char *pdw);
	loginEx(username_, pwd);

	return 0;
}

DEFUN(cmd_display_hdujudge_status_st, (char*)"display hdu-judge status STRING<1-24>", (char*)"display hdujudge status <username>", display_hdujudge_status)
{
	char username_[25] = {0};

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "%d %s %s %s\n", argc, argv[0], argv[1], argv[2], argv[3]);

	strcpy(username_, argv[3]);

	extern int getStatusEx(char *hdu_username);
	getStatusEx(username_);

	return 0;
}


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

void cmd_init()
{
	// initial cmd vector
	cmd_vec = cmd_vector_init(1);

	/* reg cmd-element */
	cmd_reg_newcmdelement(CMD_ELEM_ID_CR, 			CMD_ELEM_TYPE_END,			CMD_END,			    ""               );
	cmd_reg_newcmdelement(CMD_ELEM_ID_STRING1TO24,  CMD_ELEM_TYPE_STRING,       "STRING<1-24>",     "String lenth range form 1 to 24");
	cmd_reg_newcmdelement(CMD_ELEM_ID_INTEGER1TO24, CMD_ELEM_TYPE_INTEGER,      "INTEGER<1-100>",   "Integer range form 1 to 100");
	cmd_reg_newcmdelement(CMD_ELEM_ID_INTEGER1TO65535, CMD_ELEM_TYPE_INTEGER,   "INTEGER<1-65535>",   "Integer range form 1 to 100");

	cmd_reg_newcmdelement(CMD_ELEM_ID_SYSNAME, 		CMD_ELEM_TYPE_KEY,   		"sysname",          "Set system name");
	cmd_reg_newcmdelement(CMD_ELEM_ID_UNDO, 			CMD_ELEM_TYPE_KEY,   		"undo",				"Undo operation");
	cmd_reg_newcmdelement(CMD_ELEM_ID_ENABLE, 		CMD_ELEM_TYPE_KEY,   		"enable",			"Enable operation");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DISABLE, 		CMD_ELEM_TYPE_KEY,   		"disable",			"Disable operation");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DISPLAY, 		CMD_ELEM_TYPE_KEY,   		"display",			"Display");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DEBUG,        CMD_ELEM_TYPE_KEY,   		"debugging",		"Debugging switch");
	cmd_reg_newcmdelement(CMD_ELEM_ID_ON, 			CMD_ELEM_TYPE_KEY,   		"on",				"Debug switch open");
	cmd_reg_newcmdelement(CMD_ELEM_ID_OFF, 			CMD_ELEM_TYPE_KEY,   		"off",				"Debug switch close");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DATE, 			CMD_ELEM_TYPE_KEY,   		"date",				"Show date-time now");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DHCP, 			CMD_ELEM_TYPE_KEY,   		"dhcp",				"Dynamic Host Configuration Protocol");
	cmd_reg_newcmdelement(CMD_ELEM_ID_VERSION, 		CMD_ELEM_TYPE_KEY,   		"version",			"Show version of solfware");
	cmd_reg_newcmdelement(CMD_ELEM_ID_STP, 			CMD_ELEM_TYPE_KEY,   		"stp",				"Spanning tree protocol");

	cmd_reg_newcmdelement(CMD_ELEM_ID_CLOCK,        CMD_ELEM_TYPE_KEY,   		"clock",			"Show clock now");
	cmd_reg_newcmdelement(CMD_ELEM_ID_COMPUTER, 		CMD_ELEM_TYPE_KEY,   		"computer",			"Show computer information");

	cmd_reg_newcmdelement(CMD_ELEM_ID_HISTTORY, 		CMD_ELEM_TYPE_KEY,   		"history",			"Histrory command");
	cmd_reg_newcmdelement(CMD_ELEM_ID_BRIEF,        CMD_ELEM_TYPE_KEY,   		"brief",			"Brief information");
	cmd_reg_newcmdelement(CMD_ELEM_ID_VERBOSE, 		CMD_ELEM_TYPE_KEY,   		"verbose",			"Verbose information");
	cmd_reg_newcmdelement(CMD_ELEM_ID_VJUDGE,	    CMD_ELEM_TYPE_KEY,   		"virtual-judge", 	"Virtual judge");

	cmd_reg_newcmdelement(CMD_ELEM_ID_LOOPBACK,		CMD_ELEM_TYPE_KEY,   		"loopback", 		"Loopback");
	cmd_reg_newcmdelement(CMD_ELEM_ID_LOOPBACK_DETECT,CMD_ELEM_TYPE_KEY,   		"loopback-detect", 	"loopback-detect protocol");
	cmd_reg_newcmdelement(CMD_ELEM_ID_INTERNAL,		CMD_ELEM_TYPE_KEY,			"internal", 		"Internal");

	cmd_reg_newcmdelement(CMD_ELEM_ID_DEBUG_ERROR,  CMD_ELEM_TYPE_KEY,			"error",			"Error");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DEBUG_FUNC,   CMD_ELEM_TYPE_KEY,			"function",			"Function");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DEBUG_INFO,   CMD_ELEM_TYPE_KEY,			"info",				"Information");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DEBUG_MSG,    CMD_ELEM_TYPE_KEY,			"message",			"Message");
	cmd_reg_newcmdelement(CMD_ELEM_ID_DEBUG_FSM,    CMD_ELEM_TYPE_KEY,			"fsm",				"Finite State Machine");

	cmd_reg_newcmdelement(CMD_ELEM_ID_DEBUG_ALL,    CMD_ELEM_TYPE_KEY,			"all",				"All");

	cmd_reg_newcmdelement(CMD_ELEM_ID_LOGIN,     	CMD_ELEM_TYPE_KEY,			"login",			"Login");

	cmd_reg_newcmdelement(CMD_ELEM_ID_HDUJUDGE,		CMD_ELEM_TYPE_KEY,			"hdu-judge",		"HDU-Judge");
	cmd_reg_newcmdelement(CMD_ELEM_ID_STATUS,		CMD_ELEM_TYPE_KEY,			"status",			"Status");

	cmd_reg_newcmdelement(CMD_ELEM_ID_JUDGE,			CMD_ELEM_TYPE_KEY,			"judge",			"Judge of OJ");
	cmd_reg_newcmdelement(CMD_ELEM_ID_SOLUTION,		CMD_ELEM_TYPE_KEY,			"solution",			"The Solution");
	// install command
	// ---------------------------------------------------

	install_element(&cmd_sysname_st);

	install_element(&cmd_debugging_enable_st);
 	install_element(&cmd_undo_debugging_enable_st);

	install_element(&cmd_stp_enable_st);
	install_element(&cmd_stp_disable_st);

 	install_element(&cmd_display_clock_st);
 	install_element(&cmd_display_computer_st);
 	install_element(&cmd_display_version_st);
 	install_element(&cmd_display_history_st);
	install_element(&cmd_display_history_n_st);

	install_element(&cmd_display_stp_st);
	install_element(&cmd_display_stp_brief_st);


	install_element(&cmd_display_stp_verbose_st);

	install_element(&cmd_virtual_judge_enable_st);
	install_element(&cmd_undo_virtual_judge_enable_st);


	install_element(&cmd_disable_st);

	install_element(&cmd_display_st);

	install_element(&cmd_loopback_internal_st);
	install_element(&cmd_loopback_detect_enable_st);

	install_element(&cmd_display_loopback_st);
	install_element(&cmd_disable_loopback_detect_st);


	install_element(&cmd_display_loopback_brief_st);
	install_element(&cmd_display_loopback_detect_brief_st);

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


	install_element(&cmd_judge_solution_st);
	// ---------------------------------------------------

}
