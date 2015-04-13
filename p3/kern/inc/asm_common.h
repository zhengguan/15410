#ifndef __ASM_COMMON_H__
#define __ASM_COMMON_H__
#include <kern_common.h>

void jmp_ureg(ureg_t *ureg);
void jmp_ureg_user(ureg_t *ureg);
void jmp_user(unsigned eip, unsigned esp);
void iret_user();

void set_kernel_segs();
void set_user_segs();

void flush_tlb();
void flush_tlb_entry(void *va);

bool interrupts_enabled();

#endif /* __ASM_COMMON_H__ */