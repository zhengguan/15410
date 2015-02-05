/**
  * @file misbehave_wrap.c
  * @author Nathaniel Wesley Filardo <nwf@andrew.cmu.edu>
  * @brief Executes a given command with all misbehave values and compares
  *        return values to expected values.
  *
  * Takes the expected return value and the command line for a given test.
  * It will cycle the test through each of the several misbehave modes.
  * 
  * Licensed under Either of BSD 3-clause / GPLv2
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syscall.h>
#include <stdlib.h>
#include <stddef.h>
#include <simics.h>

#define printf( x ... ) \
  do { set_term_color(0x1F); printf(x); set_term_color(0x03); } while(0)

/**
  * Maximum number of misbehave modes, excluding -1.
  * @bug Why isn't this defined as part of the system call ABI ?
  */
#define MISBEHAVE_MAX   64

int main(int argc, char **argv)
{
  int expectrv = 0, i;

  if ( argc < 3 )
  {
    printf("Usage: %s <expect rv> <program> <prog_args>\n", argv[0]);
    return -1;
  }

  printf(">>> misbehave_wrap argument vector: \n");
  for ( i = 0; i < argc; i++ )
  {
    printf("     '%s'\n", argv[i]);
  }

  /* Extract expected return value */
  expectrv = atoi( argv[1] );
  
  /* 
   * Exec didn't seem to appreciate being passed our argv, so let's just
   * copy one onto the stack.
   */
  char **nargv = _malloc(sizeof(char*) * (argc - 1));
  for ( i = 0; i < argc - 2; i++ )
  {
    nargv[i] = _malloc(strlen(argv[i+2])+1);
    strcpy(nargv[i], argv[i+2]);
  }
  nargv[argc - 2] = NULL;
  
  printf(">>> Misbehave wrap executing : '%s'\n", nargv[0]);
  printf(">>> With argument vector: \n");
  for ( i = 0; nargv[i] != NULL; i++ )
  {
    printf("     '%s'\n", nargv[i]);
  }

  for ( i = 0; i < MISBEHAVE_MAX + 1; i++ )
  {
    printf(">>> Misbehave wrap iteration %d\n", i);
    if ( i == MISBEHAVE_MAX )
    {
      misbehave(-1);
    }
    else
    {
      misbehave(i);
    }

    int pid = fork();

    if ( pid < 0 )
    {
      printf("FORK failure (try a spoon?): %d\n", pid);
      return -1;
    }
    else if ( pid == 0 )
    {
      int ev;
      if ( ( ev = exec (nargv[0], nargv) ) < 0 )
      {
        printf("EXEC failure (NX Feature?): %d\n", ev);
        return ~expectrv;
      }
    }
    else
    {
      int status, wpid;
      if ( (wpid = wait ( &status ) ) < 0 )
      {
        printf("WAIT failure (go instead?): %d\n", wpid);
        return -1;
      }
      if ( pid != wpid )
      {
        printf("WAIT returned wrong pid (kernel confused?): %d != %d\n",
          pid, wpid);
        return -1;
      }
      if ( status != expectrv )
      {
        printf("Victim program returned unexpected value: %d\n", status);
        return -2;
      }
      else
      {
        printf(">>> Success\n");
      }
    }
  }

  return 0;
}
