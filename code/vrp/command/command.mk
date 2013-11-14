CMD_OBJ=$(OBJ_DIR)cmd.o

all:$(CMD_OBJ)

$(CMD_OBJ):$(OBJ_DIR)%.o:$(VRP_ROOT)command/source/%.cpp
	$(CC) $(CCFLAGS) $< $(CFLAGS) $@ 
