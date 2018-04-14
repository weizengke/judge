include $(PRODUCT_ROOT)/thirdpart32/cjson/cjson.mk
include $(PRODUCT_ROOT)/thirdpart32/crypt/crypt.mk

THIRDPART_OBJ=$(CJSON_OBJ)\
	$(CRYPT_OBJ)