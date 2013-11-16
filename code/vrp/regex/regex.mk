REGEX_OBJ=$(OBJ_DIR)jungle_regex.o

all:$(REGEX_OBJ)

$(REGEX_OBJ):$(OBJ_DIR)%.o:$(VRP_ROOT)regex/%.cpp
	$(CC) $(CCFLAGS) $< $(CFLAGS) $@

