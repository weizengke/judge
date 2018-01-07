NDP_OBJ=$(OBJ_DIR)/ndp.o

all:$(NDP_OBJ)

$(NDP_OBJ):$(OBJ_DIR)/%.o:$(OSP_ROOT)/ndp/source/%.cpp
	$(CC) $(CCFLAGS) $< $(CFLAGS) $@
