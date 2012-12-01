LIBSPMP8K = ../..
LIBGAME = $(LIBSPMP8K)/libgame

MODULE := backends/platform/spmp8000

MODULE_OBJS := \
	spmp8000.o

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))

LIBS += -lm
