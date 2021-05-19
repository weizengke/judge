#ifndef _CLI_VIEW_H_
#define _CLI_VIEW_H_

#include "icli.h"

extern VOID vty_view_set(ULONG vtyId, ULONG view_id);
extern VOID vty_view_quit(ULONG vtyId);
extern ULONG cmd_view_get_alias_lenth(ULONG view_id);
extern CHAR* cmd_view_get_alias(ULONG view_id);
extern CHAR* cmd_view_get_name(ULONG view_id);
extern ULONG cmd_view_get_parent_view_id(ULONG view_id);
extern VOID cmd_view_init();

#endif