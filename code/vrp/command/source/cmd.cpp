/*
Author: Jungle Wei
Date  : 2013-10
Description: A mini common command line system

Note:
	_LINUX_ for compile on Linux , #define _LINUX_  or g++ -D_LINUX_ cmd.cpp

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <stdarg.h>
#include <assert.h>

#ifdef _LINUX_
#include <curses.h>
#include <termios.h>
#include <unistd.h>
#else
#include <conio.h>
#include <io.h>
#include <windows.h>

#endif

#include "..\..\debug\include\debug_center.h"


/* config */
// log info level

#define LOG_DEBUG    3
#define LOG_WARNING  4
#define LOG_ERR      5

#define CMD_LOG_LEVEL	LOG_DEBUG

// maximum number of command to remember
#define HISTORY_MAX_SIZE	20

// maximum number of commands that can matched
#define CMD_MAX_MATCH_SIZE	100

// maximum number of command arguments
#define CMD_MAX_CMD_NUM		10

#define CMD_MAX_CMD_ELEM_SIZE 24

#define CMD_MAX_SYSNAME_SIZE 24

#define CMD_ELEM_SPACE_SIZE  CMD_MAX_CMD_ELEM_SIZE + 1

#define CMD_MAX_PROMPT_SIZE 24

// when interacting, the prompt like "jungle>"
#define CMD_PROMPT_DEFAULT	"config"

// size of input buffer size
#define CMD_BUFFER_SIZE		1024

// enter may be '\n' or '\r\n'
#define CMD_ENTER			"\r\n"

// how much number of command ouput in one line
#define CMD_LINE_NUM		2

#define FNAME "cmd-sys.c"

#define CMD_ERR 1
#define CMD_OK  0

#define CMD_YES 1
#define CMD_NO  0

#define DEBUG_DISABLE 0
#define DEBUG_ENABLE  1

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

char *szDebugName[CMD_DEBUG_TYPE_MAX] = {
	"none",
	"error",
	"function",
	"info",
	"message",
	"fsm",
};

#define CMD_DEBUG_TYPE_ISVALID(x) (x>CMD_DEBUG_TYPE_NONE && x<CMD_DEBUG_TYPE_MAX)

#define CMD_MASKLENTG 32
unsigned long g_aulDebugMask[CMD_DEBUG_TYPE_MAX/CMD_MASKLENTG + 1] = {0};
int g_debug_switch = DEBUG_DISABLE;

#define CMD_DEBUGMASK_GET(x) (( g_aulDebugMask[(x)/CMD_MASKLENTG] >> ((x)%CMD_MASKLENTG) ) & 1)
#define CMD_DEBUGMASK_SET(x) ( g_aulDebugMask[(x)/CMD_MASKLENTG] |= ( 1 << (x)%CMD_MASKLENTG ) )
#define CMD_DEBUGMASK_CLEAR(x) ( g_aulDebugMask[(x)/CMD_MASKLENTG] ^= ( 1 << (x)%CMD_MASKLENTG ) )

/* END:   Added by weizengke, 2013/10/27 */


char g_sysname[CMD_MAX_SYSNAME_SIZE] = "Jungle";

#define _CMDDEF_
#ifndef  _CMDDEF_
typedef unsigned int UINT;
#endif
#ifndef  _CMDDEF_
typedef unsigned short USHORT;
#endif
#ifndef  _CMDDEF_
typedef unsigned long ULONG;
#endif
#ifndef  _CMDDEF_
typedef unsigned char UCHAR;
#endif

/* assert(0) */
#define CMD_DBGASSERT(x) if (0 == x) printf("Assert!!!!!!!!!!!!!! Is that a bug?");

#define CMD_NOUSED(x) ((x) = (x))

void debug_print_ex(CMD_DEBUG_TYPE_EM type, const char *format, ...)
{

	if (g_debug_switch == DEBUG_DISABLE)
	{
		return;
	}


	if (!CMD_DEBUG_TYPE_ISVALID(type))
	{
		return;
	}

	if (!CMD_DEBUGMASK_GET(type))
	{
		return;
	}


	time_t  timep = time(NULL);
	struct tm *p;

    p = localtime(&timep);
    p->tm_year = p->tm_year + 1900;
    p->tm_mon = p->tm_mon + 1;

	printf("<%04d-%02d-%02d %02d:%02d:%02d>",p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\r\n");

}


void debug_print(const char *format, ...)
{
	if (g_debug_switch == DEBUG_DISABLE)
	{
		return;
	}

	time_t  timep = time(NULL);
	struct tm *p;

    p = localtime(&timep);
    p->tm_year = p->tm_year + 1900;
    p->tm_mon = p->tm_mon + 1;

	printf("<DEBUG - %04d-%02d-%02d %02d:%02d:%02d>",p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\r\n");
}


int cmd_getch()
{
	int c = 0;
	#ifdef _LINUX_
	struct termios org_opts, new_opts;
	int res = 0;
	//-----  store old settings -----------
	res = tcgetattr(STDIN_FILENO, &org_opts);
	assert(res == 0);
	//---- set new terminal parms --------
	memcpy(&new_opts, &org_opts, sizeof(new_opts));
	new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
	tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
	c = getchar();
	//------  restore old settings ---------
	res = tcsetattr(STDIN_FILENO, TCSANOW, &org_opts);
	assert(res == 0);
	#else
	c = getch();
	#endif

	return c;
}


/**
 * Cursor back one grid
 *
 * @return nonoe
 */
void cmd_back_one();

/**
 * Put a char into terminal
 *
 * @return none
 */
void cmd_put_one(char c);

/**
 * Read input
 *
 * @param vty cmd_vty
 * @return none
 */
void cmd_read(struct cmd_vty *vty);

/**
 * Write to terminal
 *
 * @return none
 */
void cmd_outstring(const char *format, ...);

/**
 * Clear one line in terminal
 *
 * @param vty cmd_vty
 * @return none
 */
void cmd_clear_line(struct cmd_vty *vty);




/**
 * A virtual tty used by CMD
 *
 * @param prompt prompt string, such as cmd, then you see 'cmd>'
 * @param buffer buffer to store user input
 * @param buf_len buffer length
 * @param used_len buffer actually used length
 * @param cur_pos current cursor point in which position in buffer
 * @param c latest input key word
 * @param history to record user input command
 * @param hpos history current position
 * @param hindex history end index
 * */
struct cmd_vty {
	char prompt[CMD_MAX_PROMPT_SIZE];
	char buffer[CMD_BUFFER_SIZE];
	int buf_len;
	int used_len;
	int cur_pos;
	char c;

	char *history[HISTORY_MAX_SIZE];
	int hpos;
	int hindex;
};

/**
 * Initial cmd_vty
 *
 * @return initialized cmd_vty
 */
struct cmd_vty *cmd_vty_init();

/**
 * Destory cmd_vty, including allocating history
 *
 * @param vty cmd_vty to destory
 * @return none
 */
void cmd_vty_deinit(struct cmd_vty *vty);

/**
 * Add one command line into cmd_vty history
 *
 * @param vty cmd_vty to add history, command is in vty->buffer
 * @return none
 */
void cmd_vty_add_history(struct cmd_vty *vty);


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

#define cmd_vector_slot(v, i)	    ((v)->data[(i)])
#define cmd_vector_max(v)		    ((v)->used_size)


#define CMD_INTEGER 	"INTEGER"
#define CMD_STRING  	"STRING"

enum CMD_ELEM_TYPE_EM
{
	CMD_ELEM_TYPE_VALID,
	CMD_ELEM_TYPE_KEY,
	CMD_ELEM_TYPE_INTEGER,
	CMD_ELEM_TYPE_STRING,

	CMD_ELEM_TYPE_END,  /* <CR> */
	CMD_ELEM_TYPE_MAX,

};

/*****************************************************************************
 Prototype    : cmd_elem_is_para_type
 Description  : 检查命令元素是否是一个参数
 Input        : CMD_ELEM_TYPE_EM type
 Output       : None
 Return Value :
 Calls        :
 Called By    :

  History        :
  1.Date         : 2013/10/5
    Author       : weizengke
    Modification : Created function

*****************************************************************************/
int cmd_elem_is_para_type(CMD_ELEM_TYPE_EM type)
{
	switch(type)
	{
		case CMD_ELEM_TYPE_INTEGER:
		case CMD_ELEM_TYPE_STRING:
			return CMD_YES;
		default:
			break;
	}

	return CMD_NO;
}

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

// command match type
enum CMD_MATCH_STATUS {
	CMD_NO_MATCH,
	CMD_FULL_MATCH,
	CMD_PART_MATCH,
	CMD_LIST_MATCH,
	CMD_ERR_AMBIGOUS,
	CMD_ERR_ARGU,
};


// Global command vector, to store user installed commands
cmd_vector_t *cmd_vec;


/* DEFUN for vty command interafce. Little bit hacky ;-). */
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
	CMD_ELEM_ID_DATE,
	CMD_ELEM_ID_DHCP,
	CMD_ELEM_ID_VERSION,
	CMD_ELEM_ID_STP,	 	/* 10 */
	CMD_ELEM_ID_SYSNAME,
	CMD_ELEM_ID_CLOCK,
	CMD_ELEM_ID_COMPUTER,

	CMD_ELEM_ID_STRING1TO24,
	CMD_ELEM_ID_INTEGER1TO24,

	CMD_ELEM_ID_HISTTORY,

	CMD_ELEM_ID_BRIEF,
	CMD_ELEM_ID_VERBOSE,

	CMD_ELEM_ID_VJUDGE,

	CMD_ELEM_ID_LOOPBACK,
	CMD_ELEM_ID_LOOPBACK_DETECT,

	CMD_ELEM_ID_INTERNAL,


	CMD_ELEM_ID_DEBUG_ERROR,
	CMD_ELEM_ID_DEBUG_FUNC,
	CMD_ELEM_ID_DEBUG_INFO,
	CMD_ELEM_ID_DEBUG_MSG,
	CMD_ELEM_ID_DEBUG_FSM,
	CMD_ELEM_ID_DEBUG_ALL,

	CMD_ELEM_ID_MAX,
};

struct para_desc g_cmd_elem[CMD_ELEM_ID_MAX];

int cmd_get_elemid_by_name(int *cmd_elem_id, char *cmd_name)
{
	int i = 0;

	if (cmd_elem_id == NULL || cmd_name == NULL)
	{
		debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd_get_elemid_by_name, param is null");
		return 1;
	}

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "In cmd_get_elemid_by_name.(cmd_elem_id=%d, cmd_name=%s)", cmd_elem_id, cmd_name);

	for (i = 0; i < CMD_ELEM_ID_MAX; i++)
	{
		debug_print_ex(CMD_DEBUG_TYPE_INFO, "In cmd_get_elemid_by_name, loop -> %d. (para=%s)", i, g_cmd_elem[i].para);
		if (0 == strcmp(cmd_name, g_cmd_elem[i].para))
		{
			*cmd_elem_id = i;
			return 0;
		}
	}

	return 1;
}


