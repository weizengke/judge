include $(PRODUCT_ROOT)/main/pdt.mk
include $(PRODUCT_ROOT)/judge/judge.mk

PRODUCT_OBJ=$(PDT_INIT_OBJ) \
	$(JUDGE_OBJ)