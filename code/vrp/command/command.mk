CMD_OBJ=$(OBJ_DIR)/command_var.o \
		$(OBJ_DIR)/command_core.o \
		$(OBJ_DIR)/command_func.o \
		$(OBJ_DIR)/command_adp.o \
		$(OBJ_DIR)/command_io.o

all:$(CMD_OBJ)

$(CMD_OBJ):$(OBJ_DIR)/%.o:$(VRP_ROOT)/command/source/%.cpp
	$(CC) $(CCFLAGS) $< $(CFLAGS) $@
