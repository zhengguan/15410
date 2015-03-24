.globl exn_handler_complete

#include <syscall_int.h>
exn_handler_complete:
    int $SYSCALL_RESERVED_0
    ret