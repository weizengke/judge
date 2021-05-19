#ifndef __CLI_UTIL_H__
#define __CLI_UTIL_H__

BOOL cmd_string_isdigit(CHAR *string);
ULONG cmd_ip_string_to_ulong(CHAR *ip);
VOID cmd_ip_ulong_to_string(ULONG ip, CHAR *buf);
BOOL cmd_string_is_ip(CHAR *str);
CHAR *cmd_util_strdup(const char *s);

#endif