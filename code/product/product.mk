include $(PRODUCT_ROOT)/main/pdt.mk
include $(PRODUCT_ROOT)/judge/judge.mk
include $(PRODUCT_ROOT)/thirdpart32/thirdpart32.mk

PRODUCT_OBJ=$(PDT_INIT_OBJ) \
	$(JUDGE_OBJ)\
	$(THIRDPART_OBJ)