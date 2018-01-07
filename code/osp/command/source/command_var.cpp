
#include "osp\command\include\command_inc.h"


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

struct cmd_vty g_vty[CMD_VTY_MAXUSER_NUM];
struct cmd_vty *g_con_vty; /* ´®¿ÚÓÃ»§ */

// Global command vector, to store user installed commands
cmd_vector_t *cmd_vec;

struct para_desc g_cmd_elem[CMD_ELEM_ID_MAX];


view_node_st *view_list;




