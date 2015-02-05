/*
 * Test program for the Spring 2004 15-410 Kernel project
 * Clone of the nibbles/snake game
 *
 * by Michael Ashley-Rollman (mpa)
 * tweaked by Dave Eckhardt (de0u) for Fall 2011 P4
 */

#include <stdlib.h>
#include <stdio.h>
#include <syscall.h>
#include <syscall_int.h>
#include <rand.h>
#include <assert.h>

#define SLEEP_TICKS 75

#define SNAKE_COLOR 0x60
#define SNAKE_CHAR " "

#define APPLE_COLOR 0x4
#define APPLE_CHAR "@"

#define BORDER_COLOR 0x30
#define BORDER_CHAR ' '

#define BLANK_COLOR 0x0
#define TEXT_COLOR 0x2

/* Maximum possible console width */
#define MAX_POS_WIDTH 128

/* Maximum snake length */
#define SNAKE_MAX_LENGTH 100

/* Minimum snake length */
#define SNAKE_MIN_LENGTH 10

#define SNAKE_UP 'w'
#define SNAKE_DOWN 's'
#define SNAKE_LEFT 'a'
#define SNAKE_RIGHT 'd'

#define SNAKE_START_ROW 10
#define SNAKE_START_COL 10
#define SNAKE_START_DIR SNAKE_RIGHT

#define APPLE_MIN_WAIT 20
#define APPLE_MAX_WAIT 40

#define APPLE_MAX_NUM 10
#define APPLE_SCORE 10

/* The snake */
typedef struct {
  int length;
  int start;
  int spot[SNAKE_MAX_LENGTH][2];
  char direction;
} snake_t;

/* The Apples */
typedef struct {
  int x;
  int y;
  int valid;
} apple_t;

/* inter-move delay
 * (adjustable for simulation vs. hardware)
 * (could be adjusted downward as play progresses)
 */
int sleep_ticks = SLEEP_TICKS;

/* width of the console */
int con_width;

/* height of the console */
int con_height;

/* the current score */
int score;

/* the last key pressed */
volatile int last_key;

int spawn_key_grabber(void);

/* prints out the play instructions */
void print_directions()
{
  printf("Move the snake using the controls listed below.\n");
  printf("Eat the apples to get points, but don't hit the\n");
  printf("wall or let the snake bite himself.\n");
  printf("UP - w\n");
  printf("DOWN - s\n");
  printf("LEFT - a\n");
  printf("RIGHT - d\n");
  printf("\n\n");
};

/* draws the border around the edge of the screen and blanks the middle */
void draw_border()
{
  int i;
  char buf[MAX_POS_WIDTH];

  for (i = 0; i < con_width; i++) {
    buf[i] = BORDER_CHAR;
  }

  set_term_color(BLANK_COLOR);
  set_cursor_pos(con_height-1,0);
  print(con_width, buf);

  set_term_color(BORDER_COLOR);
  set_cursor_pos(0,0);
  print(con_width, buf); 

  for (i = 1; i < con_height - 2; i++) {
    set_term_color(BLANK_COLOR);
    set_cursor_pos(i,0);
    print(con_width, buf);

    set_term_color(BORDER_COLOR);
    set_cursor_pos(i, 0);
    print(1, buf);

    set_cursor_pos(i, con_width-1);
    print(1, buf);
  }

  set_cursor_pos(con_height - 2,0);
  print(con_width, buf); 

  score = 0;
  set_cursor_pos(con_height - 1, 0);
  set_term_color(TEXT_COLOR);
  printf("score: %d", score);
}

/* initializes a snake to the starting size and location */
void init_snake(snake_t *snake) {
  int i;

  snake->length = SNAKE_MIN_LENGTH;
  snake->start = SNAKE_MIN_LENGTH - 1;

  for (i = 0; i < SNAKE_MIN_LENGTH; i++) {
    snake->spot[i][0] = SNAKE_START_COL;
    snake->spot[i][1] = SNAKE_START_ROW;
  }

  snake->direction = SNAKE_START_DIR;
}

/* checks to see if the snake has killed itself
 *
 * returns 0 if the snake is still alive and -1 on death
 */
