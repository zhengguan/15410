/** @file 410kern/x86/pic.c
 *  @brief Provides initialization functions for the Programmable Interrupt
 *         Controllers.
 */

#define X86_PIC_DEFINITIONS
#include <x86/pic.h>
#include <x86/asm.h>

    /** @brief Bring up the PICs on this system.
     * 
     * @param master_base is the offset into the IDT that the first PIC
     *        uses for its IRQ0.  (So master_base through master_base + 7
     *        will be used.)
     *
     * @param slave_base is the offset into the IDT that the slave PIC
     *        uses for its IRQ0.
     *
     * @post This function will enable IRQs on the PICs; EFLAGS:IF still
     *       can be used to disable interrupts.  It is therefore suggested
     *       that this be called before IF is enabled the first time.
     *
     * @note This function assumes that there are only two PICs and that
     *       they are layed out as is conventional in IO space and that
     *       they are ganged together through master's IRQ2.  The last
     *       system that violated these assumptions went offline roughly
     *       forever ago.  If you don't like these, use the APICs which
     *       are better for you anyway.
     */
void
pic_init( unsigned char master_base, unsigned char slave_base )
{
    /* Bring the master IDT up; yes I know that the IO looks odd.
     * It's supposed to be, apparently.
     */
    outb( MASTER_ICW, PICM_ICW1 );
    outb( MASTER_OCW, master_base );
    outb( MASTER_OCW, PICM_ICW3 );
    outb( MASTER_OCW, PICM_ICW4 );

    /* Same dance with the slave as with the master */
    outb(  SLAVE_ICW, PICS_ICW1 );
    outb(  SLAVE_OCW, slave_base );
    outb(  SLAVE_OCW, PICS_ICW3 );
    outb(  SLAVE_OCW, PICS_ICW4 );

    /* Tell the master and slave that any IRQs they had outstanding
     * have been acknowledged.
     */
    outb( MASTER_ICW, NON_SPEC_EOI );
    outb(  SLAVE_ICW, NON_SPEC_EOI );

    /* Enable all IRQs on master and slave */
    outb (  SLAVE_OCW, 0 );
    outb ( MASTER_OCW, 0 );
}

    /** @brief Acknowledge the master or slave correctly, based on the
     *         IRQ number given.
     *
     * @param irq The IRQ, from the master PIC's perspective, that is to
     *        be acknowledged.  This means that 0-7 are master IRQs and
     *        8-15 are slave.
     */
void
pic_acknowledge(unsigned char irq)
{
    /* Note that SEND_EOI_IRn is just n, so we don't need any fancy map */
    if ( irq <= 7 ) {
        outb(MASTER_ICW, SPECIFIC_EOI | irq );
    } else if ( irq <= 15 ) {
        outb(SLAVE_ICW, SPECIFIC_EOI | (irq & 0x07) );
        outb(MASTER_ICW, SPECIFIC_EOI | PICS_ICW3 );
    } else {
        return;
    }
}

    /** @brief Acknowledge the master PIC for an arbitrary interrupt. */
void
pic_acknowledge_any_master(void)
{
    outb(MASTER_ICW, NON_SPEC_EOI );
}

    /** @brief Acknowledge the slave PIC for an arbitrary interrupt.
     *  @note Also acknowledges the master's input from the slave.
     */
void
pic_acknowledge_any_slave(void)
{
    outb(SLAVE_ICW, NON_SPEC_EOI );
    outb(MASTER_ICW, SPECIFIC_EOI | PICS_ICW3 );
}
