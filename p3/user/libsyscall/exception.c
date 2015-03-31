#include <ureg.h>
#include <syscall.h>
#include <simics.h>
#include <exn_handler_complete.h>

void exception_wrapper(swexn_handler_t handler, void *arg, ureg_t ureg)
{
    lprintf("in exception_wrapper, %p", handler);
    handler(arg, &ureg);
    exn_handler_complete();
    lprintf("shouldn't be here");
    MAGIC_BREAK;
}
