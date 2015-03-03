/** @file thread.c
 *  @brief Handles threads.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <thread.h>
#include <stdlib.h>
#include <hashtable.h>

int next_pid = 1;
int next_tid = 1;

hashtable_t pcbs;
hashtable_t tcbs;

void thread_init() {
     hashtable_init(&pcbs, PCB_HT_SIZE);
     hashtable_init(&tcbs, TCB_HT_SIZE);
}

int new_task() {
    pcb_t *pcb = (pcb_t *)malloc(sizeof(tcb_t));
    if (pcb == NULL) {
        return -1;
    }
    
    pcb->pid = next_pid++;
    
    // TODO do more stuff
    
    hashtable_add(&pcbs, pcb->pid, pcb);
    
    return 0;
}

int new_thread() {
    tcb_t *tcb = (tcb_t *)malloc(sizeof(tcb_t));
    if (tcb == NULL) {
        return -1;
    }
    
    tcb->tid = next_tid++;
    
    // TODO do more stuff
    
    hashtable_add(&tcbs, tcb->tid, tcb);
    // TODO add to pcb linklist
    
    return 0;
}
