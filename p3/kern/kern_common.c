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
 * @brief Check a null terminated user array for validity.
 *
 * @param arr The array.
 * @return The number of elements in the array if valid,
 * a negative error code if invalid.
 */

int null_arr_check(char *arr[])
{
    int len = 0;
    while (vm_is_present(arr[len])) {
        if (arr[len] == NULL)
            return len;
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
    int len = 0;
    while (vm_is_present(&str[len])) {
        if (str[len] == '\0')
            return len;
        len++;
    }

    return -1;
}