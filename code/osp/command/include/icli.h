#ifndef _COMMAND_H_
#define _COMMAND_H_

#ifndef UCHAR
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

#ifndef BOOL
#undef BOOL
#define BOOL bool
#endif

#ifndef FALSE
#undef FALSE
#define FALSE false
#endif

#ifndef TRUE
#undef TRUE
#define TRUE true
#endif

#ifdef _WIN32_
typedef SOCKET socket_t;
#else
typedef int socket_t;
#endif

/* 命令字类型 */
enum CMD_ELEM_TYPE_E
{
	CMD_ELEM_TYPE_VALID,
	CMD_ELEM_TYPE_KEY,     /* 关键字 */
	CMD_ELEM_TYPE_INTEGER, /* 整形参数 INTEGER<A-B>*/
	CMD_ELEM_TYPE_STRING,  /* 字符型参数 STRING<A-B>*/
	CMD_ELEM_TYPE_IP,  	   /* ip地址类型参数 X.X.X.X*/
	CMD_ELEM_TYPE_MAC,     /* MAC地址类型参数 H-H-H*/
	
	CMD_ELEM_TYPE_END,     /* 命令行结束符<CR> */
	CMD_ELEM_TYPE_MAX,
};

/* 
视图id，最大255
VIEW_GLOBAL表示任意视图
VIEW_USER表示用户视图
VIEW_SYSTEM表示系统视图
VIEW_DIAGNOSE表示诊断视图
如果用户自定义新增视图，需要定义从5开始
*/
enum CMD_VIEW_ID_E {
	VIEW_NULL = 0,
	VIEW_GLOBAL = 1,
	VIEW_USER,
	VIEW_SYSTEM,
	VIEW_JUDGE_MGR,
	VIEW_VJUDGE_MGR,
	VIEW_AAA,
	VIEW_USER_VTY,
	VIEW_DIAGNOSE,

	VIEW_ID_MAX = 255,	
};

#define CMD_ELMT_IP  		"X.X.X.X"
#define CMD_ELMT_MAC  		"H-H-H"


// maximum number of command to remember
#define HISTORY_MAX_SIZE	256

// maximum number of commands that can matched, 需要修改为动态分配
#define CMD_MAX_MATCH_SIZE	1024  

// maximum number of command element
#define CMD_MAX_CMD_NUM		16

// maximum length of command element
#define CMD_MAX_CMD_ELEM_SIZE 128

// maximum length of view name or ais-name
#define CMD_MAX_VIEW_SIZE   CMD_MAX_CMD_ELEM_SIZE

// size of input buffer size
#define CMD_BUFFER_SIZE		(CMD_MAX_CMD_NUM * (CMD_MAX_CMD_ELEM_SIZE + 2))


#define CMD_VTY_MAXUSER_NUM 32
#define CMD_VTY_CONSOLE_ID CMD_VTY_MAXUSER_NUM
#define CMD_VTY_CFM_ID CMD_VTY_MAXUSER_NUM + 1

#define CMD_VTY_INVALID_ID 0xFFFFFFFF


/* high 16bit ucMID,ucTBL, low 16bit usID */
#define CMD_ELEMID_DEF(ucMID, ucTBL, usID)  ( ( 0xFF000000 & (ucMID<<24)) + (0x00FF0000 & (ucTBL<<16)) + (0x0000FFFF & usID) )
#define CMD_ELEMID_NULL 0xFFFFFFFF

#define CMD_VECTOR_NEW(vec) vec = cmd_vector_new()
#define CMD_VECTOR_FREE(vec) cmd_vector_free(&vec);


typedef struct vty_user_st {
	int level;
	int type;  /* 0:console, 1:telnet */
	int state;  /* 0:idle, 1:access */
	int terminal_debugging;
	socket_t socket;
	char user_name[32];
	char user_psw[32];
	time_t lastAccessTime;	
}VTY_USER_S;

