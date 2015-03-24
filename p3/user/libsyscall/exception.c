#include <ureg.h>
#include <syscall.h>
#include <simics.h>

void exception_wrapper(swexn_handler_t handler, void *arg, ureg_t ureg)
{
    lprintf("in exception_wrapper, %p", handler);
    // ureg_t ureg_cpy = ureg; //so user doesn't break stuff without trying
    // handler(arg, &ureg_cpy);
    handler(arg, &ureg);
    MAGIC_BREAK;


}