/** @file console.c 
 *  @brief A console driver.
 *
 *  This is the implementation of the console driver.  
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <stdio.h>
#include <x86/asm.h>
#include <x86/video_defines.h>
#include <console.h>
#include <syscall.h>

static int term_color = DEFAULT_TERM_COLOR;
static int cursor_visible = DEFAULT_CURSOR_VISIBLE;
static int cursor_row = 0;
static int cursor_col = 0;

/** @brief Prints character ch at the current location
 *         of the cursor.
 *
 *  If the character is a newline ('\n'), the cursor is
 *  be moved to the beginning of the next line (scrolling if necessary).  If
 *  the character is a carriage return ('\r'), the cursor
 *  is immediately reset to the beginning of the current
 *  line, causing any future output to overwrite any existing
 *  output on the line.  If backsapce ('\b') is encountered,
 *  the previous character is erased.  See the main console.c description
 *  for more backspace behavior.
 *
 *  @param ch the character to print
 *  @return The input character
 */
int putbyte(char ch) {
    int row = 0;
    int col = 0;
    get_cursor_pos(&row, &col);
    
    int color = 0;
    get_term_color(&color);
    
    switch(ch) {
        case '\n':
            if (row >= CONSOLE_HEIGHT - 1) {
                scroll_up();
                set_cursor_pos(CONSOLE_HEIGHT - 1, 0);
            } else {
                set_cursor_pos(row + 1, 0);
            }
            break;
            
        case '\r':
            set_cursor_pos(row, 0);
            break;
            
        case '\b':
            if (col > 0) {
                draw_char(row, col - 1, ' ', color);
                set_cursor_pos(row, col - 1);
            }
            break;
            
        default:
            draw_char(row, col, ch, color);
            if (col >= CONSOLE_WIDTH - 1) {
                if (row >= CONSOLE_HEIGHT - 1) {
                    scroll_up();
                    set_cursor_pos(CONSOLE_HEIGHT - 1, 0);
                } else {
                    set_cursor_pos(row + 1, 0);
                }
            } else {
                set_cursor_pos(row, col + 1);
            }
    }
    
    return ch; 
}

/** @brief Prints the string s, starting at the current
 *         location of the cursor.
 *
 *  If the string is longer than the current line, the
 *  string fills up the current line and then
 *  continues on the next line. If the string exceeds
 *  available space on the entire console, the screen
 *  scrolls up one line, and then the string
 *  continues on the new line.  If '\n', '\r', and '\b' are
 *  encountered within the string, they are handled
 *  as per putbyte. If len is not a positive integer or s
 *  is null, the function has no effect.
 *
 *  @param s The string to be printed.
 *  @param len The length of the string s.
 *  @return Void.
 */
void putbytes(const char *s, int len) {
    if (s == NULL || len <= 0) {
        return;
    }
    
    int i;
    for(i=0; i < len; i++) {
        putbyte(s[i]);
    }
}

/** @brief Changes the foreground and background color
 *         of future characters printed on the console.
 *
 *  If the color code is invalid, the function has no effect.
 *
 *  @param color The new color code.
 *  @return 0 on success or integer error code less than 0 if
 *          color code is invalid.
 */
int set_term_color(int color) {
    if (color < 0x00 || color > 0xFF) {
        return -1;
    }

    term_color = color;
    
    return 0;
}

/** @brief Writes the current foreground and background
 *         color of characters printed on the console
 *         into the argument color.
 *  @param color The address to which the current color
 *         information will be written.
 *  @return Void.
 */
void get_term_color(int *color) {
    if (color == NULL) {
        return;
    }
    
    *color = term_color;
}

/** @brief Sets the position of the cursor to the
 *         position (row, col).
 *
 *  Subsequent calls to putbytes should cause the console
 *  output to begin at the new position. If the cursor is
 *  currently hidden, a call to set_cursor_pos() does not show
 *  the cursor.
 *
 *  @param row The new row for the cursor.
 *  @param col The new column for the cursor.
 *  @return 0 on success or integer error code less than 0 if
 *          cursor location is invalid.
 */