int cmd_get_elem_by_id(int cmd_elem_id, struct para_desc *cmd_elem)
{
	if (cmd_elem == NULL)
	{
		debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd_get_elem_by_id, param is null");
		return 1;
	}

	if (cmd_elem_id <= CMD_ELEM_ID_NONE || cmd_elem_id >=  CMD_ELEM_ID_MAX)
	{
		debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd_get_elem_by_id, cmd_elem_id is invalid");
		return 1;
	}

	*cmd_elem = g_cmd_elem[cmd_elem_id];

	return 0;
}

int cmd_get_elem_by_name(char *cmd_name, struct para_desc *cmd_elem)
{
	int i = 0;

	if (cmd_name == NULL || cmd_elem == NULL)
	{
		debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd_get_elem_by_name, param is null");
		return 1;
	}

	for (i = 0; i < CMD_ELEM_ID_MAX; i++)
	{
		if ( g_cmd_elem[i].para == NULL)
		{
			continue;
		}

		debug_print_ex(CMD_DEBUG_TYPE_FUNC, "In cmd_get_elem_by_name, loop -> %d. (para=%s)", i, g_cmd_elem[i].para);

		if (0 == strcmp(cmd_name, g_cmd_elem[i].para))
		{
			*cmd_elem = g_cmd_elem[i];
			return 0;
		}
	}

	return 1;
}


/*****************************************************************************
 Prototype    : cmd_reg_newcmdelement
 Description  : 注册命令元素
 Input        : int cmd_elem_id
                char *cmd_name
                char *cmd_help
 Output       : None
 Return Value :
 Calls        :
 Called By    :

  History        :
  1.Date         : 2013/10/4
    Author       : weizengke
    Modification : Created function

*****************************************************************************/
int cmd_reg_newcmdelement(int cmd_elem_id, CMD_ELEM_TYPE_EM cmd_elem_type, const char *cmd_name, const char *cmd_help)
{

	/* BEGIN: Added by weizengke, 2013/10/4   PN:后续需要校验合法性 , 命名规范与变量名相似 */

	g_cmd_elem[cmd_elem_id].para = (char*)malloc(strlen(cmd_name) + 1);
	if (g_cmd_elem[cmd_elem_id].para == NULL)
	{
		debug_print_ex(CMD_DEBUG_TYPE_ERROR, "malloc memory for para fail in regNewCmdElement.");
		return 1;
	}

	g_cmd_elem[cmd_elem_id].desc = (char*)malloc(strlen(cmd_help) + 1);
	if (g_cmd_elem[cmd_elem_id].desc == NULL)
	{
		debug_print_ex(CMD_DEBUG_TYPE_ERROR, "malloc memory for desc fail in regNewCmdElement.");
		free(g_cmd_elem[cmd_elem_id].para);
		return 1;
	}

	g_cmd_elem[cmd_elem_id].elem_id = cmd_elem_id;
	g_cmd_elem[cmd_elem_id].elem_tpye = cmd_elem_type;
	strcpy(g_cmd_elem[cmd_elem_id].para, cmd_name);
	strcpy(g_cmd_elem[cmd_elem_id].desc, cmd_help);

	debug_print_ex(CMD_DEBUG_TYPE_MSG, "cmd_reg_newcmdelement(%d %d %s %s) ok.",
				cmd_elem_id,
				cmd_elem_type,
				g_cmd_elem[cmd_elem_id].para,
				g_cmd_elem[cmd_elem_id].desc);

	return 0;

}


/* key */
// definition of ASCII value of keys
// Arrow key is a sequenece started by 27, 91, XX
// Backspace value is 8, Ctrl+H equals backspace, so another value is 127
#ifdef _LINUX_
#define CMD_KEY_ARROW1	0x1b  		 //0xffffffe0
#define CMD_KEY_ARROW2	0x5b		 //0x0
#define CMD_KEY_UP		0x41         //0x48
#define CMD_KEY_DOWN 	0x42		 //0x50
#define CMD_KEY_RIGHT 	0x43		 //0x4d
#define CMD_KEY_LEFT 	0x44		 //0x4b
#define CMD_KEY_DELETE  0x7e		 //0x08
#define CMD_KEY_BACKSPACE 0x20		 //0x8
#define CMD_KEY_CTRL_H	(0x1f | 0x7f)
#else
#define CMD_KEY_ARROW1	0xffffffe0   //0x1b
#define CMD_KEY_ARROW2	0x0			 //0x5b
#define CMD_KEY_UP		0x48
#define CMD_KEY_DOWN 	0x50
#define CMD_KEY_RIGHT 	0x4d
#define CMD_KEY_LEFT 	0x4b
#define CMD_KEY_DELETE  0x08
#define CMD_KEY_BACKSPACE 0x20
#define CMD_KEY_CTRL_H	(0x1f | 0x7f)
#endif

enum CMD_KEY_CODE_EM {
	CMD_KEY_CODE_NONE = -1,
		CMD_KEY_CODE_TAB = 0,  // CMD_KEY_CODE_TAB
		CMD_KEY_CODE_ENTER,
		CMD_KEY_CODE_QUEST,
		CMD_KEY_CODE_UP,
		CMD_KEY_CODE_DOWN,
		CMD_KEY_CODE_LEFT,
		CMD_KEY_CODE_RIGHT,
		CMD_KEY_CODE_DELETE,
		CMD_KEY_CODE_NOTCARE,

		CMD_KEY_CODE_MAX
};

typedef struct key_handler {
	int key_value;
	void (*key_func)(struct cmd_vty *);
} key_handler_t;


const char *key_name[CMD_KEY_CODE_MAX] = {
	"EM_KEY_TAB",
		"CMD_KEY_CODE_ENTER",
		"CMD_KEY_CODE_QUEST",
		"CMD_KEY_CODE_UP",
		"CMD_KEY_CODE_DOWN",
		"CMD_KEY_CODE_LEFT",
		"CMD_KEY_CODE_RIGHT",
		"CMD_KEY_CODE_DELETE",
		"CMD_KEY_CODE_NOTCARE"
};

int cmd_resolve(char c);

void cmd_resolve_tab(struct cmd_vty *vty);
void cmd_resolve_enter(struct cmd_vty *vty);
void cmd_resolve_quest(struct cmd_vty *vty);
void cmd_resolve_up(struct cmd_vty *vty);
void cmd_resolve_down(struct cmd_vty *vty);
void cmd_resolve_left(struct cmd_vty *vty);
void cmd_resolve_right(struct cmd_vty *vty);
void cmd_resolve_delete(struct cmd_vty *vty);
void cmd_resolve_insert(struct cmd_vty *vty);



void cmd_outprompt(char *prompt)
{
	cmd_outstring("<%s-%s>", g_sysname, prompt);
}


void cmd_debug(int level, const char *fname, const char *fmt, ...)
{
	va_list ap;
	char logbuf[1024];

	va_start(ap, fmt);
	vsprintf(logbuf, fmt, ap);
	va_end(ap);

	if (level <= CMD_LOG_LEVEL) {
		time_t now;
		struct tm *tm_now;

		if ((now = time(NULL)) < 0)
			exit(1);
		tm_now = localtime(&now);

		fprintf(stderr, "%02d:%02d:%02d: %s:%s",
			tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec,
			fname, logbuf);
	}
}


/* vty */


