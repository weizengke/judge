
MY_ROOT=$(shell pwd)
CODE_ROOT=$(MY_ROOT)/../code
OSP_ROOT=$(MY_ROOT)/../code/osp
PRODUCT_ROOT=$(MY_ROOT)/../code/product
OBJ_DIR=./obj
BIN_DIR=./bin
TARGET = vos.o

CC=g++
LD=ld
CCFLAGS = -w -c -m32 -D_LINUX_ -I$(CODE_ROOT)/ -I$(CODE_ROOT)/osp/util/
CFLAGS=-o 
LDFLAGS =
LIBS =

include $(OSP_ROOT)/osp.mk
include $(PRODUCT_ROOT)/product.mk

all:app

OBJS = $(PRODUCT_OBJ) \
	$(OSP_OBJ)

app : $(OBJS)
	$(CC) $(LDFLAGS) -o $(BIN_DIR)/$(TARGET) $(OBJS) $(LIBS) -lpthread
	
clean:
	rm -f $(OBJ_DIR)/*.*
	rm -f $(BIN_DIR)/*.*