typedef struct cmd_vty_st {
	ULONG used;      /* vty 是否被占用 */
	ULONG vtyId;     /* vty id         */
	ULONG view_id;   /* 当前所在视图   */	
	VTY_USER_S user; /* 当前vty对应的用户信息   */	
	ULONG ulBufMaxLen;   /* 命令行输入的最大长度    */
	ULONG ulUsedLen;     /* 当前命令行输入的长度    */
	ULONG ulCurrentPos;  /* 当前命令行光标所在位置  */
	UCHAR c;   			 /* 当前输入的字符		    */
	/* BEGIN: for support TAB agian and agian */
	UCHAR ucKeyTypePre;	    					/* 上次输入类型   */
	UCHAR ucKeyTypeNow;							/* 本次输入类型   */
	CHAR tabbingString[CMD_MAX_CMD_ELEM_SIZE];	/* 最初始用来补全查找的字串*/
	CHAR tabString[CMD_MAX_CMD_ELEM_SIZE];		/* 最后一次补全的命令 */
	/* END: for support TAB agian and agian */
	CHAR res;
	CHAR szBuffer[CMD_BUFFER_SIZE];      /* 当前命令行缓冲区内容    */
	ULONG ulhpos;					     /* 当前查询的历史命令行下标 */
	ULONG ulhNum;	   					 /* 当前存在的历史命令计数   */
	CHAR *pszHistory[HISTORY_MAX_SIZE];  /* 当前存在的历史命令数组   */	
}CMD_VTY_S;

typedef struct tagCMD_VECTOR_S {
	ULONG ulSize;
	VOID **ppData;
} CMD_VECTOR_S;

#define CMD_MAX_CMD_ELEM_SIZE 64
#define CMD_MAX_CMD_ELEM_HELP_SIZE 256

typedef struct tagCMD_ELMTHELP_S {
	CHAR szElmtName[CMD_MAX_CMD_ELEM_SIZE];
	CHAR szElmtHelp[CMD_MAX_CMD_ELEM_HELP_SIZE];
} CMD_ELMTHELP_S;

typedef ULONG (*PFELMTHELPFUNC)(VOID *pRcvMsg, CMD_ELMTHELP_S **ppstCmdElmtHelp, ULONG *pulNum);  
typedef ULONG (*PFELMTCHECKFUNC)(VOID *pRcvMsg); 


/* 
cmd_vector_new函数功能: 创建命令行向量
用于注册命令行时使用
*/
extern CMD_VECTOR_S *cmd_vector_new();

/* 
cmd_vector_free函数功能:释放命令行向量
参数: CMD_VECTOR_S **ppVec - - 命令行向量指针

用于注册命令行时使用
*/
extern VOID cmd_vector_free(CMD_VECTOR_S **ppVec);

extern ULONG cmd_regelement(ULONG cmd_elem_id,
								CMD_ELEM_TYPE_E cmd_elem_type,
								CHAR *cmd_name,
								CHAR *cmd_help,
								PFELMTHELPFUNC pfElmtHelpFunc,
								PFELMTCHECKFUNC pfElmtCheckFunc,
								CMD_VECTOR_S * pVec);


/* 
cmd_regelement_new函数功能:注册命令元素
参数: ULONG cmd_elem_id - 命令元素id
      CMD_ELEM_TYPE_E cmd_elem_type -  命令元素类型
      CHAR *cmd_name -  命令元素名称
      CHAR *cmd_help - 命令元素帮助信息
      CMD_VECTOR_S *pVec - - 命令行向量

用于注册命令行时使用
*/
extern ULONG cmd_regelement_new(ULONG cmd_elem_id, CMD_ELEM_TYPE_E cmd_elem_type, CHAR *cmd_name, CHAR *cmd_help, CMD_VECTOR_S *pVec);

/* 
cmd_install_command函数功能:注册命令行
参数: ULONG mid - 模块id
      ULONG cmd_view - 命令行注册的视图id
	  CHAR *cmd_string - 需要注册的命令行线索表达式
	  CMD_VECTOR_S *pVec - 命令行向量
用于注册命令行时使用
*/
extern VOID cmd_install_command(ULONG mid, ULONG cmd_view, CHAR *cmd_string, CMD_VECTOR_S *pVec);

/* 
vty_view_set函数功能: 设置vty用户进入指定的视图
入参: ULONG vtyId - vty用户id
		 ULONG view_id - 视图id
返回值: ULONG - 处理结果，成功返回0，失败返回1
*/
extern VOID vty_view_set(ULONG vtyId, ULONG view_id);

/* 
vty_view_quit函数功能: 退出vty用户所在当前视图，并回到上一级视图
入参: ULONG vtyId - vty用户id
返回值: ULONG - 处理结果，成功返回0，失败返回1
*/
extern VOID vty_view_quit(ULONG vtyId);

/* 执行指定命令行 */
extern ULONG cmd_pub_run(CHAR *szCmdBuf);