struct cmd_vty *cmd_vty_init()
{
	struct cmd_vty *vty;

	vty = (struct cmd_vty *)calloc(1, sizeof(struct cmd_vty));
	if(vty == NULL) {
		debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd_vty_init, Not Enough Memory For vty%s", CMD_ENTER);
		return NULL;
	}

	memcpy(vty->prompt, CMD_PROMPT_DEFAULT, sizeof(CMD_PROMPT_DEFAULT));
	vty->prompt[strlen(CMD_PROMPT_DEFAULT)] = '\0';
	vty->buf_len = CMD_BUFFER_SIZE;
	vty->used_len = vty->cur_pos = 0;
	vty->hpos = vty->hindex = 0;

	return vty;
}

void cmd_vty_deinit(struct cmd_vty *vty)
{
	int i;

	if (!vty)
		return;

	// free history
	for (i = 0; i < HISTORY_MAX_SIZE; i++) {
		if (vty->history[i] != NULL)
			free(vty->history[i]);
	}

	// free vty
	free(vty);
}

void cmd_vty_add_history(struct cmd_vty *vty)
{
	int idx =  vty->hindex ? vty->hindex - 1 : HISTORY_MAX_SIZE - 1;

	// if same as previous command, then ignore
	if (vty->history[idx] &&
		!strcmp(vty->buffer, vty->history[idx])) {
		vty->hpos = vty->hindex;
		return;
	}

	// insert into history tail
	if (vty->history[vty->hindex])
		free(vty->history[vty->hindex]);
	vty->history[vty->hindex] = strdup(vty->buffer);

	// History index rotation
	vty->hindex = (vty->hindex + 1) == HISTORY_MAX_SIZE ? 0 : vty->hindex + 1;
	vty->hpos = vty->hindex;
}

/* vector */
// get an usable vector slot, if there is not, then allocate
// a new slot
static int cmd_vector_fetch(cmd_vector_t *v)
{
	int fetch_idx;

	// find next empty slot
	for (fetch_idx = 0; fetch_idx < v->used_size; fetch_idx++)
		if (v->data[fetch_idx] == NULL)
			break;

	// allocate new memory if not enough slot
	while (v->size < fetch_idx + 1) {
		debug_print_ex(CMD_DEBUG_TYPE_FUNC, "In cmd_vector_fetch, realloc memory for data.");
		v->data = (void**)realloc(v->data, sizeof(void *) * v->size * 2);
		if (!v->data) {
			debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd_vector_fetch, Not Enough Memory For data");
			return -1;
		}
		memset(&v->data[v->size], 0, sizeof(void *) * v->size);
		v->size *= 2;
	}

	return fetch_idx;
}

cmd_vector_t *cmd_vector_init(int size)
{
	cmd_vector_t *v = (cmd_vector_t *)calloc(1, sizeof(struct cmd_vector));
	if (v == NULL) {
		debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd_vector_init, Not Enough Memory For cmd_vector");
		return NULL;
	}

	/// allocate at least one slot
	if (size == 0)
		size = 1;
	v->data = (void**)calloc(1, sizeof(void *) * size);
	if (v->data == NULL) {
		debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd_vector_init, Not Enough Memory For data");
		free(v);
		return NULL;
	}

	v->used_size = 0;
	v->size = size;
	return v;
}

void cmd_vector_deinit(cmd_vector_t *v, int freeall)
{
	if (v == NULL)
		return;

	if (v->data) {
		if (freeall) {
			int i;
			for (i = 0; i < cmd_vector_max(v); i++) {
				if (cmd_vector_slot(v, i))
					free(cmd_vector_slot(v, i));
			}
		}
		free(v->data);
	}
	free(v);
}

cmd_vector_t *cmd_vector_copy(cmd_vector_t *v)
{
	int size;
	cmd_vector_t *new_v = (cmd_vector_t *)calloc(1, sizeof(cmd_vector_t));
	if (NULL == new_v)
	{
		CMD_DBGASSERT(0);
		return NULL;
	}

	new_v->used_size = v->used_size;
	new_v->size = v->size;

	size = sizeof(void *) * (v->size);
	new_v->data = (void**)calloc(1, sizeof(void *) * size);
	memcpy(new_v->data, v->data, size);

	return new_v;
}

void cmd_vector_insert(cmd_vector_t *v, void *val)
{
	int idx;

	idx = cmd_vector_fetch(v);
	v->data[idx] = val;
	if (v->used_size <= idx)
		v->used_size = idx + 1;
}

/*
	仅用于用户输入末尾补<CR>，保存在cmd_vector_t->data
*/
void cmd_vector_insert_cr(cmd_vector_t *v)
{
	char *string_cr = NULL;
	string_cr = (char*)malloc(sizeof("<CR>"));
	if (NULL == string_cr)
	{
		CMD_DBGASSERT(0);
		return;
	}

	memcpy(string_cr, "<CR>", sizeof("<CR>"));
	cmd_vector_insert(v, string_cr); /* cmd_vector_insert(v, "<CR>"); // bug of memory free("<CR>"), it's static memory*/
}

cmd_vector_t *str2vec(char *string)
{
	int str_len;
	char *cur, *start, *token;
	cmd_vector_t *vec;

	// empty string
	if (string == NULL)
		return NULL;

	cur = string;
	// skip white spaces
	while (isspace((int) *cur) && *cur != '\0')
		cur++;
	// only white spaces
	if (*cur == '\0')
		return NULL;
	// not care ! and #
	if (*cur == '!' || *cur == '#')
		return NULL;

	// copy each command pieces into vector
	vec = cmd_vector_init(1);
	while (1)
	{
		start = cur;
		while (!(isspace((int) *cur) || *cur == '\r' || *cur == '\n') &&
			*cur != '\0')
			cur++;
		str_len = cur - start;
		token = (char *)malloc(sizeof(char) * (str_len + 1));
		if (NULL == token)
		{
			debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In str2vec, There is no memory for param token.");
			return NULL;
		}

		memcpy(token, start, str_len);
		*(token + str_len) = '\0';
		cmd_vector_insert(vec, (void *)token);

		while((isspace ((int) *cur) || *cur == '\n' || *cur == '\r') &&
			*cur != '\0')
			cur++;
		if (*cur == '\0')
			return vec;
	}
}

/* end vector */


/* cmd */

/*** ---------------- Auxiliary Function ------------------ ***/
// check if 'str' is unique in matchvec, matchvec->data point to
// a double array char array like char **
static int match_unique_string(struct para_desc **match, char *str, int size)
{
	int i;

	for (i = 0; i < size; i++) {
		if (match[i]->para != NULL && strcmp(match[i]->para, str) == 0)
			return 0;
	}
	return 1;
}

int comp(const  void* a, const  void *b)
{
	return strcmp(((struct para_desc *)a)->para, ((struct para_desc *)b)->para);
}

// turn a command into vector
static cmd_vector_t *cmd2vec(char *string, char *doc)
{
	char *sp=NULL, *d_sp=NULL;	// parameter start point
	char *cp = string;	// parameter current point
	char *d_cp = doc;	// doc point
	char *token=NULL, *d_token=NULL;	// paramter token
	int len, d_len;
	cmd_vector_t *allvec=NULL;
	struct para_desc *desc = NULL;
	struct para_desc *desc_cr = NULL;

	if(cp == NULL)
		return NULL;

	// split command string into paramters, and turn these paramters
	// into vector form
	allvec = cmd_vector_init(1);
	while (1)
	{
		// get next parameter from 'string'
		while(isspace((int) *cp) && *cp != '\0')
			cp++;
		if(*cp == '\0')
			break;

		sp = cp;
		while(!(isspace ((int) *cp) || *cp == '\r' || *cp == '\n')
				&& *cp != '\0')
			cp++;
		len = cp - sp;
		token = (char*)malloc(len + 1);
		if (NULL == token)
		{
			debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd2vec, There is no memory for param token.");
			return NULL;
		}

		memcpy(token, sp, len);
		*(token + len) = '\0';

		// get next paramter info from 'doc'
		while(isspace((int)*d_cp) && *d_cp != '\0')
			d_cp++;
		if (*d_cp == '\0')
			d_token = NULL;
		else {
				d_sp = d_cp;
			while(!(*d_cp == '\r' || *d_cp == '\n') && *d_cp != '\0')
				d_cp++;
			d_len = d_cp - d_sp;
			d_token = (char*)malloc(d_len + 1);
			if (NULL == d_token)
			{
				debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd2vec, There is no memory for param d_token.");
				free(token);
				return NULL;
			}

			memcpy(d_token, d_sp, d_len);
			*(d_token + d_len) = '\0';
		}

		// add new para_vector into command vector
		desc = (struct para_desc *)calloc(1, sizeof(struct para_desc));
		if (desc == NULL)
		{
			debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd2Vec, calloc for desc fail. (token=%s)", token);
			free(token);
			free(d_token);
			return NULL;
		}

		/* BEGIN: Added by weizengke, 2013/10/4   PN:for regCmdElem  */
		if (0 != cmd_get_elem_by_name(token, desc))
		{
			debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd2Vec, cmd_get_elem_by_name fail. (token=%s)", token);
			free(token);
			free(d_token);
			free(desc);
			return NULL;
		}
		/* END:   Added by weizengke, 2013/10/4   PN:None */

		debug_print_ex(CMD_DEBUG_TYPE_FUNC, "In cmd2Vec, desc. (elem_id=%d, elem_tpye=%d, para=%s, desc=%s)", desc->elem_id, desc->elem_tpye, desc->para, desc->desc);

		cmd_vector_insert(allvec, (void *)desc);
	}

	// add <CR> into command vector
	desc_cr = (struct para_desc *)calloc(1, sizeof(struct para_desc));
	if (desc_cr == NULL)
	{
		debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd2Vec, calloc for desc_cr fail. (token=%s)", token);
		cmd_vector_deinit(allvec, 1);
		return NULL;
	}

	/* BEGIN: Added by weizengke, 2013/10/4   PN:for regCmdElem  */
	if (0 != cmd_get_elem_by_name((char*)"<CR>", desc_cr))
	{
		debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd2Vec, cmd_get_elem_by_name fail. (token=%s)", token);
		free(desc_cr);
		cmd_vector_deinit(allvec, 1);
		return NULL;
	}
	/* END:   Added by weizengke, 2013/10/4   PN:None */

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "In cmd2Vec, desc_cr. (elem_id=%d, elem_tpye=%d, para=%s, desc=%s)", desc_cr->elem_id, desc_cr->elem_tpye, desc_cr->para, desc_cr->desc);

	cmd_vector_insert(allvec, (void *)desc_cr);

