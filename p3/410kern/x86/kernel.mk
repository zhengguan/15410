410K_X86_OBJS := \
					asm.o \
					bcopy.o \
					bzero.o \
					eflags.o \
					interrupts.o \
					keyhelp.o \
					pic.o \
					rtc.o

410K_X86_OBJS := $(410K_X86_OBJS:%=$(410KDIR)/x86/%)

ALL_410KOBJS += $(410K_X86_OBJS)
410KCLEANS += $(410KDIR)/libx86.a

$(410KDIR)/libx86.a: $(410K_X86_OBJS)
