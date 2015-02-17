/** @file new_kernel_thread.h
 *  @brief This file define the function prototype for new_kernel_thread.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
/** @brief Creates a new kernel thread.
 *
 *  @param eip The new thread's instruction pointer.
 *  @param esp The new thread's stack pointer.
 *  @return The thread ID of the newly created thread.
 */
 int new_kernel_thread(unsigned eip, unsigned esp);
