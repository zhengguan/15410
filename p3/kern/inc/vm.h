/** @file vm.h
 *  @brief Prototypes for managing virtual memory.
 *
 *  This contains prototypes for managing virtual memory.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include <x86/asm.h>

#define FIRST_PHY_FRAME 0x1000000

#define PTE_PRESENT_NO 0x00
#define PTE_PRESENT_YES 0x01
#define PTE_RW_READ 0x00
#define PTE_RW_WRITE 0x02
#define PTE_SU_SUPER 0x00
#define PTE_SU_USER 0x04

#define PD_SIZE (PAGE_SIZE / sizeof(pde_t))
#define PT_SIZE (PAGE_SIZE / sizeof(pte_t))

#define PT_MASK 0x003FF000
#define PT_SHIFT 10
#define GET_PT_IDX(BASE) (((unsigned)BASE & PT_MASK) >> PT_SHIFT)

#define PD_MASK 0xFFC00000
#define PD_SHIFT 20
#define GET_PD_IDX(BASE) (((unsigned)BASE & PD_MASK) >> PD_SHIFT)

#define BASE_ADDR_MASK (unsigned)(~(PAGE_SIZE - 1))

typedef unsigned pde_t;
typedef unsigned pte_t;
typedef pde_t* pd_t;
typedef pte_t* pt_t;

void vm_init();

void *vm_pd_init();

void *vm_pt_init();

static void *get_free_frame();

int new_pages(void *base, int len);

int remove_pages(void *base);
