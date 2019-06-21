STACKWALKER_OBJ=$(OBJ_DIR)/stackwalker.o

all:$(STACKWALKER_OBJ)

$(STACKWALKER_OBJ):$(OBJ_DIR)/%.o:$(PRODUCT_ROOT)/thirdpart32/stack_walker/stackwalker.cpp
	$(CC) $(CCFLAGS) $< $(CFLAGS) $@
