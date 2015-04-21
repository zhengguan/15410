410KLIB_STDIO_OBJS := \
						doprnt.o  \
						doscan.o  \
						hexdump.o \
						printf.o  \
						putchar.o \
						puts.o    \
						sprintf.o \
						sscanf.o  \


410KLIB_STDIO_OBJS := $(410KLIB_STDIO_OBJS:%=$(410KDIR)/stdio/%)

ALL_410KOBJS += $(410KLIB_STDIO_OBJS)
410KCLEANS += $(410KDIR)/libstdio.a

$(410KDIR)/libstdio.a: $(410KLIB_STDIO_OBJS)
