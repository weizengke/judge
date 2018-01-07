include $(OSP_ROOT)/command/command.mk
include $(OSP_ROOT)/debug/debug.mk
include $(OSP_ROOT)/event/event.mk
include $(OSP_ROOT)/ndp/ndp.mk
include $(OSP_ROOT)/telnet/telnet.mk

OSP_OBJ=$(CMD_OBJ) \
	$(DEBUG_CENTER_OBJ) \
	$(EVENT_OBJ) \
        $(NDP_OBJ)\
        $(TELNET_OBJ)