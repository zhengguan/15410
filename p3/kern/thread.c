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
#include <simics.h>

int next_pid = 1;
int next_tid = 1;

hashtable_t pcbs;
hashtable_t tcbs;

int cur_pid;
int cur_tid;

void thread_init() {
     hashtable_init(&pcbs, PCB_HT_SIZE);
     hashtable_init(&tcbs, TCB_HT_SIZE);
}

int new_process() {
    pcb_t *pcb = malloc(sizeof(pcb_t));
    if (pcb == NULL) {
        return -1;
    }

    pcb->pid = next_pid++;
    cur_pid = pcb->pid;
    // TODO do more stuff

    hashtable_add(&pcbs, pcb->pid, (void *)pcb);

    return 0;
}

int new_thread() {
    tcb_t *tcb = malloc(sizeof(tcb_t));
    if (tcb == NULL) {
        return -1;
    }

    tcb->tid = next_tid++;
    tcb->pid = cur_pid;

    // TODO do more stuff

    hashtable_add(&tcbs, tcb->tid, (void*)tcb);

    pcb_t *pcb;
    if (!hashtable_get(&pcbs, tcb->pid, (void**)&pcb)) {
        //FIXME do something
        lprintf("fucked up");
        MAGIC_BREAK;
    }

    linklist_add_tail(&pcb->threads, tcb);

    // TODO add to pcb linklist

    return 0;
}
