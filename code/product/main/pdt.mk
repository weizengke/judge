PDT_INIT_OBJ=$(OBJ_DIR)/pdt_init.o

all:$(PDT_INIT_OBJ)

$(PDT_INIT_OBJ):$(OBJ_DIR)/%.o:$(PRODUCT_ROOT)/main/%.cpp

	$(CC) $(CCFLAGS) $< $(CFLAGS) $@