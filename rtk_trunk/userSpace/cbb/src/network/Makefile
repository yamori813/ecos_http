#
# Leaf Makefile
#
include $(TENDA_APP_DIR)/.config

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(TENDA_APP_DIR)/tenda_rules.mk

EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/router_shared -I$(TENDA_APP_DIR)/net_drive/include -I$(TENDA_APP_DIR)/include

obj-y = interface.o
obj-y += network.o
obj-$(CONFIG_IPV6) += tpi_network6.o
obj-y += br_start.o

ALL_OBJS += $(obj-y)

#
# Leave rule to make
#

$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)


clean:
	find . -name '*.[od]' -type f -print0 | xargs -0 rm -f

.PHONY: clean
