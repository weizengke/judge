JUDGE_OBJ=$(OBJ_DIR)judge.o \
	$(OBJ_DIR)judge_util.o

all:$(JUDGE_OBJ)

$(JUDGE_OBJ):$(OBJ_DIR)%.o:$(PRODUCT_ROOT)judge/source/%.cpp

	$(CC) $(CCFLAGS) $< $(CFLAGS) $@