int snake_disaster(snake_t *snake)
{
  int i, stop;
  int snake_x = snake->spot[snake->start][0];
  int snake_y = snake->spot[snake->start][1];

  /* check to see if the snake is within bounds */
  if (snake_x <= 0 ||
      snake_x >= con_width - 1 ||
      snake_y <= 0 ||
      snake_y >= con_height - 2)
    return -1;

  /* check to see if the snake is biting itself */
  stop = (snake->start - snake->length + SNAKE_MAX_LENGTH) % SNAKE_MAX_LENGTH;

  for (i = (snake->start - 1 + SNAKE_MAX_LENGTH) % SNAKE_MAX_LENGTH;
       i != stop;
       i = (i-1 + SNAKE_MAX_LENGTH) % SNAKE_MAX_LENGTH)
    if (snake->spot[i][0] == snake_x &&
	snake->spot[i][1] == snake_y)
      return -1;

  return 0;
}

/* moves the snake forward a space, eating any apples there */
void move_snake(snake_t *snake, apple_t *apples)
{
  int old_x, old_y;
  int new_x, new_y;
  int remove_x, remove_y;

  int i;
  
  /* get the position of the head of the snake */
  old_x = snake->spot[snake->start][0];
  old_y = snake->spot[snake->start][1];

  snake->start = (snake->start + 1) % SNAKE_MAX_LENGTH;

  /* get the position of the tail of the snake */
  remove_x = snake->spot[(snake->start -
			  snake->length +
			  SNAKE_MAX_LENGTH) %
			 SNAKE_MAX_LENGTH][0];
  remove_y = snake->spot[(snake->start -
			  snake->length +
			  SNAKE_MAX_LENGTH) %
			 SNAKE_MAX_LENGTH][1];

  /* find where the head of the snake has slithered to */
  switch (snake->direction) {
  case SNAKE_UP:
    new_x = old_x;
    new_y = old_y - 1;
    break;

  case SNAKE_DOWN:
    new_x = old_x;
    new_y = old_y + 1;
    break;

  case SNAKE_LEFT:
    new_x = old_x - 1;
    new_y = old_y;
    break;

  case SNAKE_RIGHT:
    new_x = old_x + 1;
    new_y = old_y;
    break;

  default:
    panic("Impossible direction %d", snake->direction);
    return;
  }

  /* fill int he new head */
  snake->spot[snake->start][0] = new_x;
  snake->spot[snake->start][1] = new_y;
  
  /* delete the old tail */
  if (!set_cursor_pos(remove_y, remove_x)) {
    set_term_color(BLANK_COLOR);
    print(1, " ");
  }

  /* display the head */
  if (!set_cursor_pos(new_y, new_x)) {
    set_term_color(SNAKE_COLOR);
    print(1, SNAKE_CHAR);
  }

  /* ate any apples that the snake has hit */
  for (i = 0; i < APPLE_MAX_NUM; i++)
    if (apples[i].valid == 1 &&
	apples[i].x == new_x &&
	apples[i].y == new_y) {
      score += APPLE_SCORE;
      apples[i].valid = 0;
      snake->length++;
      if (snake->length > SNAKE_MAX_LENGTH)
	snake->length = SNAKE_MAX_LENGTH;
    }      
}

/* randomly place a new apple on the screen */
void new_apple(snake_t *snake, apple_t *apple)
{
  int spot;
  int x,y;

  /* find a spot for an apple in the array */
  for (spot = 0; spot < APPLE_MAX_NUM; spot++)
    if (!apple[spot].valid) break;

  if (spot == APPLE_MAX_NUM) return;

  /* pick a place on the screen */
  x = genrand() / ((unsigned int)(-1) / (con_width - 2)) + 1;
  y = genrand() / ((unsigned int)(-1) / (con_height - 3)) + 1;

  /* set the apple */
  apple[spot].valid = 1;
  apple[spot].x = x;
  apple[spot].y = y;

  /* draw the apple */
  set_term_color(APPLE_COLOR);
  set_cursor_pos(y, x);
  print(1, APPLE_CHAR);
}

