#ifndef _CLI_CORE_H_
#define _CLI_CORE_H_

extern ULONG cmd_ip_string_to_ulong(CHAR *ip);
extern VOID cmd_ip_ulong_to_string(ULONG ip, CHAR *buf);

extern VOID cmd_vector_insert(CMD_VECTOR_S *v, VOID *val);
extern VOID cmd_vector_insert_cr(CMD_VECTOR_S *v);

extern CMD_VECTOR_S *cmd_vector_copy(CMD_VECTOR_S *v);
extern CMD_VECTOR_S *cmd_str2vec(CHAR *string);
extern CMD_VECTOR_S *cmd_vector_init();
extern VOID cmd_vector_deinit(CMD_VECTOR_S *v, ULONG freeall);

extern ULONG cmd_match_command(CMD_VECTOR_S *icmd_vec, CMD_VTY_S *vty, CMD_ELMT_S **ppstMatchCmdElem, ULONG *pulMatchNum);
extern ULONG cmd_complete_command(CMD_VECTOR_S *icmd_vec, CMD_VTY_S *vty, CMD_ELMT_S **ppstCmdElem, ULONG *pulMatchNum, ULONG *pulNoMatchPos);
extern ULONG cmd_execute_command(CMD_VECTOR_S *icmd_vec, CMD_VTY_S *vty, CMD_ELMT_S **ppstCmdElem, ULONG *pulMatchNum, ULONG *pulNoMatchPos);

extern VOID cmd_output_missmatch(CMD_VTY_S *vty, ULONG ulNoMatchPos);

extern VOID cmd_insert_word(CMD_VTY_S *vty, CHAR *str);
extern VOID cmd_delete_word(CMD_VTY_S *vty);
extern VOID cmd_delete_word_ctrl_W_ex(CMD_VTY_S *vty);

extern VOID cmd_fsm(CMD_VTY_S *vty);
extern ULONG cmd_run(CMD_VTY_S *vty);
extern VOID cmd_outprompt(CMD_VTY_S *vty);

extern ULONG cmd_get_vty_id(VOID *pRunMsg);
extern VOID *cmd_get_elem_by_index(VOID *pRunMsg, ULONG index);
extern ULONG cmd_get_elem_num(VOID *pRunMsg);
extern ULONG cmd_get_elemid(VOID *pElemMsg);
extern CHAR* cmd_get_elem_param(VOID *pElemMsg);
extern ULONG cmd_get_ulong_param(VOID *pElemMsg);
extern VOID cmd_copy_string_param(VOID *pElemMsg, CHAR *param);	
extern ULONG cmd_get_first_elem_tblid(VOID *pRunMsg);

extern VOID cmd_handle_filter(CMD_VTY_S *vty);
extern VOID cmd_handle_tab(CMD_VTY_S *vty);
extern VOID cmd_handle_enter(CMD_VTY_S *vty);
extern VOID cmd_handle_quest(CMD_VTY_S *vty);
extern VOID cmd_handle_up(CMD_VTY_S *vty);
extern VOID cmd_handle_down(CMD_VTY_S *vty);
extern VOID cmd_handle_left(CMD_VTY_S *vty);
extern VOID cmd_handle_right(CMD_VTY_S *vty);
extern VOID cmd_handle_delete(CMD_VTY_S *vty);
extern VOID cmd_handle_backspace(CMD_VTY_S *vty);
extern VOID cmd_handle_insert(CMD_VTY_S *vty);
extern VOID cmd_hanlde_del_lastword(CMD_VTY_S *vty);

extern ULONG cmd_get_command_string(CMD_LINE_S *pstLine, CHAR *pszCmdString, int iSize);

#endif 
