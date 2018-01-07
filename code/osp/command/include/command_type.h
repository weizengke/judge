#ifndef _COMMAND_TYPE_H_
#define _COMMAND_TYPE_H_

/* BEGIN: Added by weizengke, 2013/10/27  for debug switch*/


enum CMD_ELEM_TYPE_EM
{
	CMD_ELEM_TYPE_VALID,
	CMD_ELEM_TYPE_KEY,
	CMD_ELEM_TYPE_INTEGER,
	CMD_ELEM_TYPE_STRING,

	CMD_ELEM_TYPE_END,  /* <CR> */
	CMD_ELEM_TYPE_MAX,

};

// command match type
enum CMD_MATCH_STATUS {
	CMD_NO_MATCH,
	CMD_FULL_MATCH,
	CMD_PART_MATCH,
	CMD_LIST_MATCH,
	CMD_ERR_AMBIGOUS,
	CMD_ERR_ARGU,
};



enum CMD_ELEM_ID_EM {
	CMD_ELEM_ID_NONE = -1,
	CMD_ELEM_ID_CR,
	CMD_ELEM_ID_UNDO,
	CMD_ELEM_ID_ENABLE,
	CMD_ELEM_ID_DISABLE,
	CMD_ELEM_ID_DISPLAY,
	CMD_ELEM_ID_EVENT,
	CMD_ELEM_ID_DEBUG,
	CMD_ELEM_ID_TERMINAL,
	CMD_ELEM_ID_DEBUGING,
	CMD_ELEM_ID_ON,
	CMD_ELEM_ID_OFF,
	CMD_ELEM_ID_VERSION,

	CMD_ELEM_ID_COMMON,
	CMD_ELEM_ID_CMD,

	CMD_ELEM_ID_SYSNAME,
	CMD_ELEM_ID_CLOCK,
	CMD_ELEM_ID_COMPUTER,

	CMD_ELEM_ID_STRING1TO24,
	CMD_ELEM_ID_STRING1TO32,
	CMD_ELEM_ID_STRING1TO256,
	CMD_ELEM_ID_STRING1TO65535,
	CMD_ELEM_ID_INTEGER1TO24,
	CMD_ELEM_ID_INTEGER1TO65535,

	CMD_ELEM_ID_RETURN,
	CMD_ELEM_ID_QUIT,
	CMD_ELEM_ID_SYSTEM_VIEW,
	CMD_ELEM_ID_DAIGNOSE_VIEW,
	CMD_ELEM_ID_AAA,
	
    CMD_ELEM_ID_COMMAND_TREE,
    CMD_ELEM_ID_CURRENT_CFG,

	CMD_ELEM_ID_HISTTORY,

	CMD_ELEM_ID_VJUDGE,

	CMD_ELEM_ID_DEBUG_ERROR,
	CMD_ELEM_ID_DEBUG_FUNC,
	CMD_ELEM_ID_DEBUG_INFO,
	CMD_ELEM_ID_DEBUG_MSG,
	CMD_ELEM_ID_DEBUG_FSM,
	CMD_ELEM_ID_DEBUG_ALL,

    CMD_ELEM_ID_LOGIN,
    CMD_ELEM_ID_HDUJUDGE,
	CMD_ELEM_ID_GUETJUDGE,
	CMD_ELEM_ID_REMOTE_JUDGE,
    CMD_ELEM_ID_USERNAME,
    CMD_ELEM_ID_PASSWORD,

	CMD_ELEM_ID_STATUS,
    CMD_ELEM_ID_JUDGE,
    CMD_ELEM_ID_SOLUTION,
    CMD_ELEM_ID_PROBLEM,
    CMD_ELEM_ID_SET,
    CMD_ELEM_ID_CONFIG,
    CMD_ELEM_ID_REBOOT,
    CMD_ELEM_ID_BRIEF,
    CMD_ELEM_ID_IP,
    CMD_ELEM_ID_PORT,

	CMD_ELEM_ID_MODE,
	CMD_ELEM_ID_ACM,
	CMD_ELEM_ID_OI,

	CMD_ELEM_ID_ROLE,
	CMD_ELEM_ID_MASTER,
	CMD_ELEM_ID_AGENT,

	CMD_ELEM_ID_NDP,
	CMD_ELEM_ID_SERVER,
	CMD_ELEM_ID_CLIENT,
	CMD_ELEM_ID_BIND,
	CMD_ELEM_ID_NEIGHBOR,
	CMD_ELEM_ID_AUTO_DETECT,
	CMD_ELEM_ID_INTERVAL,
	CMD_ELEM_ID_DATAPATH,
	CMD_ELEM_ID_MYSQL,
	CMD_ELEM_ID_URL,
	CMD_ELEM_ID_TABLE,
	CMD_ELEM_ID_SAVE,
	CMD_ELEM_ID_SAVE_CFG,
	CMD_ELEM_ID_TELNET,
	CMD_ELEM_ID_SYSTEM,
	CMD_ELEM_ID_USERS,
	CMD_ELEM_ID_AUTHENTICATION_MODE,
	CMD_ELEM_ID_MODE_NONE,

	CMD_ELEM_ID_LOCAL_USER,
	
	CMD_ELEM_ID_MAX,
	
};


