/** @file vm.h
 *  @brief Prototypes for managing virtual memory.
 *
 *  This contains prototypes for managing virtual memory.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#define KERNEL_MEM_START 0x00000000

#define PTE_PRESENT_NO 0x00
#define PTE_PRESENT_YES 0x01
#define PTE_RW_READ 0x00
#define PTE_RW_WRITE 0x02
#define PTE_SU_SUPER 0x00
#define PTE_SU_USER 0x04

#define PD_SIZE (PAGE_SIZE / sizeof(pde_t))
#define PT_SIZE (PAGE_SIZE / sizeof(pte_t))

#define PD_MASK 0xFFC00000
#define PD_SHIFT 20
#define GET_PD() ((pd_t)get_cr3())
#define GET_PD_IDX(ADDR) (((unsigned)ADDR & PD_MASK) >> PD_SHIFT)
#define GET_PDE(ADDR) (GET_PD()[GET_PD_IDX(ADDR)])

#define PT_MASK 0x003FF000
#define PT_SHIFT 10
#define GET_PT(PDE) ((pt_t)(PDE & BASE_ADDR_MASK))
#define GET_PT_IDX(ADDR) (((unsigned)ADDR & PT_MASK) >> PT_SHIFT)
#define GET_PTE(PDE,ADDR) (GET_PT(PDE)[GET_PT_IDX(ADDR)])

#define IS_PRESENT(PTE) (PTE & PTE_PRESENT_YES) 

#define BASE_ADDR_MASK ((unsigned)(~(PAGE_SIZE - 1)))

typedef unsigned pde_t;
typedef unsigned pte_t;
typedef pde_t* pd_t;
typedef pte_t* pt_t;

/* Virtual memory functions */
void vm_init();
unsigned vm_new_pd();
void vm_new_pde(pde_t *pde, pt_t pt);
void vm_new_pt(pde_t *pde);
void vm_new_pte(void *va, unsigned pa, unsigned flags);

/* System calls */
int new_pages(void *base, int len);
int remove_pages(void *base);
