/** @file console.c 
 *  @brief A console driver.
 *
 *  This is the implementation of the console driver.  
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include <p1kern.h>
#include <stdio.h>
#include <x86/asm.h>
#include <x86/video_defines.h>
#include <console.h>

static int term_color = DEFAULT_TERM_COLOR;

static int cursor_visible = DEFAULT_CURSOR_VISIBLE;
static int cursor_row = 0;
static int cursor_col = 0;

int putbyte(char ch) {
    int row = 0;
    int col = 0;
    get_cursor(&row, &col);
    
    int color = 0;
    get_term_color(&color);
    
    switch(ch) {
        case '\n':
            if (row >= CONSOLE_HEIGHT - 1) {
                scroll_up();
                set_cursor(CONSOLE_HEIGHT - 1, 0);
            } else {
                set_cursor(row + 1, 0);
            }
            break;
            
        case '\r':
            set_cursor(row, 0);
            break;
            
        case '\b':
            if (col > 0) {
                draw_char(row, col - 1, ' ', color);
                set_cursor(row, col - 1);
            }
            break;
            
        default:
            draw_char(row, col, ch, color);
            if (col >= CONSOLE_WIDTH - 1) {
                if (row >= CONSOLE_HEIGHT - 1) {
                    scroll_up();
                    set_cursor(CONSOLE_HEIGHT - 1, 0);
                } else {
                    set_cursor(row + 1, 0);
                }
            } else {
                set_cursor(row, col + 1);
            }
    }
    
    return ch; 
}

void putbytes(const char *s, int len) {
    if (s == NULL || len <= 0) {
        return;
    }
    
    int i;
    for(i=0; i < len; i++) {
        putbyte(s[i]);
    }
}

int set_term_color(int color) {
    if (color < 0x00 || color > 0xFF) {
        return -1;
    }

    term_color = color;
    
    return 0;
}

void get_term_color(int *color) {
    if (color == NULL) {
        return;
    }
    
    *color = term_color;
}

int set_cursor(int row, int col) {
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

void get_cursor(int *row, int *col) {
    if (row != NULL) {
        *row = cursor_row;
    }
    
    if (col != NULL) {
        *col = cursor_col;
    }
}

void hide_cursor() {
    cursor_visible = 0;
    
    int offset = CONSOLE_HEIGHT * CONSOLE_WIDTH;
    outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
    outb(CRTC_DATA_REG, (uint8_t)offset);
    outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
    outb(CRTC_DATA_REG, (uint8_t)(offset >> 8));
}

void show_cursor() {
    cursor_visible = 1;
    set_cursor(cursor_row, cursor_col);
}

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
    
    set_cursor(0, 0);
}

void draw_char(int row, int col, int ch, int color) {
    *(char *)(CONSOLE_MEM_BASE + 2*(row*CONSOLE_WIDTH + col)) = ch; 
    *(char *)(CONSOLE_MEM_BASE + 2*(row*CONSOLE_WIDTH + col) + 1) = color; 
}

char get_char(int row, int col) {
  return *(char *)(CONSOLE_MEM_BASE + 2*(row*CONSOLE_WIDTH + col));
}

int get_color(int row, int col) {
  return *(char *)(CONSOLE_MEM_BASE + 2*(row*CONSOLE_WIDTH + col) + 1);
}

static void scroll_up() {
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
