/** @file kern_common.c
 *  @brief An inplementation for commonly used functions.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <kern_common.h>
#include <vm.h>
#include <stdlib.h>
#include <memlock.h>
#include <proc.h>

/**
 * @brief Check a null terminated user string array for validity.
 *
 * @param arr The array.
 * @param reqflags The required flags.
 * @param badflags The bad flags.
 * @return The number of elements in the array if valid,
 * a negative error code if invalid.
 */
int str_arr_check(char *arr[], unsigned reqflags, unsigned badflags)
{
    if ((unsigned)arr < USER_MEM_START) {
        return -1;
    }

    int len = 0;
    while (vm_check_flags_len(getpcb()->pd, arr + len, sizeof(char *),
        reqflags, badflags)) {
        if (arr[len] == NULL) {
            return len;
        }
        if (str_check(arr[len], reqflags, badflags) < 0) {
            return -2;
        }
        len++;
    }

    return -3;
}

/**
 * @brief Check a nil terminated user string for validity.
 *
 * @param str The string.
 * @param reqflags The required flags.
 * @param badflags The bad flags.
 * @return The length of the string if valid, negative error code if
 * invalid.
 */
int str_check(char *str, unsigned reqflags, unsigned badflags)
{
    if ((unsigned)str < USER_MEM_START) {
        return -1;
    }

    int len = 0;
    while (vm_check_flags(getpcb()->pd, str + len, reqflags, badflags)) {
        if (str[len] == '\0') {
            return len;
        }
        len++;
    }

    return -2;
}

/** @brief Checks a user nil terminated string for validity and locks the
 *  virtual memory region.
 *
 *  @param str The string.
 *  @return The length of the string if valid, negative error code if
 *  invalid.
 */
int str_lock(char *str) {

    if ((unsigned)str < USER_MEM_START) {
        return -1;
    }

    return vm_lock_str(str, USER_FLAGS_RO, 0, MEMLOCK_ACCESS);
}

/** @brief Checks a user buffer for validity and locks the
 *  virtual memory region.
 *
 *  @param len The length.
 *  @param buf The buffer.
 *  @return 0 on success, negative error code if invalid.
 */
int buf_lock(int len, char *buf)
{
    if ((unsigned)buf < USER_MEM_START) {
        return -1;
    }

    if (!vm_lock_len(buf, len, USER_FLAGS_RO, 0, MEMLOCK_ACCESS)) {
        return -2;
    }

    return 0;
}

/** @brief Checks a user buffer for validity and locks the
 *  virtual memory region for read-write access.
 *
 *  @param len The length.
 *  @param buf The buffer.
 *  @return 0 on success, negative error code if invalid.
 */
int buf_lock_rw(int len, char *buf)
{

    if ((unsigned)buf < USER_MEM_START) {
        return -1;
    }

    if (!vm_lock_len(buf, len, USER_FLAGS_RW, 0, MEMLOCK_ACCESS)) {
        return -2;
    }

    return 0;
}

/** @brief Checks a user integer pointer for validity and locks the
 *  virtual memory address.
 *
 *  @param n The interger pointer.
 *  @return 0 on success, negative error code if invalid.
 */
int int_lock(int *n)
{

    if ((unsigned)n < USER_MEM_START) {
        return -1;
    }

    if (!vm_lock_len(n, sizeof(int), USER_FLAGS_RO, 0, MEMLOCK_ACCESS)) {
        return -2;
    }

    return 0;
}

/** @brief Checks a user integer pointer for validity and locks the
 *  virtual memory address for read-write access.
 *
 *  @param n The integer pointer.
 *  @return 0 on success, negative error code if invalid.
 */
int int_lock_rw(int *n)
{

    if ((unsigned)n < USER_MEM_START) {
        return -1;
    }

    if (!vm_lock_len(n, sizeof(int), USER_FLAGS_RW, 0, MEMLOCK_ACCESS)) {
        return -2;
    }
    return 0;
}


/** @brief Unlcok a buffer memory lock.
 *
 *  @param len The length.
 *  @param buf The buffer.
 *  @return Void.
 */
void buf_unlock(int len, char *buf)
{

    vm_unlock_len(buf, len);
}

/** @brief Unlcok a integer pointer memory lock.
 *
 *  @param n The interger pointer.
 *  @return Void.
 */
void int_unlock(int *n)
{

    vm_unlock_len(n, sizeof(int));
}


/**
 * @brief Returns the control eflags given eflags.
 * Ignores user-flags.
 *
 * @param efl The raw eflags.
 * @return The control eflags.
 */
unsigned control_eflags(unsigned efl)
{
    return efl & (
      EFL_RESV1            /**< Reserved, Maintain 1 */
    | EFL_RESV2            /**< Reserved, Maintain 0 */
    | EFL_RESV3            /**< Reserved, Maintain 0 */
    | EFL_TF               /**< Trap Flag (single step) */
    | EFL_IF               /**< Interrupt Flag */
    | EFL_DF               /**< Direction Flag */
    | EFL_IOPL_RING3       /**< IO for all and sundry */
    | EFL_NT               /**< Nested Task */
    | EFL_RESV4            /**< Reserved, Maintain 0 */);

}