/*
	{
		struct para_desc *desc_ = NULL;
		for (int i = 0; i < cmd_vector_max(allvec); i++)
		{
			desc_ = (struct para_desc *)cmd_vector_slot(allvec, i);
			printf("cmd2vec + %s\n",(char*)desc_->para);
		}
	}
*/
	return allvec;
}


/*  get range form INTEGER<a-b> ISTRING<a-b>

	type for INTEGER or STRING
*/
int cmd_get_range_symbol(char *string, int *type, int *a, int *b)
{
	char *pleft = NULL;
	char *pright = NULL;
	char *pline = NULL;
	char type_string[256] = {0};
	/*
		请确保string有效性
	*/

	if (string == NULL || a == NULL || b == NULL)
	{
		return CMD_ERR;
	}

	pleft  = strchr(string, '<');
	pline  = strchr(string, '-');
	pright = strchr(string, '>');

	if (pleft == NULL || pline == NULL || pright == NULL)
	{
		return CMD_ERR;
	}

	*a = 0;
	*b = 0;
	*type = CMD_ELEM_TYPE_VALID;

	sscanf(string,"%[A-Z]<%d-%d>", type_string, a, b);

	if (strcmp(type_string, CMD_STRING) == 0)
	{
		*type = CMD_ELEM_TYPE_STRING;
	}

	if (strcmp(type_string, CMD_INTEGER) == 0)
	{
		*type = CMD_ELEM_TYPE_INTEGER;
	}

	debug_print_ex(CMD_DEBUG_TYPE_MSG, "%s<%d-%d>",type_string, *a, *b);

	return CMD_OK;

}

/* 检查字符串是不是只包含数字 */
int cmd_string_isdigit(char *string)
{
	int i = 0;
	if (string == NULL)
	{
		return CMD_ERR;
	}

	for (i = 0; i < (int)strlen(string); i++)
	{
		if (!isdigit(*(string + i)))
		{
			debug_print_ex(CMD_DEBUG_TYPE_ERROR, "cmd_string_isdigit return error.");
			return CMD_ERR;
		}
	}

	debug_print_ex(CMD_DEBUG_TYPE_INFO, "cmd_string_isdigit return ok.");
	return CMD_OK;
}

/* match INTEGER or STRING */
int cmd_match_special_string(char *icmd, char *dest)
{
	/*
	num

	string

	ohter

	*/
	int dest_type = CMD_ELEM_TYPE_VALID;
	int a = 0;
	int b = 0;

	if (icmd == NULL || dest == NULL)
	{
		debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd_match_special_string, param is null.");
		return CMD_ERR;
	}

	if (cmd_get_range_symbol(dest, &dest_type, &a, &b))
	{
		//debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd_match_special_string, cmd_get_range_symbol fail, (dest_type=%d,a=%d,b=%d)", dest_type, a, b);
		return CMD_ERR;
	}

	switch (dest_type)
	{
		case CMD_ELEM_TYPE_INTEGER:
			if (CMD_OK == cmd_string_isdigit(icmd))
			{
				int icmd_i = atoi(icmd);
				if (icmd_i >= a && icmd_i <=b)
				{
					debug_print_ex(CMD_DEBUG_TYPE_INFO, "In cmd_match_special_string, match INTEGER<%d-%d>.", a, b);
					return CMD_OK;
				}
			}
			break;
		case CMD_ELEM_TYPE_STRING:
			{
				int icmd_len = (int)strlen(icmd);
				if (icmd_len >= a && icmd_len <= b)
				{
					debug_print_ex(CMD_DEBUG_TYPE_INFO, "In cmd_match_special_string, match STRING<%d-%d>.", a, b);
					return CMD_OK;
				}
			}
			break;
		default:
			break;
	}


	return CMD_ERR;
}

// get LCD of matched command
int match_lcd(struct para_desc **match, int size)
{
	int i, j;
	int lcd = -1;
	char *s1, *s2;
	char c1, c2;

	if (size < 2)
		return 0;

	for (i = 1; i < size; i++) {
		s1 = match[i - 1]->para;
		s2 = match[i]->para;

		for (j = 0; (c1 = s1[j]) && (c2 = s2[j]); j++)
			if (c1 != c2)
				break;

		if (lcd < 0)
			lcd = j;
		else {
			if (lcd > j)
				lcd = j;
		}
	}

	return lcd;
}

static int cmd_filter_command(char *cmd, cmd_vector_t *v, int index)
{
	int i;
	struct cmd_elem_st *elem;
	struct para_desc *desc;

	// For each command, check the 'index'th parameter if it matches the
	// 'index' paramter in command, set command vector which does not
	// match cmd NULL

	/* BEGIN: Added by weizengke, 2013/10/4   PN:check cmd valid*/
	if (cmd == NULL || 0 == strlen(cmd))
	{
		debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd_filter_command, the param cmd is null.");
		return 0;
	}

	/* <CR> 不参与过滤，防止命令行子串也属于命令行时误过滤 */
	if (0 == strcmp(cmd, "<CR>"))
	{
		debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd_filter_command, the param cmd is <CR>.");
		return 0;
	}

	/* END:   Added by weizengke, 2013/10/4   PN:None */

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "In cmd_filter_command. (cmd=%s,size=%d)", cmd, strlen(cmd));

	for (i = 0; i < cmd_vector_max(v); i++)
	{
		if ((elem = (struct cmd_elem_st*)cmd_vector_slot(v, i)) != NULL)
		{
			if (index >= cmd_vector_max(elem->para_vec))
			{
				debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd_filter_command. for loop -> %d filter. (cmd=%s)", i, cmd);
				cmd_vector_slot(v, i) = NULL;
				continue;
			}

			desc = (struct para_desc *)cmd_vector_slot(elem->para_vec, index);

			debug_print_ex(CMD_DEBUG_TYPE_FUNC, "In cmd_filter_command. for loop -> %d. (cmd=%s,desc->para=%s)", i, cmd, desc->para);

			/* match STRING , INTEGER */
			if (CMD_OK != cmd_match_special_string(cmd, desc->para))
			{
				debug_print_ex(CMD_DEBUG_TYPE_FUNC, "In cmd_filter_command. cmd_match_special_string return ERR. (cmd=%s,para=%s)",cmd,desc->para);
				if(strncmp(cmd, desc->para, strlen(cmd)) != 0)
				{
					debug_print_ex(CMD_DEBUG_TYPE_INFO, "In cmd_filter_command. for loop -> %d filter. (cmd=%s,desc->para=%s)", i, cmd, desc->para);
					cmd_vector_slot(v, i) = NULL;
				}
			}

		}
	}

	return 0;
}


/*** ---------------- Interface Function ------------------ ***/
int cmd_match_command(cmd_vector_t *icmd_vec, struct cmd_vty *vty,
		struct para_desc **match, int *match_size, char *lcd_str)
{
	int i;
	cmd_vector_t *cmd_vec_copy = cmd_vector_copy(cmd_vec);
	int isize = 0;
	int size = 0;

	CMD_NOUSED(vty);
	CMD_NOUSED(lcd_str);

	// Three steps to find matched commands in 'cmd_vec'
	// 1. for input command vector 'icmd_vec', check if it is matching cmd_vec
	// 2. store last matching command parameter in cmd_vec into match_vec
	// 3. according to match_vec, do no match, one match, part match, list match

	if (icmd_vec == NULL)
	{
		debug_print_ex(CMD_DEBUG_TYPE_ERROR,"In cmd_match_command, icmd_vec is null.");
		return CMD_NO_MATCH;
	}

