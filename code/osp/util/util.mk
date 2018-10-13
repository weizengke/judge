UTIL_OBJ=$(OBJ_DIR)/util.o \
         $(OBJ_DIR)/win32.o \
         $(OBJ_DIR)/linux.o

all:$(UTIL_OBJ)

$(UTIL_OBJ):$(OBJ_DIR)/%.o:$(OSP_ROOT)/util/%.cpp
	$(CC) $(CCFLAGS) $< $(CFLAGS) $@
