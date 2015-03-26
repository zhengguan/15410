/** @file exec_run.h
 *  @brief This file defines function protoypes for running a user program.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#ifndef _EXEC_RUN_H
#define _EXEC_RUN_H

/** @brief Runs a user program.
 *
 *  @param ds The data segment register.
 *  @param es The extra segment register.
 *  @param eip The interuction pointer register.
 *  @param cs The code segment register.
 *  @param eflags The EFLAGS register.
 *  @param esp The stack pointer register.
 *  @param ss The stack segment register.
 *  @return Does not return.
 */
void exec_run(unsigned ds, unsigned es, unsigned eip, unsigned cs, unsigned eflags, unsigned esp, unsigned ss);

#endif /* _EXEC_RUN_H */
