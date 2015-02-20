/** @file new_kernel_thread.h
 *  @brief This file define the function prototype for thread_fork.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#ifndef _THREAD_FORK_H
#define _THREAD_FORK_H

/** @brief Creates a new kernel thread in the current task.
 *
 *  @param eip The new thread's instruction pointer.
 *  @param esp The new thread's stack pointer.
 *  @return The thread ID of the newly created thread.
 */
 int thread_fork(unsigned eip, unsigned esp);

#endif /* _THREAD_FORK_H */
