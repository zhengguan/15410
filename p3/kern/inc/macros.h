#ifndef __MACROS_H__
#define __MACROS_H__
#include <syscall.h>

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#define ROUND_DOWN_TO_PAGE(x) ((x)&(~(PAGE_SIZE-1)))
#define ROUND_UP_TO_PAGE(x) (ROUND_DOWN_TO_PAGE((x)+PAGE_SIZE-1))

#endif /* __MACROS_H__ */