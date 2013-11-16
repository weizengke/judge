include $(VRP_ROOT)command/command.mk
include $(VRP_ROOT)debug/debug.mk
include $(VRP_ROOT)regex/regex.mk

VRP_OBJ=$(CMD_OBJ) \
	$(DEBUG_CENTER_OBJ) \
	REGEX_OBJ