MAIN_OBJ=$(OBJ_DIR)/main.o \
	$(OBJ_DIR)/sysmng.o

all:$(MAIN_OBJ)

$(MAIN_OBJ):$(OBJ_DIR)/%.o:$(PRODUCT_ROOT)/main/%.cpp

	$(CC) $(CCFLAGS) $< $(CFLAGS) $@