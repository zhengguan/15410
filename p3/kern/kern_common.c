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

/**
 * @brief Check a null terminated user string array for validity.
 *
 * @param arr The array.
 * @return The number of elements in the array if valid,
 * a negative error code if invalid.
 */

int str_arr_check(char *arr[])
{
    if ((unsigned)arr < USER_MEM_START)
        return -3;
    int len = 0;
    while (vm_is_present_len(arr + len, sizeof(char*))) {
        if (arr[len] == NULL)
            return len;
        if (str_check(arr[len]) < 0)
            return -2;
        len++;
    }

    return -1;
}


/**
 * @brief Check a nil terminated user string for validity.
 *
 * @param str The string.
 * @return The length of the string if valid, a negative error code if
 * invalid.
 */

int str_check(char *str)
{
    if ((unsigned)str < USER_MEM_START)
        return -2;
    int len = 0;
    while (vm_is_present(str + len)) {
        if (str[len] == '\0')
            return len;
        len++;
    }

    return -1;
}

int buf_check(char *buf)
{
    if ((unsigned)buf < USER_MEM_START) {
        return -1;
    }
    
    if (!vm_is_present(buf)) {
        return -2;
    }

    return 0;
}

int int_check(int *n)
{
    if (n == NULL)
        return 0;
    if ((unsigned)n < USER_MEM_START)
        return -1;
    if (!vm_is_present_len(n, sizeof(int)))
        return -2;
    return 0;
}