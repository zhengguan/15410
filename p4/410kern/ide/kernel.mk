410KLIB_IDE_OBJS:= \
                        pci.o     \
                        ide.o

410KLIB_IDE_OBJS:= $(410KLIB_IDE_OBJS:%=$(410KDIR)/ide/%)

ALL_410KOBJS += $(410KLIB_IDE_OBJS)
410KCLEANS += $(410KDIR)/libide.a

$(410KDIR)/libide.a: $(410KLIB_IDE_OBJS)
