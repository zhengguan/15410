410K_MISC_OBJS := \
					gccisms.o \

410K_MISC_OBJS := $(410K_MISC_OBJS:%=$(410KDIR)/misc/%)

ALL_410KOBJS += $(410K_MISC_OBJS)
410KCLEANS += $(410KDIR)/libmisc.a

$(410KDIR)/libmisc.a: $(410K_MISC_OBJS)
