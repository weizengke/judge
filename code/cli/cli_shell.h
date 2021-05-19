#ifndef __CLI_SHELL_H__
#define __CLI_SHELL_H__

typedef struct cmd_ntf_node_st
{
	ULONG  ulMid;
	ULONG  (*pfCallBackFunc)(VOID *pRcvMsg); /* memory free by caller */

	struct cmd_ntf_node_st *pNext;
}CMD_NTF_NODE_S;

typedef struct tagCMD_RUNMSG_ELEM_S
{
	ULONG ulElmtId;
	CHAR aszElmtArray[128];
}CMD_RUNMSG_ELEM_S;

typedef struct tagCMD_RUNMSG_S
{
	ULONG vtyId;
	ULONG argc;
	CMD_RUNMSG_ELEM_S *argv;
}CMD_RUNMSG_S;

ULONG cmd_run_notify(CMD_LINE_S *pstCmdLine, CMD_VTY_S *vty, CHAR **argv, ULONG argc);

#endif