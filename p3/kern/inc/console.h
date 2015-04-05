/** @file console.h
 *  @brief Function prototypes for the console driver.
 *
 *  This contains the prototypes and global variables for the console
 *  driver
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <video_defines.h>

#define DEFAULT_TERM_COLOR (FGND_WHITE | BGND_BLACK)
#define DEFAULT_CURSOR_VISIBLE 1

/* Console functions */
int putbyte( char ch );
void putbytes(const char* s, int len);
void get_term_color(int* color);
void hide_cursor();
void show_cursor();
void clear_console();
void draw_char(int row, int col, int ch, int color);
char get_char(int row, int col);
int get_color(int row, int col);
void scroll_up();

#endif /* _CONSOLE_H */
