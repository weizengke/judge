REGEX_OBJ=$(OBJ_DIR)/jungle_regex.o

all:$(REGEX_OBJ)

$(REGEX_OBJ):$(OBJ_DIR)/%.o:$(OSP_ROOT)/regex/%.cpp
	$(CC) $(CCFLAGS) $< $(CFLAGS) $@

