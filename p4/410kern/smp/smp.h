/**
 * @file   smp.h
 * @brief  smp related definitions and stuff
 *
 * @author Ryan Pearl (rpearl)
 * @author Michael Sullivan (mjsulliv)
 */

#ifndef SMP_H
#define SMP_H

#include <stdint.h>
#include <stddef.h>

#define MAX_CPUS 16

void smp_boot(void (*entry)(int));
int smp_get_cpu(void);

uint64_t tss_desc_create(void *tss, size_t tss_size);

#endif
