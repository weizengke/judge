/*
Author: Jungle Wei
Date  : 2013-10
Description: A mini common command line system

Note:
	_LINUX_ for compile on Linux , #define _LINUX_  or g++ -D_LINUX_ cmd.cpp

*/

#include "..\include\command_inc.h"



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
int cmd_vector_fetch(cmd_vector_t *v)
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
	string_cr = (char*)malloc(sizeof(CMD_END));
	if (NULL == string_cr)
	{
		CMD_DBGASSERT(0);
		return;
	}

	memcpy(string_cr, CMD_END, sizeof(CMD_END));
	cmd_vector_insert(v, string_cr); /* cmd_vector_insert(v, CMD_END); // bug of memory free(<CR>), it's static memory*/
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


int cmd_get_nth_elem_pos(char *string, int n, int *pos)
{
	int str_len;
	int m = 0;
	char *cur, *start;

	*pos = 0;

	// empty string
	if (string == NULL)
		return CMD_ERR;

	cur = string;
	// skip white spaces
	while (isspace((int) *cur) && *cur != '\0')
		cur++;
	// only white spaces
	if (*cur == '\0')
		return CMD_ERR;
	// not care ! and #
	if (*cur == '!' || *cur == '#')
		return CMD_ERR;

	while (1)
	{
		start = cur;
		while (!(isspace((int) *cur) || *cur == '\r' || *cur == '\n') &&
			*cur != '\0')
			cur++;

		str_len = cur - start;

		if (n == m)
		{
			*pos = (int)(start - string);
			return CMD_OK;
		}
		else
		{
			m++;
		}

		while((isspace ((int) *cur) || *cur == '\n' || *cur == '\r') &&
			*cur != '\0')
			cur++;

		if (*cur == '\0')
			return CMD_ERR;

	}
}

/*****************************************************************************
*   Prototype    : cmd_output_missmatch
*   Description  : Output miss match command position
*   Input        : cmd_vty *vty
*                  int nomath_pos
*   Output       : None
*   Return Value : void
*   Calls        :
*   Called By    :
*
*   History:
*
*       1.  Date         : 2013/11/19
*           Author       : weizengke
*           Modification : Created function
*
*****************************************************************************/
void cmd_output_missmatch(cmd_vty *vty, int nomath_pos)
{
	int i = 0;
	int n = 0;
	char buf[CMD_BUFFER_SIZE] = {0};

	/* 输出箭头位置 3 is < - >*/
	//int pos_arrow = 3 + strlen(g_sysname) + strlen(vty->prompt);
	int pos_arrow = 2 + strlen(g_sysname);

	strcpy(buf, vty->buffer);

	(void)cmd_get_nth_elem_pos(vty->buffer, nomath_pos, &n);
	pos_arrow += n;

	for (i=0;i<pos_arrow;i++)
	{
		cmd_outstring(" ");
	}

	cmd_outstring("^\r\n");

	cmd_outstring("Error: Unrecognized command at '^' position.\r\n");

}


