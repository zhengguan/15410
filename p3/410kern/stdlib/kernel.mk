410KLIB_STDLIB_OBJS := \
						abs.o     \
						atol.o    \
						ctype.o   \
						panic.o   \
                        qsort.o   \
                        rand.o    \
						strtol.o  \
						strtoul.o \


410KLIB_STDLIB_OBJS := $(410KLIB_STDLIB_OBJS:%=$(410KDIR)/stdlib/%)

ALL_410KOBJS += $(410KLIB_STDLIB_OBJS)
410KCLEANS += $(410KDIR)/libstdlib.a

$(410KDIR)/libstdlib.a: $(410KLIB_STDLIB_OBJS)
