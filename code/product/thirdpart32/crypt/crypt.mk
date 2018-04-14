CRYPT_OBJ=$(OBJ_DIR)/aes256.o \
          $(OBJ_DIR)/md5.o \
          $(OBJ_DIR)/sha256.o \
		  $(OBJ_DIR)/base64.o \
		  $(OBJ_DIR)/crypt.o

all:$(CRYPT_OBJ)

$(CRYPT_OBJ):$(OBJ_DIR)/%.o:$(PRODUCT_ROOT)/thirdpart32/crypt/%.cpp
	$(CC) $(CCFLAGS) $< $(CFLAGS) $@
