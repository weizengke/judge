THIRDPART_OBJ=$(OBJ_DIR)/cJSON.o 

all:$(THIRDPART_OBJ)

$(THIRDPART_OBJ):$(OBJ_DIR)/%.o:$(PRODUCT_ROOT)/thirdpart32/cjson/%.cpp

	$(CC) $(CCFLAGS) $< $(CFLAGS) $@