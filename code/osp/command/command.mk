CMD_OBJ=$(OBJ_DIR)/command_main.o \
        $(OBJ_DIR)/command_bdn.o \
        $(OBJ_DIR)/command_core.o \
        

all:$(CMD_OBJ)

$(CMD_OBJ):$(OBJ_DIR)/%.o:$(OSP_ROOT)/command/source/%.cpp
	$(CC) $(CCFLAGS) $< $(CFLAGS) $@
