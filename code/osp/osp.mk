include $(OSP_ROOT)/command/command.mk
include $(OSP_ROOT)/debug/debug.mk
include $(OSP_ROOT)/event/event.mk
include $(OSP_ROOT)/regex/regex.mk

OSP_OBJ=$(CMD_OBJ) \
	$(DEBUG_CENTER_OBJ) \
	$(EVENT_OBJ) \
	REGEX_OBJ