	isize = cmd_vector_max(icmd_vec) - 1;
	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "In cmd_match_command. (isize=%d)", isize);

	// Step 1
	for (i = 0; i < isize; i++)
	{
		char *ipara = (char*)cmd_vector_slot(icmd_vec, i);
		cmd_filter_command(ipara, cmd_vec_copy, i);

		debug_print_ex(CMD_DEBUG_TYPE_FUNC, "In cmd_match_command, cmd_filter_command(ipara=%s, i=%d)", ipara, i);
	}

	// Step 2
	// insert matched command word into match_vec, only insert the next word
	for(i = 0; i < cmd_vector_max(cmd_vec_copy); i++)
	{

		debug_print_ex(CMD_DEBUG_TYPE_FUNC, "In cmd_match_command, for loop -> %d.", i);

		struct cmd_elem_st *elem = NULL;
		elem = (struct cmd_elem_st *)cmd_vector_slot(cmd_vec_copy, i);

		if(elem != NULL)
		{
			if (elem->para_vec == NULL)
			{
 				debug_print_ex(CMD_DEBUG_TYPE_ERROR, " ++ In cmd_match_command, for loop -> %d. elem->para_vec is null", i);
				continue;
 			}

 			debug_print_ex(CMD_DEBUG_TYPE_FUNC, "In cmd_match_command, for loop -> %d, (isize=%d, v_size=%d)", i, isize, cmd_vector_max(elem->para_vec));

			if (isize >= cmd_vector_max(elem->para_vec))
			{
				cmd_vector_slot(cmd_vec_copy, i) = NULL;
				continue;
			}

			struct para_desc *desc = (struct para_desc *)cmd_vector_slot(elem->para_vec, isize);
			char *str = (char*)cmd_vector_slot(icmd_vec, isize);

			if (str == NULL || strncmp(str, desc->para, strlen(str)) == 0)
			{
				if (match_unique_string(match, desc->para, size))
					match[size++] = desc;
			}

		}
		else
		{
			debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd_match_command, for loop -> %d, elem is null.", i);
		}
	}

	cmd_vector_deinit(cmd_vec_copy, 0);	// free cmd_vec_copy, no longer use

	// Step 3
	// No command matched
	if (size == 0) {
		*match_size = size;
		return CMD_NO_MATCH;
	}

	// Only one command matched
	if (size == 1) {
		*match_size = size;
		return CMD_FULL_MATCH;
	}

#if 0
	/* dele the part match */

	// Make it sure last element is NULL
	if (cmd_vector_slot(icmd_vec, isize) != NULL) {
		int lcd = match_lcd(match, size);
		if(lcd) {
			int len = strlen((char*)cmd_vector_slot(icmd_vec, isize));
			if (len < lcd) {
				memcpy(lcd_str, match[0]->para, lcd);
				lcd_str[lcd] = '\0';
				*match_size = size = 1;
				return CMD_PART_MATCH;
			}
		}
	}
#endif
	*match_size = size;
	return CMD_LIST_MATCH;
}

/*****************************************************************************
 Prototype    : cmd_complete_command
 Description  : complete command　for ? complete
 Input        : cmd_vector_t *icmd_vec  input cmd vector
                struct cmd_vty *vty     the input vty

 Output       : char **doc              the return doc
 Return Value :
 Calls        :
 Called By    :

  History        :
  1.Date         : 2013/10/4
    Author       : weizengke
    Modification : Created function

*****************************************************************************/
int cmd_complete_command(cmd_vector_t *icmd_vec, struct cmd_vty *vty, struct para_desc **match, int *match_size)
{
	int i;
	cmd_vector_t *cmd_vec_copy = cmd_vector_copy(cmd_vec);
	int match_num = 0;

	char *str;
	struct para_desc *para_desc_;
	struct cmd_elem_st *elem;

	if (icmd_vec == NULL || vty == NULL || match == NULL || match_size == NULL)
	{
		debug_print_ex(CMD_DEBUG_TYPE_ERROR, "in cmd_complete_command, param is null");
		return 1;
	}

	// Two steps to find matched commands in 'cmd_vec'
	// 1. for input command vector 'icmd_vec', check if it is matching cmd_vec
	// 2. check last matching command parameter in cmd_vec match cmd_vec

	// Step 1
	/* BEGIN: Modified by weizengke, 2013/10/4   PN:循环过滤每一个向量 */
	for (i = 0; i < cmd_vector_max(icmd_vec); i++)
	{
		cmd_filter_command((char*)cmd_vector_slot(icmd_vec, i), cmd_vec_copy, i);
	}
	/* END:   Modified by weizengke, 2013/10/4   PN:None */

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "In cmd_complete_command, after cmd_filter_command, cmd_vector_max(cmd_vec_copy)=%d",cmd_vector_max(cmd_vec_copy));

	// Step 2
	// insert matched command word into match_vec, only insert the next word
	/* BEGIN: Added by weizengke, 2013/10/4   PN:only the last vector we need */
	for(i = 0; i < cmd_vector_max(cmd_vec_copy); i++)
	{
		debug_print_ex(CMD_DEBUG_TYPE_FUNC, "In cmd_complete_command, for loop->i = %d. (icmd_vec_size=%d, cmd_vec_size=%d)", i, cmd_vector_max(icmd_vec), cmd_vector_max(cmd_vec_copy));

		elem = (struct cmd_elem_st *)cmd_vector_slot(cmd_vec_copy, i);
		if(elem  != NULL)
		{
			if (cmd_vector_max(icmd_vec) - 1 >= cmd_vector_max(elem->para_vec))
			{
				cmd_vector_slot(cmd_vec_copy, i) = NULL;
				continue;
			}

			str = (char*)cmd_vector_slot(icmd_vec, cmd_vector_max(icmd_vec) - 1);
			para_desc_ = (struct para_desc*)cmd_vector_slot(elem->para_vec, cmd_vector_max(icmd_vec) - 1);
			if (para_desc_ == NULL)
			{
				debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd_complete_command, para_desc_ is null");
				continue;
			}

			debug_print_ex(CMD_DEBUG_TYPE_FUNC, "In cmd_complete_command, for loop->i = %d, (str=%s, id=%d, para=%s)", i, str, para_desc_->elem_id, para_desc_->para);

			if ( str != NULL &&
			     (strncmp(str, "<CR>", strlen(str)) == 0
			     ||(strncmp(str, para_desc_->para, strlen(str)) == 0)))
			{

				if (match_unique_string(match, para_desc_->para, match_num))
				{
					debug_print_ex(CMD_DEBUG_TYPE_INFO, "In cmd_complete_command, match(%s).", para_desc_->para);
					match[match_num++] = para_desc_;
				}
			}

		}
	}

	cmd_vector_deinit(cmd_vec_copy, 0);	// free cmd_vec_copy, no longer use

	/* BEGIN: Added by weizengke, 2013/10/27 sort for ? complete */
	{
		int j;
		for (i = 0; i < match_num - 1; i++)
		{
			for (j = i; j < match_num; j++)
			{
				struct para_desc *para_desc__ = NULL;
				if (1 == strcmp(match[i]->para, match[j]->para))
				{
					para_desc__ = match[i];
					match[i] =  match[j];
					match[j] = para_desc__;
				}
			}
		}
	}
	/* END: Added by weizengke, 2013/10/27 sort for ? complete */

	*match_size = match_num;

	return 0;
}