// turn a command into vector
cmd_vector_t *cmd2vec(char *string, char *doc)
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
	if (0 != cmd_get_elem_by_name((char*)CMD_END, desc_cr))
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
	int match_cmd = CMD_ERR;
	struct cmd_elem_st *elem;
	struct para_desc *desc;

	// For each command, check the 'index'th parameter if it matches the
	// 'index' paramter in command, set command vector which does not
	// match cmd NULL

	/* BEGIN: Added by weizengke, 2013/10/4   PN:check cmd valid*/
	if (cmd == NULL || 0 == strlen(cmd))
	{
		debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd_filter_command, the param cmd is null.");
		return CMD_ERR;
	}

	/* <CR> 不参与过滤，防止命令行子串也属于命令行时误过滤 */
	if (0 == strcmp(cmd, CMD_END))
	{
		debug_print_ex(CMD_DEBUG_TYPE_ERROR, "In cmd_filter_command, the param cmd is <CR>.");
		return CMD_OK;
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
					continue;
				}
			}

			/* BEGIN: Added by weizengke, 2013/11/19 for support unkown cmd pos*/
			match_cmd = CMD_OK;
			/* END:   Added by weizengke, 2013/11/19 */
		}
	}

	return match_cmd;
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
int cmd_complete_command(cmd_vector_t *icmd_vec, struct cmd_vty *vty,
									 struct para_desc **match, int *match_size, int *match_pos)
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

	*match_pos = -1;

	// Step 1
	/* BEGIN: Modified by weizengke, 2013/10/4   PN:循环过滤每一个向量 */
	for (i = 0; i < cmd_vector_max(icmd_vec); i++)
	{
		if (CMD_OK != cmd_filter_command((char*)cmd_vector_slot(icmd_vec, i), cmd_vec_copy, i))
		{
			/* BEGIN: Added by weizengke, 2013/11/19 这里可以优化，不命中可以不需要再匹配了 */
			/* 保存在第几个命令字无法匹配 */
			*match_pos = (*match_pos == -1)?(i):(*match_pos);
		}
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



			/* BEGIN: Added by weizengke, 2013/11/19 */
			/* match STRING , INTEGER */
			if (CMD_OK == cmd_match_special_string(str, para_desc_->para))
			{
				debug_print_ex(CMD_DEBUG_TYPE_INFO, "In cmd_complete_command, match(%s).", para_desc_->para);
				match[match_num++] = para_desc_;

				continue;
			}
			/* END:   Added by weizengke, 2013/11/19 */

			/* match key */
			if (strncmp(str, CMD_END, strlen(str)) == 0
			    ||(strncmp(str, para_desc_->para, strlen(str)) == 0))
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
				if (0 == strncmp(match[i]->para, CMD_END, strlen(match[i]->para))
					|| ( 1 == strcmp(match[i]->para, match[j]->para)
					&& 0 != strncmp(match[j]->para, CMD_END, strlen(match[j]->para)))
					)
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

int cmd_execute_command(cmd_vector_t *icmd_vec, struct cmd_vty *vty, struct para_desc **match, int *match_size, int *match_pos)
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

	*match_pos = -1;

	/* Step 1 */
	for (i = 0; i < cmd_vector_max(icmd_vec); i++)
	{
		if (CMD_OK != cmd_filter_command((char*)cmd_vector_slot(icmd_vec, i), cmd_vec_copy, i))
		{
			/* BEGIN: Added by weizengke, 2013/11/19 这里可以优化，不命中可以不需要再匹配了 */
			/* 保存在第几个命令字无法匹配 */
			*match_pos = (*match_pos == -1)?(i):(*match_pos);
		}
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



// insert a word into the tail of input buffer
void cmd_insert_word(struct cmd_vty *vty, const char *str)
{
	strcat(vty->buffer, str);
	vty->cur_pos += (int)strlen(str);
	vty->used_len += (int)strlen(str);
}

// delete the last word from input buffer
void cmd_delete_word(struct cmd_vty *vty)
{
	int pos = strlen(vty->buffer);

	while (pos > 0 && vty->buffer[pos - 1] != ' ') {
		pos--;
	}

	vty->buffer[pos] = '\0';
	vty->cur_pos = strlen(vty->buffer);
	vty->used_len = strlen(vty->buffer);
}

// delete the last word from input buffer
void cmd_delete_word_ctrl_W(struct cmd_vty *vty)
{
	/* 删除最后一个elem */

	int pos = strlen(vty->buffer);

	if (pos == 0)
	{
		return;
	}

	/* ignore suffix-space */
	while (vty->buffer[pos - 1] == ' ')
	{
		pos--;
	}

	/* del the last word */
	while (pos > 0 && vty->buffer[pos - 1] != ' ') {
		pos--;
	}

	vty->buffer[pos] = '\0';
	vty->cur_pos = strlen(vty->buffer);
	vty->used_len = strlen(vty->buffer);
}

// delete the last word from input buffer
void cmd_delete_word_ctrl_W_ex(struct cmd_vty *vty)
{
	/* 删除光标所在当前或之前elem */
	int start_pos = 0;
	int end_pos  = 0;
	int len = strlen(vty->buffer);
	int pos = vty->cur_pos;

	if (pos == 0)
	{
		return;
	}

	/* ignore suffix-space */
	if (vty->buffer[pos] == ' ')
	{
		end_pos = pos;

		pos--;
		/* 往回找第一个非空字符 */
		while (pos  >= 0 && vty->buffer[pos] == ' ')
		{
			pos--;
		}

		if (pos == 0)
		{
			return;
		}

		/* 继续往回找第一个空格或命令头 */
		while (pos  >= 0 && vty->buffer[pos] != ' ')
		{
			pos--;
		}

		start_pos = pos + 1;

	}
	else
	{
		/* 分别往左右找空格 */
		pos++;
		while (vty->buffer[pos] != ' ')
		{
			pos++;
		}

		end_pos = pos - 1;

		pos = vty->cur_pos;
		while (vty->buffer[pos] != ' ')
		{
			pos--;
		}

		start_pos = pos + 1;
	}

	//printf ("\r\ns_p = %d, e_p = %d\r\n", start_pos, end_pos);

	/* back the cur_pos */
	int i = 0;
	int size = 0;

	size = vty->cur_pos - start_pos;
	for (i = 0; i < size; i++)
		cmd_back_one();

	/* output the right chars */
	for (i = 0; i < len - start_pos; i ++)
		cmd_put_one(vty->buffer[end_pos + i]);

	size = len - start_pos;

	for (i = 0; i < size; i++)
		cmd_back_one();

	memcpy(&vty->buffer[start_pos], &vty->buffer[end_pos], strlen(&vty->buffer[end_pos]));
	vty->buffer[len - (vty->cur_pos - start_pos)]= '\0';
	vty->cur_pos -= (vty->cur_pos - start_pos);
	vty->used_len -= (vty->cur_pos - start_pos);

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



void cmd_read(struct cmd_vty *vty)
{
	int key_type;

	g_InputMachine_prev = CMD_KEY_CODE_NOTCARE;
	g_InputMachine_now = CMD_KEY_CODE_NOTCARE;

	while ((vty->c = cmd_getch()) != EOF) {

		//printf("c=%d\r\n",vty->c);

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

void cmd_outcurrent()
{
	if (NULL == vty)
	{
		return ;
	}

	cmd_outstring("%s", CMD_ENTER);
	cmd_outprompt(vty->prompt);
	cmd_outstring("%s", vty->buffer);
}

int cmd_init()
{
	/* initial cmd vector */
	cmd_vec = cmd_vector_init(1);

	cmd_install();

	vty = cmd_vty_init();
	if (vty == NULL)
	{
		return OS_ERR;
	}

	return OS_OK;
}


void cmd_main_entry(void *pEntry)
{
	/*
		cmd_outprompt(vty->prompt);
	*/
	for (;;)
	{
		cmd_read(vty);
	}

	cmd_vty_deinit(vty);

	return ;
}


APP_INFO_S g_CMDAppInfo =
{
	NULL,
	"Command",
	cmd_init,
	cmd_main_entry
};

void Cmd_RegAppInfo()
{
	RegistAppInfo(&g_CMDAppInfo);
}

