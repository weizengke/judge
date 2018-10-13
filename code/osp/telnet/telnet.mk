TELNET_OBJ=$(OBJ_DIR)/telnet_server.o \
           $(OBJ_DIR)/telnet_client.o \
           $(OBJ_DIR)/telnet_cmd.o

all:$(TELNET_OBJ)

$(TELNET_OBJ):$(OBJ_DIR)/%.o:$(OSP_ROOT)/telnet/%.cpp
	$(CC) $(CCFLAGS) $< $(CFLAGS) $@
