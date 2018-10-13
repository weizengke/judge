include $(OSP_ROOT)/command/command.mk
include $(OSP_ROOT)/debug/debug.mk
include $(OSP_ROOT)/event/event.mk
include $(OSP_ROOT)/ndp/ndp.mk
include $(OSP_ROOT)/aaa/aaa.mk
include $(OSP_ROOT)/telnet/telnet.mk
include $(OSP_ROOT)/ftp/ftp.mk
include $(OSP_ROOT)/util/util.mk

OSP_OBJ=$(CMD_OBJ) \
	$(DEBUG_CENTER_OBJ) \
	$(EVENT_OBJ) \
    $(NDP_OBJ)\
    $(AAA_OBJ)\
    $(TELNET_OBJ)\
    $(FTP_OBJ)\
	$(UTIL_OBJ)