int set_cursor_pos(int row, int col) {
    if (row < 0 || row >= CONSOLE_HEIGHT ||
        col < 0 || col >= CONSOLE_WIDTH) {
        return -1;
    }

    cursor_row = row;
    cursor_col = col;
    
    if (cursor_visible) {
        int offset = row * CONSOLE_WIDTH + col;
        outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
        outb(CRTC_DATA_REG, (uint8_t)offset);
        outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
        outb(CRTC_DATA_REG, (uint8_t)(offset >> 8));
    }
    
    return 0;
}

/** @brief Writes the current position of the cursor
 *         into the arguments row and col.
 *  @param row The address to which the current cursor
 *         row will be written.
 *  @param col The address to which the current cursor
 *         column will be written.
 *  @return Void.
 */
int get_cursor_pos(int *row, int *col) {
    if (row == NULL) {
        return -1;
    }
    
    if (col != NULL) {
        return -2;
    }
    
    *row = cursor_row;
    *col = cursor_col;
    
    return 0;
}

/** @brief Hides the cursor.
 *
 *  Subsequent calls to putbytes do not cause the
 *  cursor to show again.
 *
 *  @return Void.
 */
void hide_cursor() {
    cursor_visible = 0;
    
    int offset = CONSOLE_HEIGHT * CONSOLE_WIDTH;
    outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
    outb(CRTC_DATA_REG, (uint8_t)offset);
    outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
    outb(CRTC_DATA_REG, (uint8_t)(offset >> 8));
}

/** @brief Shows the cursor.
 *  
 *  If the cursor is already shown, the function has no effect.
 *
 *  @return Void.
 */
void show_cursor() {
    cursor_visible = 1;
    set_cursor_pos(cursor_row, cursor_col);
}

/** @brief Clears the entire console.
 *
 * The cursor is reset to the first row and column
 *
 *  @return Void.
 */
void clear_console() {
    int color = 0;
    get_term_color(&color);
    
    int row;
    int col;
    for(row = 0; row < CONSOLE_HEIGHT; row++) {    
        for(col = 0; col < CONSOLE_WIDTH; col++) {
                draw_char(row, col, ' ', color);            
        }
    }
    
    set_cursor_pos(0, 0);
}

/** @brief Prints character ch with the specified color
 *         at position (row, col).
 *
 *  If any argument is invalid, the function has no effect.
 *
 *  @param row The row in which to display the character.
 *  @param col The column in which to display the character.
 *  @param ch The character to display.
 *  @param color The color to use to display the character.
 *  @return Void.
 */
void draw_char(int row, int col, int ch, int color) {
    *(char *)(CONSOLE_MEM_BASE + 2*(row*CONSOLE_WIDTH + col)) = ch; 
    *(char *)(CONSOLE_MEM_BASE + 2*(row*CONSOLE_WIDTH + col) + 1) = color; 
}

/** @brief Returns the character displayed at position (row, col).
 *  @param row Row of the character.
 *  @param col Column of the character.
 *  @return The character at (row, col).
 */
char get_char(int row, int col) {
  return *(char *)(CONSOLE_MEM_BASE + 2*(row*CONSOLE_WIDTH + col));
}

/** @brief Returns the color of the character at position (row, col).
 *
 *  @param row Row of the character.
 *  @param col Column of the character.
 *  @return The color at (row, col).
 */
int get_color(int row, int col) {
  return *(char *)(CONSOLE_MEM_BASE + 2*(row*CONSOLE_WIDTH + col) + 1);
}

/** @brief Scroll the console up by one line.
 *
 *  @return Void.
 */
void scroll_up() {
    int row;
    int col;
    for(row = 1; row < CONSOLE_HEIGHT; row++) {    
        for(col = 0; col < CONSOLE_WIDTH; col++) {
            char ch = get_char(row, col);
            int color = get_color(row, col);
            draw_char(row - 1, col - 1, ch, color);
        }
    }
    
    int color = 0;
    get_term_color(&color);
    
    for(col = 0; col < CONSOLE_WIDTH; col++) {
        draw_char(CONSOLE_HEIGHT - 1, col, ' ', color);        
    }
}

int print(int len, char *buf) 
{
    // TODO add saftey checks
    
    putbytes(buf, len);
    
    return 0;
}
