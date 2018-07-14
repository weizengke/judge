AAA_OBJ=$(OBJ_DIR)/aaa.o \
           $(OBJ_DIR)/aaa_cmd.o

all:$(AAA_OBJ)

$(AAA_OBJ):$(OBJ_DIR)/%.o:$(OSP_ROOT)/aaa/%.cpp
	$(CC) $(CCFLAGS) $< $(CFLAGS) $@
