#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
	
#ifdef _LINUX_
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#endif

#include "securec.h"
#include "util/util.h"
#include "cjson/cJSON.h"
#include "sysmng/config.h"
#include "ic/include/debug_center_inc.h"

#define CONFIG_FILE_MAX_SIZE 40960

cJSON *config_get_json_obj(){
    char jsonBuff[CONFIG_FILE_MAX_SIZE] = {0};
	(void)util_fread(CONFIG_JSON_PATH, jsonBuff, sizeof(jsonBuff) - 1);

    cJSON *json = cJSON_Parse(jsonBuff);
	if (json == NULL) {
		write_log(JUDGE_INFO, "config.json cJSON_Parse failed.");
		return NULL;
	}

    return json;
}

int config_json_get_string(char *item, char *key, char *default_return, char *buff_return, int buff_size) 
{
    strcpy_s(buff_return, buff_size, default_return);

    cJSON *json = config_get_json_obj();
    if (json == NULL) {        
        return 1;
    }

    cJSON *json_item_obj = cJSON_GetObjectItem(json, item);
    if(json_item_obj == NULL) {
        write_log(JUDGE_ERROR, "get config cJson item failed. item=%s, key=%s", item, key);
        cJSON_Delete(json);
        return 1;
    }

    cJSON *json_value = cJSON_GetObjectItem(json_item_obj, key);
    if (json_value == NULL) {
        write_log(JUDGE_ERROR, "get config cJson key value failed. item=%s, key=%s", item, key);
        cJSON_Delete(json);
        return 1;
    }

    if (json_value->type != cJSON_String) {
        write_log(JUDGE_ERROR, "get config cJson key value type is not string. item=%s, key=%s", item, key);
        cJSON_Delete(json);
        return 1;
    }

    strcpy_s(buff_return, buff_size, json_value->valuestring);
    
    write_log(JUDGE_INFO, "config_json_get_string item=%s, key=%s, return=%s", item, key, buff_return);

    cJSON_Delete(json);

    return 0;
}

int config_json_get_int(char *item, char *key, int value_default) 
{
    cJSON *json = config_get_json_obj();
    if (json == NULL) {        
        return value_default;
    }

    cJSON *json_item_obj = cJSON_GetObjectItem(json, item);
    if(json_item_obj == NULL) {
        write_log(JUDGE_ERROR, "get config cJson item failed. item=%s, key=%s", item, key);
        cJSON_Delete(json);
        return value_default;
    }

    cJSON *json_value = cJSON_GetObjectItem(json_item_obj, key);
    if (json_value == NULL) {
        write_log(JUDGE_ERROR, "get config cJson key value failed. item=%s, key=%s", item, key);
        cJSON_Delete(json);
        return value_default;
    }

    if (json_value->type != cJSON_Number) {
        write_log(JUDGE_ERROR, "get config cJson key value type is not number. item=%s, key=%s", item, key);
        cJSON_Delete(json);
        return value_default;
    }

    int valueint = json_value->valueint;
    
    write_log(JUDGE_INFO, "config_json_get_int item=%s, key=%s, value=%d", item, key, valueint);

    cJSON_Delete(json);

    return valueint;
}

int config_json_get_array_item_string(char *item, int id, char *key, char *default_return, char *buff_return, int buff_size) 
{
    strcpy_s(buff_return, buff_size, default_return);

    cJSON *json = config_get_json_obj();
    if (json == NULL) {        
        return 0;
    }

    cJSON *json_array_obj = cJSON_GetObjectItem(json, item);
    if(json_array_obj == NULL) {
        write_log(JUDGE_ERROR, "get config cJson item failed. item=%s, id=%d", item, id);
        cJSON_Delete(json);
        return 0;
    }

    if (json_array_obj->type != cJSON_Array) {
        write_log(JUDGE_ERROR, "get config cJson item is not array. item=%s, id=%d", item, id);
        cJSON_Delete(json);
        return 0;
    }

    int n = cJSON_GetArraySize(json_array_obj);
    for (int i = 0 ; i < n; i++) {
        cJSON *json_item = cJSON_GetArrayItem(json_array_obj, i);
        if (json_item == NULL) {
            write_log(JUDGE_ERROR, "get config cJson array item failed. item=%s, key=%s", item, key);
            continue;
        }

        cJSON *json_id_obj = cJSON_GetObjectItem(json_item, "id");
        if(json_id_obj == NULL || json_id_obj->valueint != id) {
            continue;
        }

        cJSON *json_value_obj = cJSON_GetObjectItem(json_item, key);
        if(json_value_obj == NULL || json_value_obj->type != cJSON_String) {
            write_log(JUDGE_ERROR, "get config cJson item value failed. item=%s, id=%d, key=%s", item, id, key);
            cJSON_Delete(json);
            return 1;
        }

        strcpy_s(buff_return, buff_size, json_value_obj->valuestring);
    }

    write_log(JUDGE_INFO, "config_json_get_array_item_string item=%s, id=%d, key=%s, return=%s", item, id, key, buff_return);
    
    cJSON_Delete(json);

    return 0;
}

int config_json_get_array_item_int(char *item, int id, char *key, int default_value) 
{
    cJSON *json = config_get_json_obj();
    if (json == NULL) {        
        return default_value;
    }

    cJSON *json_array_obj = cJSON_GetObjectItem(json, item);
    if(json_array_obj == NULL) {
        write_log(JUDGE_ERROR, "get config cJson item failed. item=%s, id=%d", item, id);
        cJSON_Delete(json);
        return default_value;
    }

    int valueint = default_value;
    if (json_array_obj->type != cJSON_Array) {
        write_log(JUDGE_ERROR, "get config cJson item is not array. item=%s, id=%d", item, id);
        cJSON_Delete(json);
        return default_value;
    }

    int n = cJSON_GetArraySize(json_array_obj);
    for (int i = 0 ; i < n; i++) {
        cJSON *json_item = cJSON_GetArrayItem(json_array_obj, i);
        if (json_item == NULL) {
            write_log(JUDGE_ERROR, "get config cJson array item failed. item=%s, key=%s", item, key);
            continue;
        }

        cJSON *json_id_obj = cJSON_GetObjectItem(json_item, "id");
        if(json_id_obj == NULL || json_id_obj->valueint != id) {
            continue;
        }

        cJSON *json_value_obj = cJSON_GetObjectItem(json_item, key);
        if(json_value_obj == NULL || json_value_obj->type != cJSON_Number) {
            write_log(JUDGE_ERROR, "get config cJson item value failed. item=%s, id=%d, key=%s", item, id, key);
            cJSON_Delete(json);
            return default_value;
        }

        valueint = json_value_obj->valueint;
    }

    write_log(JUDGE_INFO, "config_json_get_array_item_int item=%s, id=%d, key=%s, value=%d", item, id, key, valueint);
    
    cJSON_Delete(json);

    return valueint;
}
void config_json_test()
{   
    char buff[128] = {0};
    config_json_get_string("system", "startup_config", "cfg.cfg", buff, sizeof(buff));
    printf("startup_config=%s\r\n", buff);

    int port = config_json_get_int("system", "sock_port", 5001);
    printf("sock_port=%d\r\n", port);

    config_json_get_array_item_string("languages", 3, "language_name", "h++", buff, sizeof(buff));
    printf("language_name=%s\r\n", buff);

    int time_limit = config_json_get_array_item_int("languages", 3, "time_limit", 5001);
    printf("time_limit=%d\r\n", time_limit);
}