/* plays through the game once */
void play_game(snake_t *snake)
{
  int next_apple = 0;
  int i;

  /* initialize the apples */
  apple_t apples[APPLE_MAX_NUM];

  for (i = 0; i < APPLE_MAX_NUM; i++)
    apples[i].valid = 0;

  do {
    /* change the snake direction if appropriate */
    switch (last_key) {
    case SNAKE_UP:
      if (snake->direction != SNAKE_DOWN)
	snake->direction = SNAKE_UP;
      break;

    case SNAKE_DOWN:
      if (snake->direction != SNAKE_UP)
	snake->direction = SNAKE_DOWN;
      break;

    case SNAKE_LEFT:
      if (snake->direction != SNAKE_RIGHT)
	snake->direction = SNAKE_LEFT;
      break;

    case SNAKE_RIGHT:
      if (snake->direction != SNAKE_LEFT)
	snake->direction = SNAKE_RIGHT;
      break;

    default:
      break;
    }

    /* add a new apple if it's time */
    if (next_apple == 0) {
      new_apple(snake, apples);
      next_apple = genrand() /
	(((unsigned int)-1) / (APPLE_MAX_WAIT - APPLE_MIN_WAIT))
	+ APPLE_MIN_WAIT;
    }

    /* move the snake */
    move_snake(snake, apples);

    /* update the score */
    set_cursor_pos(con_height - 1, 7);
    set_term_color(TEXT_COLOR);
    printf("%d", score);

    /* nap for a bit to prevent the game from going too fast */
    sleep(sleep_ticks);
    next_apple--;
  }
  /* loop until the snake kills itself */
  while(!snake_disaster(snake));
}

/* the game nibbles */
void nibbles()
{
  
  char again;
  snake_t snake;

  do {
    /* do the initial set up */
    draw_border();
    init_snake(&snake);
    last_key = -1;
    
    /* play the game */
    play_game(&snake);

    /* query to play again */
    set_term_color(TEXT_COLOR);
    set_cursor_pos(con_height / 2, (con_width - 11) / 2);

    last_key = -1;
    print(11, "Play again?");

    while ((again = last_key) != 'y' &&
	   again != 'Y' &&
	   again != 'n' &&
	   again != 'N');

  } while (again == 'y' || again == 'Y');

  task_vanish(0);
}

int main(int argc, char *argv[])
{
  int rand_seed, child_tid;

  if (argc > 1) {
    sleep_ticks = atoi(argv[1]);
  }

  /* find the width of the console */
  for(con_width = 0; !set_cursor_pos(0, con_width); con_width++);

  /* find the height of the console */
  for(con_height = 0; !set_cursor_pos(con_height, 0); con_height++);

  /* spawn key_scanner */
  child_tid = spawn_key_grabber();

  /* print out the directions */
  print_directions();

  /* get random number */
  printf("Press any key to begin nibbles\n");
  for (rand_seed = 0; last_key == -1; rand_seed++)
    sleep(1);
  sgenrand(rand_seed);

  /* start game */
  nibbles();

  return 0;
}

/*****************************************************************
 *  ____    _____    ___    ____  
 * / ___|  |_   _|  / _ \  |  _ \ 
 * \___ \    | |   | | | | | |_) |
 *  ___) |   | |   | |_| | |  __/ 
 * |____/    |_|    \___/  |_|    
 *
 * You may NOT write code this way.
 *
 * This code was written this way so it doesn't need to depend
 * on your thread library, and then because the build framework
 * requires each test program to be built from just one source
 * file.
 *
 *****************************************************************/

void foobar() {
asm(".global spawn_key_grabber\n"
	
"spawn_key_grabber:\n"
"       movl    $-1,last_key\n"         /* set the last key pressed to none*/
"	int	%0\n"           	/* Fork a clone */
"	cmpl	$0,%%eax\n"
"	je	key_grab\n"		/* Child starts grabbing keys */
"	ret\n"				/* Parent returns child tid */
	
"key_grab:\n"
"	int	%1\n"       	        /* Get a new character */
"	movl	%%eax,last_key\n"	/* Save it */
"	jmp	key_grab\n"	        /* Do it again =) */
    :: "i" (THREAD_FORK_INT)
    , "i" (GETCHAR_INT)); 
}