int cmd_execute_command(cmd_vector_t *icmd_vec, struct cmd_vty *vty, struct para_desc **match, int *match_size)
{
	int i;
	cmd_vector_t *cmd_vec_copy = cmd_vector_copy(cmd_vec);

	struct cmd_elem_st *match_elem = NULL;
	int match_num = 0;

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "In cmd_execute_command, input_command_size=%d. (include <CR>)", cmd_vector_max(icmd_vec));
	/*
	Two steps to find matched commands in 'cmd_vec'
	 1. for input command vector 'icmd_vec', check if it is matching cmd_vec
	 2. check last matching command parameter in cmd_vec match cmd_vec
	*/

	/* Step 1 */
	for (i = 0; i < cmd_vector_max(icmd_vec); i++)
	{
		cmd_filter_command((char*)cmd_vector_slot(icmd_vec, i), cmd_vec_copy, i);
	}

	/* Step 2 */
	/*  insert matched command word into match_vec, only insert the next word */
	for(i = 0; i < cmd_vector_max(cmd_vec_copy); i++) {
		char *str;
		struct para_desc *desc;
		struct cmd_elem_st *elem = NULL;

		elem = (struct cmd_elem_st *)cmd_vector_slot(cmd_vec_copy, i);
		if(elem != NULL) {
			str = (char*)cmd_vector_slot(icmd_vec, cmd_vector_max(icmd_vec) - 1);
			desc = (struct para_desc *)cmd_vector_slot(elem->para_vec, cmd_vector_max(icmd_vec) - 1);

			debug_print_ex(CMD_DEBUG_TYPE_FUNC, "In cmd_execute_command, loop->%d. (str=%s, para=%s, desc=%s)", i, str, desc->para, desc->desc);

			/* modified for command without argv */
			if (cmd_vector_max(icmd_vec) == cmd_vector_max(elem->para_vec))
			{
				/* BEGIN: Added by weizengke, 2013/10/5   PN:for support STRING<a-b> & INTEGER<a-b> */
				if (CMD_OK == cmd_match_special_string(str, desc->para) ||
					str != NULL && strncmp(str, desc->para, strlen(str)) == 0)
				{
					/* BEGIN: Added by weizengke, 2013/10/6   PN:command exec ambigous, return the last elem (not the <CR>) */
					match[match_num] = (struct para_desc *)cmd_vector_slot(elem->para_vec, cmd_vector_max(icmd_vec) - 2);
					/* END:   Added by weizengke, 2013/10/6   PN:None */

					debug_print_ex(CMD_DEBUG_TYPE_INFO, "***** In cmd_execute_command, match one **** (match_string=%s)", str, match[match_num]->para);

					match_num++;
					match_elem = elem;

				}
			}
		}
	}

	*match_size = match_num;


	cmd_vector_deinit(cmd_vec_copy, 0);

	/* check if exactly match */
	if (match_num == 0)
	{
		return CMD_NO_MATCH;
	}

	/* BEGIN: Added by weizengke, 2013/10/6   PN:command exec ambigous */
	if (match_num > 1)
	{
		return CMD_ERR_AMBIGOUS;
	}
	/* END:   Added by weizengke, 2013/10/6   PN:None */

	/* BEGIN: Added by weizengke, 2013/10/5  for support argv. PN:push param into function stack */
	/* now icmd_vec and match_elem will be the same vector ,can push into function stack */
	int argc = 0;
	char *argv[CMD_MAX_CMD_NUM];

	if (NULL == match_elem || match_elem->para_vec == NULL)
	{
		debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd_execute_command, bug of match_elem is NULL........");
		return CMD_NO_MATCH;
	}

	for (i = 0, argc = 0; i < match_elem->para_num; i ++)
	{
		struct para_desc  *desc = (struct para_desc *)cmd_vector_slot(match_elem->para_vec, i);
		if (NULL == desc)
		{
			debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd_execute_command, bug of desc is NULL........");
			return CMD_NO_MATCH;
		}

		debug_print_ex(CMD_DEBUG_TYPE_FUNC, "elem_id=%d, elem_tpye=%d, para=%s(%s)", desc->elem_id, desc->elem_tpye, desc->para, (char*)cmd_vector_slot(icmd_vec, i));

		/* <CR> no need to push */
		if (desc->elem_tpye == CMD_ELEM_TYPE_END)
		{
			continue;
		}

		/* push param to argv and full command*/
		if (CMD_YES == cmd_elem_is_para_type(desc->elem_tpye))
		{
			argv[argc++] = (char*)cmd_vector_slot(icmd_vec, i);
		}
		else
		{
			argv[argc++] = desc->para;
		}
	}
	/* END:   Added by weizengke, 2013/10/5   PN:None */

	/* execute command */
	(*match_elem->func)(match_elem, vty, argc, argv);

	return CMD_FULL_MATCH;
}

void install_element(struct cmd_elem_st *elem)
{
	if (cmd_vec == NULL) {
		debug_print_ex(CMD_DEBUG_TYPE_ERROR, "Command Vector Not Exist");
		exit(1);
	}

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "install_element, string(%s), doc(%s).", elem->string, elem->doc);

	cmd_vector_insert(cmd_vec, elem);
	elem->para_vec = cmd2vec(elem->string, elem->doc);
	elem->para_num = cmd_vector_max(elem->para_vec);

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "install_element ok, string(%s), generate para_num(%d). ", elem->string, elem->para_num);

}


/* end cmd */


/* resolve */

/* ------------------ Auxiliary Function ----------------- */
#define ANOTHER_LINE(i)	(((i) != 0) && ((i) % CMD_LINE_NUM == 0))

// insert a word into the tail of input buffer
static void cmd_insert_word(struct cmd_vty *vty, const char *str)
{
	strcat(vty->buffer, str);
	vty->cur_pos += (int)strlen(str);
	vty->used_len += (int)strlen(str);
}

// delete the last word from input buffer
static void cmd_delete_word(struct cmd_vty *vty)
{
	int pos = strlen(vty->buffer);

	while (pos > 0 && vty->buffer[pos - 1] != ' ') {
		pos--;
	}

	vty->buffer[pos] = '\0';
	vty->cur_pos = strlen(vty->buffer);
	vty->used_len = strlen(vty->buffer);
}

static inline void free_matched(char **matched)
{
	int i;

	if (!matched)
		return;
	for (i = 0; matched[i] != NULL; i++)
		free(matched[i]);
	free(matched);
}


int g_InputMachine_prev = CMD_KEY_CODE_NOTCARE;
int g_InputMachine_now = CMD_KEY_CODE_NOTCARE;

char g_tabbingString[CMD_MAX_CMD_ELEM_SIZE] = {0};  /* 最初始用来补全查找的字串*/
char g_tabString[CMD_MAX_CMD_ELEM_SIZE] = {0};      /* 最后一次补全的命令 */
int g_tabStringLenth = 0;

/* ------------------ Interface Function ----------------- */
int cmd_resolve(char c)
{
	int key_type = CMD_KEY_CODE_NOTCARE;	// default is not special key

	switch (c) {
	case CMD_KEY_ARROW1:
		c = cmd_getch();
		#ifdef _LINUX_
		if (c == CMD_KEY_ARROW2)
		{
			c = cmd_getch();
		#endif
			switch (c) {
			case CMD_KEY_UP:
				key_type = CMD_KEY_CODE_UP;
				break;
			case CMD_KEY_DOWN:
				key_type = CMD_KEY_CODE_DOWN;
				break;
			case CMD_KEY_RIGHT:
				key_type = CMD_KEY_CODE_RIGHT;
				break;
			case CMD_KEY_LEFT:
				key_type = CMD_KEY_CODE_LEFT;
				break;
			default:
				break;
			}
		#ifdef _LINUX_
		}
		#endif
		break;
#ifndef _LINUX_  /* windwos */
		case CMD_KEY_ARROW2:
			c = cmd_getch();
			switch (c) {
			case CMD_KEY_UP:
				key_type = CMD_KEY_CODE_UP;
				break;
			case CMD_KEY_DOWN:
				key_type = CMD_KEY_CODE_DOWN;
				break;
			case CMD_KEY_RIGHT:
				key_type = CMD_KEY_CODE_RIGHT;
				break;
			case CMD_KEY_LEFT:
				key_type = CMD_KEY_CODE_LEFT;
				break;
			default:
				break;
			}
		break;
#endif
	case CMD_KEY_DELETE:
		key_type = CMD_KEY_CODE_DELETE;
		break;
	case CMD_KEY_BACKSPACE:
	case CMD_KEY_CTRL_H:
		/* Linux 下空格后回车无法tab补全与'?'联想 待修复*/
		break;
	case '\t':
		key_type = CMD_KEY_CODE_TAB;
		break;
	case '\r':
	case '\n':
		key_type = CMD_KEY_CODE_ENTER;
		break;
	case '?':
		/* BEGIN: Added by weizengke, 2013/10/4   PN:need print '?' */
		cmd_put_one('?');
		/* END:   Added by weizengke, 2013/10/4   PN:need print '?' */
		key_type = CMD_KEY_CODE_QUEST;
		break;
	default:
		break;
	}

	return key_type;
}
/* end key*/
/*
int cmd_resolve(char c)
{
	int key_type = CMD_KEY_CODE_NOTCARE;	// default is not special key

	switch (c) {
		case CMD_KEY_ARROW1:
			if (cmd_getch() == CMD_KEY_ARROW2) {
				c = cmd_getch();	// get real key
				switch (c) {
					case CMD_KEY_UP:
						key_type = CMD_KEY_CODE_UP;
						break;
					case CMD_KEY_DOWN:
						key_type = CMD_KEY_CODE_DOWN;
						break;
					case CMD_KEY_RIGHT:
						key_type = CMD_KEY_CODE_RIGHT;
						break;
					case CMD_KEY_LEFT:
						key_type = CMD_KEY_CODE_LEFT;
						break;
					default:
						break;
				}
			}
			break;
		case CMD_KEY_BACKSPACE:
		case CMD_KEY_CTRL_H:
			key_type = CMD_KEY_CODE_DELETE;
			break;
		case '\t':
			key_type = CMD_KEY_CODE_TAB;
			break;
		case '\r':
		case '\n':
			key_type = CMD_KEY_CODE_ENTER;
			break;
		case '?':
			key_type = CMD_KEY_CODE_QUEST;
			break;
		default:
			break;
	}

	return key_type;
}
*/

