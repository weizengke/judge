FTP_OBJ=$(OBJ_DIR)/ftp_common.o \
        $(OBJ_DIR)/ftp_server.o \
        $(OBJ_DIR)/ftp_main.o

all:$(FTP_OBJ)

$(FTP_OBJ):$(OBJ_DIR)/%.o:$(OSP_ROOT)/ftp/%.cpp
	$(CC) $(CCFLAGS) $< $(CFLAGS) $@
