#ifndef _CONFIG_H_
#define _CONFIG_H_

#define CONFIG_JSON_PATH "conf\/config.json"

int config_json_get_string(char *item, char *key, char *default_return, char *buff_return, int buff_size);
int config_json_get_int(char *item, char *key, int value_default);
int config_json_get_array_item_string(char *item, int id, char *key, char *default_return, char *buff_return, int buff_size);
int config_json_get_array_item_int(char *item, int id, char *key, int default_value);

#endif