// resolve a whole line, including tab, enter, quest, up, down
void cmd_resolve_tab(struct cmd_vty *vty)
{
	int i;
	cmd_vector_t *v;
	struct para_desc *match[CMD_MAX_MATCH_SIZE] = {0};	// matched string
	int match_size = 0;
	int match_type = CMD_NO_MATCH;
	int isNeedMatch = 1;   /* 非TAB场景(无空格)，不需要匹配 */
	char lcd_str[1024] = {0};	// if part match, then use this
	char *last_word = NULL;

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "TAB for completing command. (buf=%s)", vty->buffer);

	if (g_InputMachine_prev == CMD_KEY_CODE_TAB)
	{
		debug_print_ex(CMD_DEBUG_TYPE_FUNC, "TAB for completing command. continue tabMachine. (g_tabbingString=%s)", g_tabbingString);
		cmd_delete_word(vty);
		cmd_insert_word(vty, g_tabbingString);
	}
	else
	{
		memset(g_tabString,0,sizeof(g_tabString));
		g_tabStringLenth = 0;
	}

	v = str2vec(vty->buffer);
	if (v == NULL)
	{
		debug_print_ex(CMD_DEBUG_TYPE_INFO, "TAB for completing command. str2vec return is null. (buf=%s)", vty->buffer);
		/*
		v = cmd_vector_init(1);
		cmd_vector_insert(v, '\0');
		*/
		isNeedMatch = 0;
	}

	if (isspace((int)vty->buffer[vty->used_len - 1]))
	{
		debug_print_ex(CMD_DEBUG_TYPE_INFO, "TAB for completing command. the last one is space (buf=%s)", vty->buffer);
		isNeedMatch = 0;
	}

	if (1 == isNeedMatch && NULL != v)
	{
		match_type = cmd_match_command(v, vty, match, &match_size, lcd_str);

		last_word = (char*)cmd_vector_slot(v, cmd_vector_max(v) - 1);

		if (g_InputMachine_prev != CMD_KEY_CODE_TAB)
		{
			strcpy(g_tabbingString, last_word);
		}

		debug_print_ex(CMD_DEBUG_TYPE_INFO, "TAB for completing command. the last word is (last_word=%s, vector_size=%d)", last_word, cmd_vector_max(v) - 1);

		cmd_vector_deinit(v, 1);
	}

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "TAB for completing command. after cmd_match_command. (match_type=%d)", match_type);

	cmd_outstring("%s", CMD_ENTER);
	switch (match_type) {
		case CMD_NO_MATCH:
			cmd_outprompt(vty->prompt);
			cmd_outstring("%s", vty->buffer);
			break;
		case CMD_FULL_MATCH:
			cmd_delete_word(vty);
			if (NULL != match[0])
			{
				cmd_insert_word(vty, match[0]->para);
			}
			/* BEGIN: Added by weizengke, 2013/10/14 for full match then next input*/
			cmd_insert_word(vty, " ");
			/* END:   Added by weizengke, 2013/10/14 */
			cmd_outprompt(vty->prompt);
			cmd_outstring("%s", vty->buffer);

			/* BEGIN: Added by weizengke, 2013/10/27 PN: fix the bug of CMD_FULL_MATCH and then continue TAB*/
			memset(g_tabString,0,sizeof(g_tabString));
			memset(g_tabbingString,0,sizeof(g_tabbingString));
			g_tabStringLenth = 0;
			/* END:   Added by weizengke, 2013/10/27 */

			break;
		case CMD_PART_MATCH:
			/*  delete at 2013-10-05, CMD_PART_MATCH will never reach, CMD_LIST_MATCH instead.

				case like this:
					disable , display
					>di  TAB
					>dis  ==> CMD_PART_MATCH
			*/
			cmd_delete_word(vty);
			cmd_insert_word(vty, lcd_str);
			cmd_outprompt(vty->prompt);
			cmd_outstring("%s", vty->buffer);
			break;
		case CMD_LIST_MATCH:

			if (g_InputMachine_prev != CMD_KEY_CODE_TAB)
			{
				debug_print_ex(CMD_DEBUG_TYPE_FSM, "TAB for completing command. enter tabMachine. (g_tabString=%s)", g_tabString);
				memset(g_tabString,0,sizeof(g_tabString));
				strcpy(g_tabString, match[0]->para);
				g_tabStringLenth = strlen(g_tabString);

				/* cmd_outstring("%s", CMD_ENTER); */
			}
			else
			{
				debug_print_ex(CMD_DEBUG_TYPE_FSM, "TAB for completing command. continue tabMachine. (g_tabString=%s)", g_tabString);

				for (i = 0; i < match_size; i++)
				{
					if (0 == strcmp(g_tabString, match[i]->para))
					{
						break;
					}
				}

				debug_print_ex(CMD_DEBUG_TYPE_FSM, "TAB for completing command. continue tabMachine. (i=%d, match_size=%d)", i, match_size);

				if (i == match_size)
				{
					debug_print_ex(CMD_DEBUG_TYPE_ERROR, "TAB for completing command. bug of tab continue. (g_tabString=%s)", g_tabString);
				}

				i++;

				if (i == match_size)
				{
					i = 0;
				}

				memset(g_tabString,0,sizeof(g_tabString));
				strcpy(g_tabString, match[i]->para);
				g_tabStringLenth = strlen(g_tabString);

				debug_print_ex(CMD_DEBUG_TYPE_FSM, "TAB for completing command. continue tabMachine. (match[i]->para=%s, g_tabStringLenth=%d)", match[i]->para, g_tabStringLenth);

			}

			/*for (i = 0; i < match_size; i++) {
				if (ANOTHER_LINE(i))
					cmd_outstring("%s", CMD_ENTER);
				cmd_outstring("%-25s", match[i]->para);
			}
			*/

			cmd_delete_word(vty);
			cmd_insert_word(vty, g_tabString);

			cmd_outprompt(vty->prompt);
			cmd_outstring("%s", vty->buffer);
			break;
		default:
			break;
	}
}

/*

bugs:
	1:	display loopback-detect brief
		display loopback brief

		<Jungle-config>dis loop brief
		Command 'dis loop brief ' anbigous follow:
		 brief                     brief
		<Jungle-config>

	2:
		display loopback-detect brief
		display loopback brief
		disable loopback-detect

		<Jungle-config>dis loopback
		Command 'dis loopback' anbigous follow:
		 loopback                  loopback-detect
		<Jungle-config>


*/
void cmd_resolve_enter(struct cmd_vty *vty)
{
	struct para_desc *match[CMD_MAX_MATCH_SIZE];	// matched string
	int match_size = 0;

	int match_type = CMD_NO_MATCH;
	cmd_vector_t *v;

	int i = 0;

	v = str2vec(vty->buffer);
	if (v == NULL) {
		cmd_outstring("%s", CMD_ENTER);
		cmd_outprompt(vty->prompt);
		return;
	}

	/* BEGIN: Added by weizengke, 2013/10/5   PN:for cmd end with <CR> */
	cmd_vector_insert_cr(v);
	/* END:   Added by weizengke, 2013/10/5   PN:None */

	cmd_outstring("%s", CMD_ENTER);

	// do command
	match_type = cmd_execute_command(v, vty, match, &match_size);
	// add executed command into history
	cmd_vty_add_history(vty);

	if (match_type == CMD_NO_MATCH)
	{
		cmd_outstring("Unknown Command");
		cmd_outstring("%s", CMD_ENTER);
	}
	else if (match_type == CMD_ERR_ARGU)
	{
		cmd_outstring("Too Many Arguments");
		cmd_outstring("%s", CMD_ENTER);
	}

	if (match_type == CMD_ERR_AMBIGOUS)
	{
		if (match_size)
		{
			cmd_outstring("Command '%s' anbigous follow:", vty->buffer);
			cmd_outstring("%s", CMD_ENTER);
			for (i = 0; i < match_size; i++)
			{
				if (ANOTHER_LINE(i))
					cmd_outstring("%s", CMD_ENTER);
				cmd_outstring(" %-25s", match[i]->para);
			}
			cmd_outstring("%s", CMD_ENTER);
			/* del 10-29
			cmd_outprompt(vty->prompt);
			cmd_outstring("%s", vty->buffer);
			*/
			vty->cur_pos = vty->used_len = 0;
			memset(vty->buffer, 0, vty->buf_len);
			cmd_outprompt(vty->prompt);
		}
	}
	else
	{
		// ready for another command
		vty->cur_pos = vty->used_len = 0;
		memset(vty->buffer, 0, vty->buf_len);
		cmd_outprompt(vty->prompt);
	}
}

/*
   1: 完全匹配输入时，只返回该命令的联想  2013-10-27 (未实现)

   2:bug:
   		display loopback
   		disable loopback-detect
   	  >dis loop   =======> show what
*/
void cmd_resolve_quest(struct cmd_vty *vty)
{
	cmd_vector_t *v;
	struct para_desc *match[CMD_MAX_MATCH_SIZE];	// matched string
	int match_size = 0;
	int i = 0;

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "'?' for associating command. (buf=%s)", vty->buffer);

	v = str2vec(vty->buffer);
	if (v == NULL)
	{
		debug_print_ex(CMD_DEBUG_TYPE_INFO, "'?' for associating command. after str2vec, v is null (buf=%s)", vty->buffer);

		v = cmd_vector_init(1);
		cmd_vector_insert_cr(v);
	}
	else if (isspace((int)vty->buffer[vty->used_len - 1]))
	{
		debug_print_ex(CMD_DEBUG_TYPE_INFO, "'?' for associating command. the last one is space (buf=%s)", vty->buffer);
		cmd_vector_insert_cr(v);
	}

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "In cmd_resolve_quest, after str2vec, get %d vector.", v->size);

	for (i = 0; i < cmd_vector_max(v); i++)
	{
		debug_print_ex(CMD_DEBUG_TYPE_INFO, "In cmd_resolve_quest, str=%s.", (char*)cmd_vector_slot(v, i));
	}

	cmd_complete_command(v, vty, match, &match_size);

	debug_print_ex(CMD_DEBUG_TYPE_FUNC, "In cmd_resolve_quest, after cmd_complete_command, get %d matched.", match_size);

	cmd_outstring("%s", CMD_ENTER);
	if (match_size) {
		for (i = 0; i < match_size; i++) {
			cmd_outstring(" %-25s%s\r\n", match[i]->para,match[i]->desc);
		}
		cmd_outprompt(vty->prompt);
		cmd_outstring("%s", vty->buffer);
	} else {
		cmd_outstring("Unknow Command%s", CMD_ENTER);
		cmd_outprompt(vty->prompt);
		cmd_outstring("%s", vty->buffer);
	}

	cmd_vector_deinit(v, 0);
}

