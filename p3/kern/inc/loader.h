/* The 15-410 kernel project
 *
 *     loader.h
 *
 * Structure definitions, #defines, and function prototypes
 * for the user process loader.
 */

#ifndef _LOADER_H
#define _LOADER_H

#include <macros.h>


/* --- Prototypes --- */

int getbytes( const char *filename, int offset, int size, char *buf );
int load(char *filename, char *argv[], bool kernel_mode);

#endif /* _LOADER_H */
