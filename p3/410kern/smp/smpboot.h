/**
 * @file   smpboot.h
 * @brief  Defines related to the initial startup of APs
 * Needs to be included from an assembly file
 * 
 * @author Ryan Pearl (rpearl)
 * @author Michael Sullivan (mjsulliv)
 * @bug    None known
 */

#ifndef SMPBOOT_H
#define SMPBOOT_H

#define SMP_INIT_PAGE 0x2000

#define INIT_STACK_SHIFT 11
#define INIT_STACK_SIZE (1<<INIT_STACK_SHIFT)

#endif
