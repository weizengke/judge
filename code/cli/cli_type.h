#ifndef _CLI_TYPE_H_
#define _CLI_TYPE_H_

// command match type
enum CMD_MATCH_STATUS {
	CMD_NO_MATCH,
	CMD_FULL_MATCH,
	CMD_PART_MATCH,
	CMD_LIST_MATCH,
	CMD_ERR_AMBIGOUS,
};

enum CMD_KEY_CODE_EM {
	CMD_KEY_CODE_NONE = -1,
	CMD_KEY_CODE_FILTER = 0,
	CMD_KEY_CODE_TAB,
	CMD_KEY_CODE_ENTER,
	CMD_KEY_CODE_QUEST,
	CMD_KEY_CODE_UP,
	CMD_KEY_CODE_DOWN,
	CMD_KEY_CODE_LEFT,
	CMD_KEY_CODE_RIGHT,
	CMD_KEY_CODE_DELETE,	 	
	CMD_KEY_CODE_BACKSPACE,
	CMD_KEY_CODE_DEL_LASTWORD,
	CMD_KEY_CODE_INSERT,

	CMD_KEY_CODE_MAX
};

typedef struct cmd_line_st {
	ULONG ulIndex;
	ULONG ulMid;			 	/* 模块id */
	ULONG ulViewId;				/* 视图id */
	ULONG ulElmtNum;			/* 命令行元素个数 */
	CMD_VECTOR_S *pstElmtVec;	/* 命令行元素向量 */
}CMD_LINE_S;

typedef struct cmd_elmt_st {
	CMD_ELEM_TYPE_E  eElmtType;
	ULONG ulElmtId;  		/* 命令字元素id */
	CHAR szElmtName[CMD_MAX_CMD_ELEM_SIZE];    /* 命令字元素名 */
	CHAR szElmtHelp[CMD_MAX_CMD_ELEM_HELP_SIZE];    /* 命令字帮助信息 */
	
	PFELMTHELPFUNC pfElmtHelpFunc;/* 自定义帮助回调 */
	PFELMTCHECKFUNC pfElmtCheckFunc;	/* 自定义检查回调 */
	
}CMD_ELMT_S;

typedef struct key_handle_st {
	ULONG ulKeyCode;
	VOID (*pKeyCallbackfunc)(CMD_VTY_S *);
} CMD_KEY_HANDLE_S;

#endif
