DEBUG_CENTER_OBJ=$(OBJ_DIR)/debug_center.o

all:$(DEBUG_CENTER_OBJ)

$(DEBUG_CENTER_OBJ):$(OBJ_DIR)/%.o:$(OSP_ROOT)/debug/source/%.cpp
	$(CC) $(CCFLAGS) $< $(CFLAGS) $@
