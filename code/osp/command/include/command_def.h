#ifndef _COMMAND_DEF_H_
#define _COMMAND_DEF_H_

#ifdef UCHAR
#undef UCHAR
#define UCHAR unsigned char
#endif

#ifndef ULONG
#undef ULONG
#define ULONG unsigned long
#endif

#ifndef CHAR
#undef CHAR
#define CHAR char
#endif

#ifndef VOID
#undef VOID
#define VOID void
#endif

#ifndef CMD_NULL_DWORD
#undef CMD_NULL_DWORD
#define CMD_NULL_DWORD 0xFFFFFFFF
#endif


/* config */
// log info level

#define LOG_DEBUG    3
#define LOG_WARNING  4
#define LOG_ERR      5

#define CMD_LOG_LEVEL	LOG_DEBUG

// enter may be '\n' or '\r\n'
#define CMD_ENTER			"\r\n"

// how much number of command ouput in one line
#define CMD_LINE_NUM		2

/* ------------------ Auxiliary Function ----------------- */
#define ANOTHER_LINE(i)	(((i) != 0) && ((i) % CMD_LINE_NUM == 0))

#define CMD_ERR 1
#define CMD_OK  0
#define CMD_YES 1
#define CMD_NO  0

#define CMD_NOUSED(x) ((x) = (x))

#define CMD_END         "<cr>"
#define CMD_INTEGER 	"INTEGER"
#define CMD_STRING  	"STRING"
#define CMD_IP  		"X.X.X.X"
#define CMD_MAC  		"H-H-H"

#define BUFSIZE 65535

#define CMD_DBGASSERT(x,...) if (0 == x) printf("\r\nAssert at %s:%d. ", __FILE__, __LINE__);

/* 
#define CMD_DBGASSERT(x,...) \
if (0 == x) {\
	printf("\r\nAssert at %s:%d. ", __FILE__, __LINE__);\
	//printf(x, ##__VA_ARGS__);\
	write_log(0, "Assert at %s:%d.", __FILE__, __LINE__);\
	write_log(0, ##__VA_ARGS__);\
}\
*/

#define cmd_vector_get(v, i)	    ((v)->ppData[(i)])
#define cmd_vector_size(v)		    ((v)->ulSize)

#ifdef _LINUX_
#define CMD_KEY_ARROW1	0x1b  		 
#define CMD_KEY_ARROW2	0x5b
#define CMD_KEY_ARROW3	0x33
#define CMD_KEY_UP		0x41  /* 1b 5b 41 */
#define CMD_KEY_DOWN 	0x42
#define CMD_KEY_RIGHT 	0x43
#define CMD_KEY_LEFT 	0x44
#define CMD_KEY_HOME	0x47
#define CMD_KEY_END	0x4f
#define CMD_KEY_PGUP	0x49
#define CMD_KEY_PHDN	0x51

#define CMD_KEY_DELETE     0x7e   /* 1b 5b 33 7e */
#define CMD_KEY_BACKSPACE  0x7f
#define CMD_KEY_CTRL_H	(0x1f | 0x7f)
#else
#define CMD_KEY_ARROW1	0xe0
#define CMD_KEY_ARROW2	0x0
#define CMD_KEY_UP		0x48          /* 0xe048 */
#define CMD_KEY_DOWN 	0x50		  /* 0xe050 */
#define CMD_KEY_RIGHT 	0x4d		  /* 0xe04d */
#define CMD_KEY_LEFT 	0x4b		  /* 0xe04b */
#define CMD_KEY_HOME	0x47		  /* 0xe047 */
#define CMD_KEY_END	0x4f		  /* 0xe04f */
#define CMD_KEY_PGUP	0x49		  /* 0xe049 */
#define CMD_KEY_PHDN	0x51		  /* 0xe051 */

#define CMD_KEY_DELETE  0x53		  /* 0xe053 */
#define CMD_KEY_BACKSPACE  0x08

#define CMD_KEY_CTRL_H	(0x1f | 0x7f)
#endif

#define CMD_KEY_CTRL_W	0x17
#define CMD_KEY_CR	    0x0d  /* '\r' */
#define CMD_KEY_LF	    0x0a  /* '\n' */
#define CMD_KEY_TAB	0x09  /* '\n' */
#define CMD_KEY_QUEST	0x3f /* '?'  */
#define CMD_KEY_SPACE 0x20
#define CMD_KEY_BACKSPACE_VTY 0x08
#define CMD_KEY_DELETE_VTY     0x7f

#define CMD_SYSNAME_SIZE    24

#define CMD_DEBUG_ERROR DEBUG_TYPE_ERROR
#define CMD_DEBUG_FUNC  DEBUG_TYPE_FUNC
#define CMD_DEBUG_INFO  DEBUG_TYPE_INFO
#define CMD_DEBUG_MSG   DEBUG_TYPE_MSG.

#endif

