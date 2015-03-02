410K_SIMICS_OBJS := \
				simics.o   \
				simics_c.o 

410K_SIMICS_OBJS := $(410K_SIMICS_OBJS:%=$(410KDIR)/simics/%)

ALL_410KOBJS += $(410K_SIMICS_OBJS)
410KCLEANS += $(410KDIR)/libsimics.a

$(410KDIR)/libsimics.a: $(410K_SIMICS_OBJS)
