#ifndef _X86_MISC_H_
#define _X86_MISC_H_

/* FIXME! This file is full of things that need better homes. */

    /** @brief #PF thrown from user code */
#define PF_ERRCODE_USER     0x4
    /** @brief #PF thrown due to write error */
#define PF_ERRCODE_WRITE    0x2
    /** @brief #PF thrown due to protection error */
#define PF_ERRCODE_PROT     0x1

#endif
