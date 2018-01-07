TELNET_OBJ=$(OBJ_DIR)/telnet_server.o

all:$(TELNET_OBJ)

$(TELNET_OBJ):$(OBJ_DIR)/%.o:$(OSP_ROOT)/telnet/%.cpp
	$(CC) $(CCFLAGS) $< $(CFLAGS) $@
