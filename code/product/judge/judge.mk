JUDGE_OBJ=$(OBJ_DIR)/judge.o \
	$(OBJ_DIR)/judge_util.o \
	$(OBJ_DIR)/judge_io.o \
	$(OBJ_DIR)/judge_hdu.o \
	$(OBJ_DIR)/judge_var.o \
	$(OBJ_DIR)/judge_sql.o\
	$(OBJ_DIR)/judge_guet.o\
	$(OBJ_DIR)/judge_cmd.o

all:$(JUDGE_OBJ)

$(JUDGE_OBJ):$(OBJ_DIR)/%.o:$(PRODUCT_ROOT)/judge/source/%.cpp

	$(CC) $(CCFLAGS) $< $(CFLAGS) $@