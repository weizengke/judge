include $(PRODUCT_ROOT)/main/main.mk
include $(PRODUCT_ROOT)/judge/judge.mk
include $(PRODUCT_ROOT)/thirdpart32/thirdpart32.mk

PRODUCT_OBJ=$(MAIN_OBJ) \
	$(JUDGE_OBJ)\
	$(THIRDPART_OBJ)