/* bug of up twice with last key is not up, the hpos not restart */
void cmd_resolve_up(struct cmd_vty *vty)
{
	int try_idx = vty->hpos == 0 ? (HISTORY_MAX_SIZE - 1) : vty->hpos - 1;

	// if no history
	if (vty->history[try_idx] == NULL)
		return;
	vty->hpos = try_idx;

	// print history command
	cmd_clear_line(vty);
	strcpy(vty->buffer, vty->history[vty->hpos]);
	vty->cur_pos = vty->used_len = strlen(vty->history[vty->hpos]);
	cmd_outstring("%s", vty->buffer);
}

void cmd_resolve_down(struct cmd_vty *vty)
{
	int try_idx = vty->hpos ==( HISTORY_MAX_SIZE - 1) ? 0 : vty->hpos + 1;

	// if no history
	if (vty->history[try_idx] == NULL)
		return;
	vty->hpos = try_idx;

	// print history command
	cmd_clear_line(vty);
	strcpy(vty->buffer, vty->history[vty->hpos]);
	vty->cur_pos = vty->used_len = strlen(vty->history[vty->hpos]);
	cmd_outstring("%s", vty->buffer);
}

// handle in read buffer, including left, right, delete, insert
void cmd_resolve_left(struct cmd_vty *vty)
{
	// already at leftmost, cannot move more
	if (vty->cur_pos <= 0)
		return;
	// move left one step
	vty->cur_pos--;
	cmd_back_one();
}

void cmd_resolve_right(struct cmd_vty *vty)
{
	// already at rightmost, cannot move more
	if (vty->cur_pos >= vty->used_len)
		return;
	// move right one step
	cmd_put_one(vty->buffer[vty->cur_pos]);
	vty->cur_pos++;
}

void cmd_resolve_delete(struct cmd_vty *vty)
{
	int i, size;

	// no more to delete
	if (vty->cur_pos <= 0)
		return;
	size = vty->used_len - vty->cur_pos;
	CMD_DBGASSERT(size >= 0);

	// delete char
	vty->cur_pos--;
	vty->used_len--;
	cmd_back_one();

	// print left chars
	memcpy(&vty->buffer[vty->cur_pos], &vty->buffer[vty->cur_pos + 1], size);
	vty->buffer[vty->used_len] = '\0';
	for (i = 0; i < size; i ++)
		cmd_put_one(vty->buffer[vty->cur_pos + i]);
	cmd_put_one(' ');
	for (i = 0; i < size + 1; i++)
		cmd_back_one();
}

void cmd_resolve_insert(struct cmd_vty *vty)
{
	int i, size;

	// no more to insert
	if (vty->used_len >= vty->buf_len)
		return;
	size = vty->used_len - vty->cur_pos;
	CMD_DBGASSERT(size >= 0);

	memcpy(&vty->buffer[vty->cur_pos + 1], &vty->buffer[vty->cur_pos], size);
	vty->buffer[vty->cur_pos] = vty->c;
	/* BEGIN: Added by weizengke, 2013/10/4   PN:bug for continue tab */
	vty->buffer[vty->cur_pos + 1] = '\0';
	/* END:   Added by weizengke, 2013/10/4   PN:None */
	// print left chars, then back size
	for (i = 0; i < size + 1; i++)
		cmd_put_one(vty->buffer[vty->cur_pos + i]);
	for (i = 0; i < size; i++)
		cmd_back_one();

	vty->cur_pos++;
	vty->used_len++;

}

key_handler_t key_resolver[] = {
	// resolve a whole line
	{ CMD_KEY_CODE_TAB, 	cmd_resolve_tab },
	{ CMD_KEY_CODE_ENTER, 	cmd_resolve_enter },
	{ CMD_KEY_CODE_QUEST, 	cmd_resolve_quest },
	{ CMD_KEY_CODE_UP, 		cmd_resolve_up },
	{ CMD_KEY_CODE_DOWN, 	cmd_resolve_down },
	// resolve in read buffer
	{ CMD_KEY_CODE_LEFT, 	cmd_resolve_left },
	{ CMD_KEY_CODE_RIGHT, 	cmd_resolve_right },
	{ CMD_KEY_CODE_DELETE, 	cmd_resolve_delete },
	{ CMD_KEY_CODE_NOTCARE, cmd_resolve_insert },
};
/* end resolve */



/* api */

// Define your API function here
/*
 * Three part: function declaration; define structure; function definition
 * For part one - function declaration:
 *			= Advise make function name as test function name plus t
 *			  e.g. To test function: parse_macaddr(which is a function in your interface), then
 *				   define its name as 'parse_macaddr_t'
 * For part two - define structure:
 *			= Advise make structure name as test function name plus n
 *			  e.g. So parse_macaddr has struct name 'parse_macaddr_n'
 *			  Then define three member: cmdstr, funcname, helpstr
 *				- cmdstr: Advise test function name, so let it be "parse_macaddr"
 *				- funcname: function name in part one, so be parse_macaddr_t, intention, this is not string
 *				- helpstr: words to explain your function, including usage, purpose and parameters
 * For part three - function definition
 *			= Define your function, notice argc and argv starts from parameters, so not including
 *			"parse_macaddr" itself
 *
 * Below is an example, you can copy, paste, modify, and compile
 */

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


void cmd_init()
{
	// initial cmd vector
	cmd_vec = cmd_vector_init(1);

	//reg cmd-element
	cmd_reg_newcmdelement(CMD_ELEM_ID_CR, 			CMD_ELEM_TYPE_END,			"<CR>",			    ""               );
	cmd_reg_newcmdelement(CMD_ELEM_ID_STRING1TO24,  CMD_ELEM_TYPE_STRING,       "STRING<1-24>",     "String lenth range form 1 to 24");
	cmd_reg_newcmdelement(CMD_ELEM_ID_INTEGER1TO24, CMD_ELEM_TYPE_INTEGER,      "INTEGER<1-100>",   "Integer range form 1 to 100");

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

	// ---------------------------------------------------

/*
	for (int i=0; i < sizeof(g_cmd_elem)/sizeof(para_desc); i++)
	{
		printf("%d  %s  %s\n", g_cmd_elem[i].elem_id, g_cmd_elem[i].para, g_cmd_elem[i].desc);
	}
*/
}



/* io */

void cmd_back_one()
{
	printf("\b");
}

void cmd_put_one(char c)
{
	printf("%c", c);
}

void cmd_read(struct cmd_vty *vty)
{
	int key_type;

	g_InputMachine_prev = CMD_KEY_CODE_NOTCARE;
	g_InputMachine_now = CMD_KEY_CODE_NOTCARE;

	while ((vty->c = cmd_getch()) != EOF) {
		// step 1: get input key type
		key_type = cmd_resolve(vty->c);

		g_InputMachine_now = key_type;


		if (key_type <= CMD_KEY_CODE_NONE || key_type > CMD_KEY_CODE_NOTCARE) {
			debug_print_ex(CMD_DEBUG_TYPE_ERROR, "Unidentify Key Type, c = %c, key_type = %d\n", vty->c, key_type);
			continue;
		}

		// step 2: take actions according to input key
		key_resolver[key_type].key_func(vty);
		g_InputMachine_prev = g_InputMachine_now;

		if (g_InputMachine_now != CMD_KEY_CODE_TAB)
		{
			memset(g_tabString,0,sizeof(g_tabString));
			memset(g_tabbingString,0,sizeof(g_tabbingString));
			g_tabStringLenth = 0;
		}

	}
}

void cmd_outstring(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

void cmd_clear_line(struct cmd_vty *vty)
{
	int size = vty->used_len - vty->cur_pos;

	CMD_DBGASSERT(size >= 0);
	while (size--) {
		cmd_put_one(' ');
	}

	while (vty->used_len) {
		vty->used_len--;
		cmd_back_one();
		cmd_put_one(' ');
		cmd_back_one();
	}
	memset(vty->buffer, 0, HISTORY_MAX_SIZE);
}

struct cmd_vty *vty;



int cmd_main_entry ()
{
	cmd_init();

	vty = cmd_vty_init();
	if (vty == NULL)
	{
		exit(1);
	}

	pdt_debug_print("VRP Command-line init ok...");

	cmd_outprompt(vty->prompt);

	for (;;)
	{
		cmd_read(vty);
	}

	cmd_vty_deinit(vty);

	return 0;
}
