/** @file 410user/progs/shell.c
 *  @author ?
 *  @brief The shell.
 *  @public yes
 *  @for p2 p3
 *  @covers fork exec wait set_status vanish print ls
 *  @status done
 */

/* --- Since we're not calling thr_init() --- */
/* (this is sleazy violence, *not* good form) */
#define malloc _malloc
#define calloc _calloc
#define realloc _realloc
#define free _free

/* --- Includes --- */
#include <string.h>
#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>

#include <simics.h>

/* --- Defines --- */
#define MAX_LENGTH 1024


/* --- Initialized Global Strings --- */
char prompt[]     = "[410-shell]$ ";
char startmsg[]   = "Starting shell...\n";
char exitmsg[]    = "Exiting shell...\n";
char forkerrmsg[] = "Shell: Cannot fork process.\n";
char waitfailed[] = "wait() failed\n";
char finished[]   = "Process finished\n";
char too_long[]   = "That string is too long.\n";
char null_check[] = "I've lost null termination!\n";
char nothing[]    = "Readline returned nothing!\n";

/* --- These are global only to avoid stack growth --- */
/* (unclean, but lets the shell run before swexn() and new_pages()) */
char buf[MAX_LENGTH];
char *cmd_argv[MAX_LENGTH];
char separators[4] = { ' ', '\t', '\n' };

static void print_ls(void);
static void sort_names(char **names, int num_entries);
static int mystical_readline(int len, char *buf);
static int console_height(void);

/* Since P3 does not require GETCHAR, we define it here */
#ifndef HAVE_GETCHAR
#define getchar static_getchar
static char getchar(void)
{
  char buf[2];
  mystical_readline(2, buf);

  return buf[0];
}

#endif

/* --- Main --- */
int
main(int argc, char *argv[])
{
  int n;
  int pid;
  int res;
  int j;
  int ret;

  print(strlen(startmsg), startmsg);

  while(1) {

    bzero(buf, MAX_LENGTH);
    print(strlen(prompt), prompt);
    n = mystical_readline(MAX_LENGTH - 1, buf);
    if (n == 0) {
      /* Empty result; this shouldn't happen. */
      print(strlen(nothing), nothing);
      continue;
    } else if(n >= MAX_LENGTH) {
      /* line too big -- kernel declared that it clobbered memory */
      print(strlen(too_long), too_long);
      exit(-1);
    } else if(buf[MAX_LENGTH - 1] != '\0') {
      /* Overflow -- kernel tapdanced on random memory! */
      print(strlen(null_check), null_check);
      exit(-1);
    }

    buf[n] = '\0';
    j = 0;
    if(!(cmd_argv[j++] = strtok(buf, separators))) {
      continue;
    }
    if((strcmp(cmd_argv[0], "exit") == 0)) {
      print(strlen(exitmsg), exitmsg);
      exit(0);
    }
    if((strcmp(cmd_argv[0], "ls") == 0)) {
      print_ls();
      continue;
    }

    while((cmd_argv[j++] = strtok(NULL, separators)));

    pid = fork();
    if(pid < 0) {
      print(sizeof(forkerrmsg), forkerrmsg);
      continue;
    }
    if(pid == 0) {
      exec(cmd_argv[0], cmd_argv);
      exit(-1);
    }
    else {
      if((ret = wait(&res)) < 0) {
        printf("\nshell: wait on process %d failed!\n", pid);
      }
      else {
        printf("\nshell: process %d finished with exit status %d\n", ret, res);
      }
    }
  } /* while loop */
}

/* emulate the old ls() system call using readfile() */
int ls(int size, char *buf)
{
  int i;
  int num_files = 0;
  int amt_read;

  amt_read = readfile(".", buf, size, 0);
  if (amt_read < 0) return amt_read;
  /* ls() returned failure if the buffer wasn't big enough for the
   * entire listing. So if readfile() didn't return short (that is,
   * there might be more), fail. */
  if (amt_read == size) return -1;

  /* Now scan through the buffer counting null terminators.
   * Skip the one at the end. */
  for (i = 0; i < amt_read - 1; i++) {
    if (buf[i] == '\0') {
      num_files++;
    }
  }

  return num_files;
}

static void print_ls()
{
  char *buf;
  char *bufptr;
  char ** names;
  int index = 0;
  int size = 4096;
  int num_entries;
  int len;

  /* get the array of entries */
  buf = (char *)malloc(sizeof(char) * size);
  printf("Listing the user tests\n");
  while ((num_entries = ls(size, buf)) < 0) {
    size *= 2;
    buf = (char *)realloc(buf, sizeof(char) * size);
  }

  names = (char **)calloc(num_entries, sizeof(char *));

  /* get pointers to each string in the array */
  bufptr = buf;
  while ((len = strlen(bufptr)) > 0) {
    names[index++] = bufptr;
    bufptr += len + 1;
  }

  sort_names(names, num_entries);

  for (index = 0; index < num_entries; index++) {
    if (index != 0 && index % (console_height() - 2) == 0) {
      printf("Hit a key to continue\n");
      getchar();
    }
    printf("%s\n", names[index]);
  }

  free(buf);
  free(names);
  buf = NULL;
  names = NULL;
}

static void sort_names(char **names, int num_entries) {
    /* I invented this cool new sorting algorithm! It's really fast! */
    int swapped;
    int i;
    do {
        swapped = 0;
        for (i = 0; i < num_entries - 1; i++) {
            if (strcmp(names[i], names[i+1]) > 0) {
                char *tmp = names[i];
                names[i] = names[i+1];
                names[i+1] = tmp;
                swapped = 1;
            }
        }
    } while (swapped);
}

static int mystical_readline(int len, char *buf) {
    if (len <= 0) { return 0; }

    buf[0] = '\0';
    /* If we're in the test harness, this does something very special! */
    sim_call(0x04108005, buf, len);

    return (buf[0] ? strlen(buf) : readline(len, buf));
}

static int console_height() {
  static int console_height = -1;

  if (console_height == -1) {
    /* determine the height of the console */
    int curstartrow, curstartcol;
    get_cursor_pos(&curstartrow, &curstartcol);
    console_height = 0;
    while (set_cursor_pos(console_height, 0) == 0) {
      console_height++;
    }
    set_cursor_pos(curstartrow, curstartcol);

  }

  return console_height;
}
