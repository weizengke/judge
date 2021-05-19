
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef WIN32

#if CYGWIN
#include <ncurses.h>
#else
#include <conio.h>
#endif

#include <io.h>
#include <winsock2.h>
#endif

#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#ifdef _LINUX_
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <termios.h>
#include <assert.h>
#endif

#include "icli.h"
#include "cli_def.h"
#include "cli_type.h"
#include "cli_core.h"
#include "cli_view.h"
#include "cli_history.h"
#include "cli_reg.h"

ULONG cmd_regelement_new(ULONG cmd_elem_id,
						 CMD_ELEM_TYPE_E cmd_elem_type,
						 CHAR *cmd_name,
						 CHAR *cmd_help,
						 CMD_VECTOR_S *pVec)
{
	CMD_ELMT_S *pstCmdElem = NULL;

	if (CMD_MAX_CMD_ELEM_SIZE <= strlen(cmd_name))
	{
		CMD_DBGASSERT(0, "cmd_regelement_new");
		return CMD_ERR;
	}

	pstCmdElem = (CMD_ELMT_S *)malloc(sizeof(CMD_ELMT_S));
	if (NULL == pstCmdElem)
	{
		return CMD_ERR;
	}
	memset(pstCmdElem, 0, sizeof(CMD_ELMT_S));

	pstCmdElem->ulElmtId = cmd_elem_id;
	pstCmdElem->eElmtType = cmd_elem_type;

	strcpy(pstCmdElem->szElmtName, cmd_name);
	strcpy(pstCmdElem->szElmtHelp, cmd_help);

	cmd_vector_insert(pVec, pstCmdElem);

	return CMD_OK;
}
