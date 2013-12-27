include $(PRODUCT_ROOT)/main/pdt.mk
include $(PRODUCT_ROOT)/judge/judge.mk
#include $(PRODUCT_ROOT)/captcha/captcha.mk

PRODUCT_OBJ=$(PDT_INIT_OBJ) \
	$(JUDGE_OBJ)
#\
#	$(CAPTCHA_OBJ)