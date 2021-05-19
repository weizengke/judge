#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef WIN32
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
#include "cli_util.h"
#include "cli_history.h"

#include "ic/include/debug_center_inc.h"

VOID cmd_vty_add_history(CMD_VTY_S *vty)
{
    ULONG prev_idx = vty->ulhNum ? vty->ulhNum - 1 : HISTORY_MAX_SIZE - 1;

    /* if same as previous command, then ignore */
    if (vty->pszHistory[prev_idx] &&
        !strcmp(vty->szBuffer, vty->pszHistory[prev_idx])) {
        /* fix: run command via history */
        vty->ulhpos = vty->ulhNum;
        return;
    }

    /* tail insert, firstly free if exist */
    if (vty->pszHistory[vty->ulhNum]) {
        free(vty->pszHistory[vty->ulhNum]);
        vty->pszHistory[vty->ulhNum] = NULL;
    }
    vty->pszHistory[vty->ulhNum] = cmd_util_strdup(vty->szBuffer);

    /* calc the next index */
    vty->ulhNum = ((vty->ulhNum + 1) == HISTORY_MAX_SIZE) ? 0 : vty->ulhNum + 1;
    vty->ulhpos = vty->ulhNum;
}

VOID cmd_vty_free_histories(CMD_VTY_S *vty)
{
    int i;

    for (i = 0; i < HISTORY_MAX_SIZE; i++){
        if (vty->pszHistory[i] != NULL){
            free(vty->pszHistory[i]);
            vty->pszHistory[i] = NULL;
        }
    }
}

CHAR *cmd_vty_get_history(CMD_VTY_S *vty, ULONG next)
{
    ULONG idx = 0;

    if (next) {
        idx = ((vty->ulhpos == (HISTORY_MAX_SIZE - 1)) ? 0 : vty->ulhpos + 1);
    } else {
        idx = ((vty->ulhpos == 0) ? (HISTORY_MAX_SIZE - 1) : vty->ulhpos - 1);
    }

    if (idx >= HISTORY_MAX_SIZE) {
        return NULL;
    }

    if (vty->pszHistory[idx] == NULL) {
        return NULL;
    }

    vty->ulhpos = idx;

    return vty->pszHistory[vty->ulhpos];
}

CHAR *cmd_vty_get_prev_history(CMD_VTY_S *vty)
{
    return cmd_vty_get_history(vty, 0);
}

CHAR *cmd_vty_get_next_history(CMD_VTY_S *vty)
{
    return cmd_vty_get_history(vty, 1);
}

VOID cmd_vty_show_history(CMD_VTY_S *vty, ULONG num)
{
    int i;

    if (NULL != vty) {
        for (i = 0; i < HISTORY_MAX_SIZE; i++) {
            if (vty->pszHistory[i] == NULL) {
                break;
            }
        }

        for (i = i - 1; i >= 0 && num > 0; i--,num--)
        {
            if (vty->pszHistory[i] == NULL) {
                break;
            }

            vty_printf(vty->vtyId, "%s \r\n", vty->pszHistory[i]);
        }
    }   
}