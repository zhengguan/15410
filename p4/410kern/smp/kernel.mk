410K_SMP_OBJS := \
      smp.o \
     apic.o \
  mptable.o \
smp_start.o

410K_SMP_OBJS := $(410K_SMP_OBJS:%=$(410KDIR)/smp/%)

ALL_410KOBJS += $(410K_SMP_OBJS)
410KCLEANS += $(410KDIR)/libsmp.a

$(410KDIR)/libsmp.a: $(410K_SMP_OBJS)
