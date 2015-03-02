410K_ELF_OBJS := \
                 load_helper.o

410K_ELF_OBJS := $(410K_ELF_OBJS:%=$(410KDIR)/elf/%)

ALL_410KOBJS += $(410K_ELF_OBJS)
410KCLEANS += $(410KDIR)/libelf.a

$(410KDIR)/libelf.a: $(410K_ELF_OBJS)
