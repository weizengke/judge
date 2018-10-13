PCRE_OBJ=$(OBJ_DIR)/pcre_xclass.o \
	$(OBJ_DIR)/pcre_version.o \
	$(OBJ_DIR)/pcre_valid_utf8.o \
	$(OBJ_DIR)/pcre_ucd.o \
	$(OBJ_DIR)/pcre_try_flipped.o \
	$(OBJ_DIR)/pcre_tables.o \
	$(OBJ_DIR)/pcre_study.o \
	$(OBJ_DIR)/pcre_refcount.o \
	$(OBJ_DIR)/pcre_ord2utf8.o \
	$(OBJ_DIR)/pcre_newline.o \
	$(OBJ_DIR)/pcre_maketables.o \
	$(OBJ_DIR)/pcre_info.o \
	$(OBJ_DIR)/pcre_globals.o \
	$(OBJ_DIR)/pcre_get.o \
	$(OBJ_DIR)/pcre_fullinfo.o \
	$(OBJ_DIR)/pcre_exec.o \
	$(OBJ_DIR)/pcre_dfa_exec.o \
	$(OBJ_DIR)/pcre_config.o \
	$(OBJ_DIR)/pcre_compile.o \
	$(OBJ_DIR)/pcre_chartables.o \
	
all:$(PCRE_OBJ)

$(PCRE_OBJ):$(OBJ_DIR)/%.o:$(PRODUCT_ROOT)/thirdpart32/pcre/%.c
	$(CC) $(CCFLAGS) -DHAVE_CONFIG_H -DPCRE_STATIC $< $(CFLAGS) $@
