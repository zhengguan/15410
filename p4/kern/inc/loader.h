/* The 15-410 kernel project
 *
 *     loader.h
 *
 * Structure definitions, #defines, and function prototypes
 * for the user process loader.
 */

#ifndef _LOADER_H
#define _LOADER_H

#include <kern_common.h>


/* --- Prototypes --- */

int getbytes( const char *filename, int offset, int size, char *buf );
int load(char *filename, char *argv[], unsigned *eip, unsigned *esp);

#endif /* _LOADER_H */