/* 用于给指定用户打印信息 */
extern VOID vty_printf(ULONG vtyId, CHAR *format, ...);

/* 用于向所有用户打印信息 */
extern VOID vty_print2all(CHAR *format, ...);

/* 用于获取空闲的vtyid */
extern ULONG cmd_get_idle_vty();

/* 用于用户下线 */
extern VOID vty_offline(ULONG vtyId);	

/* 用于指定用户名用户下线 */
extern VOID vty_offline_by_username(CHAR *pszName);

/* 设置vty的socket */
extern ULONG vty_set_socket(ULONG vtyId, ULONG socket);

/* vty用户开始运行 */
extern VOID vty_go(ULONG vtyId);

/* 用户所在视图 */
extern ULONG vty_get_current_viewid(ULONG vtyId);

/* 
cmd_view_regist函数功能: 注册自定义视图
入参: CHAR *view_name - 视图名称
		CHAR *view_ais - 视图别名
		ULONG view_id - 视图id
		ULONG parent_view_id - 上一级视图id
返回值: ULONG - 处理结果，成功返回0，失败返回1
*/
extern ULONG cmd_view_regist(CHAR *view_name, CHAR *view_ais, ULONG view_id, ULONG parent_view_id);

/* 
cmd_get_vty_id函数功能: 获取vty用户id
入参: VOID *pRunMsg - 命令行回调的消息指针
返回值: ULONG - 为vty用户id
*/
extern ULONG cmd_get_vty_id(VOID *pRunMsg);

/* 
cmd_get_elem_by_index函数功能: 根据命令行参数索引获取命令元素
入参: VOID *pRunMsg - 命令行回调的消息指针
	  ULONG index - 命令行参数索引
返回值: VOID * - 为命令元素指针
*/
extern VOID *cmd_get_elem_by_index(VOID *pRunMsg, ULONG index);

/* 
cmd_get_elem_num函数功能: 获取命令元素的个数
入参: VOID *pRunMsg - 命令行回调的消息指针
返回值: ULONG - 为命令元素的个数
*/
extern ULONG cmd_get_elem_num(VOID *pRunMsg);

/* 
cmd_get_elemid函数功能: 获取命令元素id
入参: VOID *pElemMsg - 命令行元素指针
返回值: ULONG - 命令元素id
*/
extern ULONG cmd_get_elemid(VOID *pElemMsg);

/* 
cmd_get_ulong_param函数功能: 从命令元素中获取整形参数的值
入参: VOID *pElemMsg - 命令行元素指针
返回值: ULONG - 整形参数的值
注: 当该命令元素为整形参数时使用
*/
extern ULONG cmd_get_ulong_param(VOID *pElemMsg);

/* 
cmd_copy_string_param函数功能: 从命令元素中获取字符型参数的值
入参: VOID *pElemMsg - 命令行元素指针
返回值: CHAR *param  - 字符型参数的值
注: 当该命令元素为字符型参数时使用
*/
extern VOID cmd_copy_string_param(VOID *pElemMsg, CHAR *param);

/* 
cmd_get_ulong_param函数功能: 从命令元素中获取整形参数的值
入参: VOID *pElemMsg - 命令行元素指针
返回值: ULONG - 整形参数的ip值
注: 当该命令元素为整形参数ip时使用
*/
extern ULONG cmd_get_ip_ulong_param(VOID *pElemMsg);


/* 
cmd_get_first_elem_tblid函数功能: 获取命令行处理的tableid
入参: VOID *pRunMsg - 命令行回调的消息指针
返回值: ULONG - tableid
用于一个模块注册多个不同子模块命令时，区分回调处理
*/
extern ULONG cmd_get_first_elem_tblid(VOID *pRunMsg);

/*  */
/* 
cmd_regcallback函数功能: 命令行处理回调注册
入参: ULONG ulMid - 处理该回调的模块id(与注册命令字的tblid需要一致)
返回值: ULONG (*pfCallBackFunc)(VOID *pRcvMsg) - 需要注册的回调函数指针
每一个模块都要注册一个回调，以便处理命令行执行过程
*/
extern ULONG cmd_regcallback(ULONG ulMid, ULONG (*pfCallBackFunc)(VOID *pRcvMsg));


extern CMD_VTY_S *cmd_vty_getById(ULONG vtyId);
extern CMD_VTY_S g_vty[CMD_VTY_MAXUSER_NUM];
extern CMD_VTY_S *g_con_vty;

#endif

