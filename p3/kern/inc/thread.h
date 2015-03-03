/** @file thread.h
 *  @brief Prototypes for managing kernel threads.
 *
 *  This contains prototypes for managing kernel threads.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
typedef struct pcb {
    int pid;
} pcb_t;

typedef struct tcb {
    int tid;
    int esp;
    pcb_t *process;
} tcb_t;
