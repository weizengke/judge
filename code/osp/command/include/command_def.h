#ifndef _COMMAND_DEF_H_
#define _COMMAND_DEF_H_

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

/* ------------------ Auxiliary Function ----------------- */
#define ANOTHER_LINE(i)	(((i) != 0) && ((i) % CMD_LINE_NUM == 0))


#define FNAME "cmd-sys.c"

#define CMD_ERR 1
#define CMD_OK  0

#define CMD_YES 1
#define CMD_NO  0

#define DEBUG_DISABLE 0
#define DEBUG_ENABLE  1

/* assert(0) */
#define CMD_DBGASSERT(x,y) if (0 == x) printf("Assert at %s", y);

#define CMD_NOUSED(x) ((x) = (x))


#define CMD_END         "<cr>"
#define CMD_INTEGER 	"INTEGER"
#define CMD_STRING  	"STRING"


#define cmd_vector_slot(v, i)	    ((v)->data[(i)])
#define cmd_vector_max(v)		    ((v)->used_size)

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
#define CMD_KEY_BACKSPACE  0x7e		 //0x08
#define CMD_KEY_SPACE   0x20		 //0x8
#define CMD_KEY_CTRL_H	(0x1f | 0x7f)
#else
#define CMD_KEY_ARROW1	0xffffffe0   //0x1b
#define CMD_KEY_ARROW2	0x0			 //0x5b
#define CMD_KEY_UP		0x48
#define CMD_KEY_DOWN 	0x50
#define CMD_KEY_RIGHT 	0x4d
#define CMD_KEY_LEFT 	0x4b
#define CMD_KEY_DELETE  0x53
#define CMD_KEY_BACKSPACE  0x08
#define CMD_KEY_SPACE   0x20
#define CMD_KEY_CTRL_H	(0x1f | 0x7f)

#define CMD_KEY_CTRL_W	0x17

#endif

#endif
