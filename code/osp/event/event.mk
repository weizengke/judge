EVENT_OBJ=$(OBJ_DIR)/event_pub.o

all:$(EVENT_OBJ)

$(EVENT_OBJ):$(OBJ_DIR)/%.o:$(OSP_ROOT)/event/source/%.cpp
	$(CC) $(CCFLAGS) $< $(CFLAGS) $@