enum CMD_KEY_CODE_EM {
	CMD_KEY_CODE_NONE = -1,
		CMD_KEY_CODE_FILTER = 0,
		CMD_KEY_CODE_TAB,  // CMD_KEY_CODE_TAB
		CMD_KEY_CODE_ENTER,
		CMD_KEY_CODE_QUEST,
		CMD_KEY_CODE_UP,
		CMD_KEY_CODE_DOWN,
		CMD_KEY_CODE_LEFT,
		CMD_KEY_CODE_RIGHT,
		CMD_KEY_CODE_DELETE,	 /* delete键 */
		CMD_KEY_CODE_BACKSPACE,  /* 退格键 */
	    CMD_KEY_CODE_DEL_LASTWORD,    /* 10 */

		CMD_KEY_CODE_NOTCARE,   /* 11 */

		CMD_KEY_CODE_MAX
};


enum VIEW_ID_EN {
	VIEW_NULL = 0,
	VIEW_GLOBAL = 1,
	VIEW_USER,
	VIEW_SYSTEM,
	VIEW_AAA,
	VIEW_USER_VTY,
	VIEW_DIAGNOSE,
};

struct vty_user_s {
	int level;
	int type;  /* 0:com, 1:telnet */
	int state;  /* 0:idle, 1:access */
	int terminal_debugging;
	SOCKET socket;
	char user_name[32];
	char user_psw[32];
	time_t lastAccessTime;
	
};

/**
 * A virtual tty used by CMD
 *
 * @param prompt prompt string, such as cmd, then you see 'cmd>'
 * @param buffer buffer to store user input
 * @param buf_len buffer max_length
 * @param used_len buffer actually used length
 * @param cur_pos current cursor point in which position in buffer
 * @param c latest input key word
 * @param history to record user input command
 * @param hpos history current position
 * @param hindex history end index
 * */
typedef struct cmd_vty {
	int valid;
	VIEW_ID_EN view_id; /* 当前所在视图 */
	vty_user_s user;
	int buf_len;
	int used_len;
	int cur_pos;
	char c;
	char res[3];
	char prompt[CMD_MAX_PROMPT_SIZE];
	char buffer[CMD_BUFFER_SIZE];

	/* BEGIN: Added by weizengke, for support TAB agian and agian */
	int inputMachine_prev;
	int inputMachine_now;
	char tabbingString[CMD_MAX_CMD_ELEM_SIZE];	/* 最初始用来补全查找的字串*/
	char tabString[CMD_MAX_CMD_ELEM_SIZE];		/* 最后一次补全的命令 */
	int tabStringLenth;
	/* END: Added by weizengke, for support TAB agian and agian */
	
	int hpos;
	int hindex;
	char *history[HISTORY_MAX_SIZE];
}CMD_VTY;

 

/**
 * struct to store command string
 *
 * @param size total size of vector
 * @param used_size already allocated size
 * @param data point to data
 */
typedef struct cmd_vector {
	int size;
	int used_size;
	void **data;
} cmd_vector_t;

/*
 * A struct cmd_node relative to some command of special prompt
*/
typedef struct cmd_node {
	char prompt[CMD_MAX_PROMPT_SIZE];
    cmd_vector_t cmd_vector;

}cmd_node_st;


/**
 * A struct cmd_elem_st relative to One command
 *
 * @param string command string, such as 'show vlan'
 * @param doc command refenrence, each command word has one
 * @param func excute when command called
 * @param para_vec command string in para_vec form
 * @param para_size command parameter number
 */
struct cmd_elem_st {
	VIEW_ID_EN view_id;
	char *string;
	char *doc;
	int (*func)(struct cmd_elem_st *, struct cmd_vty *, int , char **);
	cmd_vector_t *para_vec;
	int para_num;

	//char prompt[CMD_MAX_PROMPT_SIZE];
	
	int cmd_id; /* for comand function callback */
};

/**
 * A struct para_desc relative to One command parameter
 *
 * @param para command parameter, such as 'show' of 'show vlan'
 * @param desc command parameter reference
 */
struct para_desc {
	CMD_ELEM_TYPE_EM  elem_tpye;
	int  elem_id;
	char *para;
	char *desc;
};

#define CMD_VIEW_SONS_NUM 100

typedef struct cmd_view_node {
	VIEW_ID_EN  view_id;
	char view_name[CMD_MAX_VIEW_SIZE];
    char view_ais_name[CMD_MAX_VIEW_SIZE];

	struct cmd_view_node *pParent;
	
	struct cmd_view_node **ppSons;
	int view_son_num;
	
}view_node_st;


typedef struct key_handler {
	int key_value;
	void (*key_func)(struct cmd_vty *);
} key_handler_t;


/* CMD_DEFINE for vty command interafce. */
#define CMD_DEFINE(cmdname, cmdstr, helpstr, funcname) \
	int funcname (struct cmd_elem_st *, struct cmd_vty *, int, char **); \
	struct cmd_elem_st cmdname = \
	{ \
		VIEW_GLOBAL, \
		cmdstr, \
		helpstr, \
		funcname \
	}; \
	int funcname \
	(struct cmd_elem_st *self, struct cmd_vty *vty, int argc, char **argv)

#endif


