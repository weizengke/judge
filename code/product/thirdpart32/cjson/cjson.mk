CJSON_OBJ=$(OBJ_DIR)/cJSON.o

all:$(CJSON_OBJ)

$(CJSON_OBJ):$(OBJ_DIR)/%.o:$(PRODUCT_ROOT)/thirdpart32/cjson/cJSON.cpp
	$(CC) $(CCFLAGS) $< $(CFLAGS) $@
