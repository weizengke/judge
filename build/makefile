
MY_ROOT=$(shell pwd)
CODE_ROOT=$(MY_ROOT)/../code
OSP_ROOT=$(MY_ROOT)/../code/osp
PRODUCT_ROOT=$(MY_ROOT)/../code/product
OBJ_DIR=./obj
LIB_DIR=./lib
BIN_DIR=./bin
TARGET = vos.o

CC=g++
LD=ld
CCFLAGS = -w -c -m32 -D_LINUX_ -I$(CODE_ROOT)/ -I$(CODE_ROOT)/osp/util/
CFLAGS=-o 
LDFLAGS =
LIBS = -lcurl

include $(OSP_ROOT)/osp.mk
include $(PRODUCT_ROOT)/product.mk

all:app

OBJS = $(PRODUCT_OBJ) \
	$(OSP_OBJ)

app : $(OBJS)
	$(CC) $(LDFLAGS) -o $(BIN_DIR)/$(TARGET) -L$(LIB_DIR) $(OBJS) $(LIBS) -lpthread -lmysql -ldl lib/libcurl.a lib/libmysql.a
	
clean:
	rm -f $(OBJ_DIR)/*.*
	rm -f $(BIN_DIR)/*.*
