#ifndef _COMMAND_TYPE_H_
#define _COMMAND_TYPE_H_

/* BEGIN: Added by weizengke, 2013/10/27  for debug switch*/
enum CMD_DEBUG_TYPE_EM
{
	CMD_DEBUG_TYPE_NONE,

	CMD_DEBUG_TYPE_ERROR,
	CMD_DEBUG_TYPE_FUNC,
	CMD_DEBUG_TYPE_INFO,
	CMD_DEBUG_TYPE_MSG,
	CMD_DEBUG_TYPE_FSM,

	CMD_DEBUG_TYPE_MAX,
};


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
	CMD_ELEM_ID_DEBUG,
	CMD_ELEM_ID_ON,
	CMD_ELEM_ID_OFF,
	CMD_ELEM_ID_VERSION,

	CMD_ELEM_ID_SYSNAME,
	CMD_ELEM_ID_CLOCK,
	CMD_ELEM_ID_COMPUTER,

	CMD_ELEM_ID_STRING1TO24,
	CMD_ELEM_ID_STRING1TO65535,
	CMD_ELEM_ID_INTEGER1TO24,
	CMD_ELEM_ID_INTEGER1TO65535,
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
		CMD_KEY_CODE_DELETE,	 /* delete¼ü */
		CMD_KEY_CODE_BACKSPACE,  /* ÍË¸ñ¼ü */
	    CMD_KEY_CODE_DEL_LASTWORD,

		CMD_KEY_CODE_NOTCARE,

		CMD_KEY_CODE_MAX
};


/* BEGIN:   Added by weizengke, 2013/10/27 */
#define CMD_DEBUG_TYPE_ISVALID(x) (x>CMD_DEBUG_TYPE_NONE && x<CMD_DEBUG_TYPE_MAX)
#define CMD_MASKLENTG 32
#define CMD_DEBUGMASK_GET(x) (( g_aulDebugMask[(x)/CMD_MASKLENTG] >> ((x)%CMD_MASKLENTG) ) & 1)
#define CMD_DEBUGMASK_SET(x) ( g_aulDebugMask[(x)/CMD_MASKLENTG] |= ( 1 << (x)%CMD_MASKLENTG ) )
#define CMD_DEBUGMASK_CLEAR(x) ( g_aulDebugMask[(x)/CMD_MASKLENTG] ^= ( 1 << (x)%CMD_MASKLENTG ) )
/* END:   Added by weizengke, 2013/10/27 */


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
struct cmd_vty {
	int buf_len;
	int used_len;
	int cur_pos;
	char c;
	char res[3];
	char prompt[CMD_MAX_PROMPT_SIZE];
	char buffer[CMD_BUFFER_SIZE];

	int hpos;
	int hindex;
	char *history[HISTORY_MAX_SIZE];


};

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
	char *string;
	char *doc;
	int (*func)(struct cmd_elem_st *, struct cmd_vty *, int , char **);
	cmd_vector_t *para_vec;
	int para_num;

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


typedef struct key_handler {
	int key_value;
	void (*key_func)(struct cmd_vty *);
} key_handler_t;


/* DEFUN for vty command interafce. */
#define DEFUN(cmdname, cmdstr, helpstr, funcname) \
	int funcname (struct cmd_elem_st *, struct cmd_vty *, int, char **); \
	struct cmd_elem_st cmdname = \
	{ \
		cmdstr, \
		helpstr, \
		funcname \
	}; \
	int funcname \
	(struct cmd_elem_st *self, struct cmd_vty *vty, int argc, char **argv)

#endif

