410KLIB_STRING_OBJS:= \
                        memcmp.o     \
                        memset.o     \
                        rindex.o     \
                        strcat.o     \
                        strchr.o     \
                        strcmp.o     \
                        strcpy.o     \
                        strcspn.o    \
                        strdup.o     \
                        strlen.o     \
                        strncat.o    \
                        strncmp.o    \
                        strncpy.o    \
                        strpbrk.o    \
                        strrchr.o    \
                        strspn.o     \
                        strstr.o     \
                        strtok.o     \


410KLIB_STRING_OBJS:= $(410KLIB_STRING_OBJS:%=$(410KDIR)/string/%)

ALL_410KOBJS += $(410KLIB_STRING_OBJS)
410KCLEANS += $(410KDIR)/libstring.a

$(410KDIR)/libstring.a: $(410KLIB_STRING_OBJS)
