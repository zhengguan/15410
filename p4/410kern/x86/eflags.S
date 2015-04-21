/**@file x86/eflags.S */

    /** @brief Returns the EFLAGS register. */
.global get_eflags
get_eflags:
    /* Listen up.  Because X86 loves you, there isn't a move from
     * eflags to any general purpose register.  However, it can be
     * sent via the stack.  Thus we...
     */
    pushf          /* Push flags to stack */
    pop %eax       /* Pop flags to return */
    ret

.global set_eflags
    /** @brief Sets the EFLAGS register. */
set_eflags:
    /* The dual principle applies here as well: we have to push to
     * the stack so we can pop the flags back off.  This brings us to
     * everybody's favorite nonvirtualizable instruction, popf.
     * If you attempt to set EFLAGS:IF or other interesting bits without
     * permission, the processor ignores you, whereas it "should" trigger
     * a #GP fault or some other such behavior.
     */
    push 4(%esp)   /* Push flags to stack */
    popf           /* Pop flags to EFLAGS */
    ret

