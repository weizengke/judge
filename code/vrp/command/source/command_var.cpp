
#include "vrp\command\include\command_inc.h"


char *szDebugName[CMD_DEBUG_TYPE_MAX] = {
	"none",
	"error",
	"function",
	"info",
	"message",
	"fsm",
};



const char *key_name[CMD_KEY_CODE_MAX] = {
	"EM_KEY_TAB",
		"CMD_KEY_CODE_ENTER",
		"CMD_KEY_CODE_QUEST",
		"CMD_KEY_CODE_UP",
		"CMD_KEY_CODE_DOWN",
		"CMD_KEY_CODE_LEFT",
		"CMD_KEY_CODE_RIGHT",
		"CMD_KEY_CODE_BACKSPACE",
		"CMD_KEY_CODE_NOTCARE"
};


unsigned long g_aulDebugMask[CMD_DEBUG_TYPE_MAX/32 + 1] = {0};

int g_debug_switch = DEBUG_DISABLE;


char g_sysname[CMD_MAX_SYSNAME_SIZE] = "Jungle";


struct cmd_vty *vty;
// Global command vector, to store user installed commands
cmd_vector_t *cmd_vec;

struct para_desc g_cmd_elem[CMD_ELEM_ID_MAX];


int g_InputMachine_prev = CMD_KEY_CODE_NOTCARE;
int g_InputMachine_now = CMD_KEY_CODE_NOTCARE;
char g_tabbingString[CMD_MAX_CMD_ELEM_SIZE] = {0};  /* 最初始用来补全查找的字串*/
char g_tabString[CMD_MAX_CMD_ELEM_SIZE] = {0};      /* 最后一次补全的命令 */
int g_tabStringLenth = 0;





